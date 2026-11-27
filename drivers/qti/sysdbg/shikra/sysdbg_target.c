/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>
#include <lib/mmio.h>
#include <common/debug.h>
#include <arch_helpers.h>
#include <qti_plat.h>
#include <drivers/qti/sysdbg/sysdbg_target.h>
#include <drivers/qti/sysdbg/sysdbg_reset.h>
#include <drivers/qti/sysdbg/sysdbg_main.h>
#include <sysdbg_target_config.h>
#include <sysdbg_hwio.h>

/*
 * Target information structure - cluster and CPU topology
 * Ported from TZ sysdbg_target.c
 */
typedef struct {
	uint8_t cores;           /* Number of cores in cluster */
	const char *cpuname;     /* CPU architecture name */
} cluster_info_t;

typedef struct {
	cluster_info_t clusters[SYSDBG_NUM_CLUSTERS];
	uint32_t cpumask;        /* CPU present bitmask from HW register */
} subsys_info_t;

typedef struct {
	subsys_info_t subsys_apps;
	uint32_t chip_version;
} target_info_t;

static target_info_t target_info;
static bool target_info_init;

/* Forward declarations */
static bool sysdbg_target_cold_clock_init(void);

uint32_t sysdbg_get_chip_rev(void)
{
	return target_info.chip_version;
}

/*
 * sysdbg_target_info_update - Initialize target CPU topology information
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/qsee/src/target/shikra/sysdbg_target.c:921
 *
 * In TZ, this function uses a static init flag to run configuration once.
 * We preserve this pattern in TFA to ensure idempotent initialization.
 *
 * Shikra CPU topology (from sysdbg_target_config.h):
 * - TARGET_CLUSTER_0_SIZE = 3 (Cluster 0: 3 cores)
 * - TARGET_CLUSTER_1_SIZE = 1 (Cluster 1: 1 core)
 * - TARGET_APPS_CPUS = 4 (Total: 4 CPUs)
 *
 * Reference TZ code:
 *   target_info.subsys[SUBSYS_APPS].clusters[0].cores = TARGET_CLUSTER_0_SIZE;
 *   target_info.subsys[SUBSYS_APPS].clusters[1].cores = TARGET_CLUSTER_1_SIZE;
 *   target_info.subsys[SUBSYS_APPS].clusters[0].cpuname = ARMV8_CPU;
 *   target_info.subsys[SUBSYS_APPS].cpumask = HWIO_INF(APSS_SHARED_CPU_PRESENT, CORE_PRESENT);
 */
bool sysdbg_target_info_update(void)
{
	if (!target_info_init) {
		/* Store chip version */
		target_info.chip_version = 0x10;  /* Shikra v1.0 */

		/*
		 * Configure cluster topology
		 * Cluster 0: 3 cores (ARMv8 CPUs)
		 * Cluster 1: 1 core (ARMv8 CPU)
		 */
		target_info.subsys_apps.clusters[0].cores = SYSDBG_CLUSTER_0_SIZE;
		target_info.subsys_apps.clusters[1].cores = SYSDBG_CLUSTER_1_SIZE;
		target_info.subsys_apps.clusters[0].cpuname = "ARMv8";
		target_info.subsys_apps.clusters[1].cpuname = "ARMv8";

		/*
		 * CPU present mask
		 *
		 * TZ reads from HWIO_INF(APSS_SHARED_CPU_PRESENT, CORE_PRESENT),
		 * but TZ comment states: "Switch back to static values because
		 * register changed from v1 to v2"
		 *
		 * Strategy: Read the register for diagnostic purposes and log
		 * comparison, but use static value (0x0F) as TZ does.
		 *
		 * Expected: 4 CPUs = 0x0F bitmask
		 */
		uint32_t hw_cpumask = mmio_read_32(HWIO_APSS_SHARED_CPU_PRESENT_ADDR) &
				      HWIO_APSS_SHARED_CPU_PRESENT_CORE_PRESENT_BMSK;
		uint32_t static_cpumask = 0x0F;  /* Expected: All 4 CPUs present */

		/* Use static value as TZ does, but log comparison */
		target_info.subsys_apps.cpumask = static_cpumask;

		if (hw_cpumask != static_cpumask) {
			WARN("SDI: CPU mask mismatch - HW=0x%x, Static=0x%x (using static)\n",
			     hw_cpumask, static_cpumask);
		} else {
			INFO("SDI: CPU mask verified - HW=0x%x matches expected 0x%x\n",
			     hw_cpumask, static_cpumask);
		}

		target_info_init = true;

		INFO("SDI: Target info initialized\n");
		INFO("SDI: Chip rev=0x%x, Cluster0=%u cores, Cluster1=%u cores, CPU mask=0x%x\n",
		     target_info.chip_version,
		     target_info.subsys_apps.clusters[0].cores,
		     target_info.subsys_apps.clusters[1].cores,
		     target_info.subsys_apps.cpumask);
	}

	return true;
}

static bool sysdbg_target_cold_clock_init(void)
{
	/*
	 * Scandump needs the APB clock on if reset debug is enabled
	 * This is a workaround for v1 hardware. No harm applying the vote on v2 as well.
	 */
	mmio_setbits_32(HWIO_GCC_APCS_TZ_VOTE_QDSS_APB_CLK_ADDR,
			HWIO_GCC_APCS_TZ_VOTE_QDSS_APB_CLK_CLK_ENABLE_BMSK);

	/* Memory barrier to ensure clock enable completes */
	dsbsy();

	INFO("SDI: QDSS APB clock enabled\n");
	return true;
}

/* Vendor SYSTEM_RESET2 sub-type for a PMIC warm reset (see pm_ps_hold.c) */
#define SYSDBG_RESET2_PMIC_WARM_RESET	0U

bool sysdbg_target_enable(void)
{
	const sysdbg_config_t *config = sysdbg_get_config();
	uint32_t data = 0;

	INFO("SDI: Target enable started\n");

	/* Check if reset debug is globally enabled */
	if (!config->reset_dbgen) {
		INFO("SDI: Reset debug disabled in config\n");
		return true;
	}

	/* Configure clocks */
	sysdbg_target_cold_clock_init();

	/* Initialize dump table magic number */
	uint32_t *dump_table = (uint32_t *)(sysdbg_get_shared_imem_base() + 0x10);
	*dump_table = 0x42445953;

	/* Build GCC_RESET_DEBUG_ENABLE register value */
	data = HWIO_GCC_RESET_DEBUG_ENABLE_AUTO_PS_HOLD_SET_EN_BMSK;

	if (config->wdog_dbgen)
		data |= HWIO_GCC_RESET_DEBUG_ENABLE_SECURE_WDOG_RESET_DEBUG_EN_BMSK;

	if (config->tsense_dbgen)
		data |= HWIO_GCC_RESET_DEBUG_ENABLE_MSM_TSENSE_RESET_DEBUG_EN_BMSK;

	if (config->pmic_resin_dbgen)
		data |= HWIO_GCC_RESET_DEBUG_ENABLE_PMIC_RESIN_RESET_DEBUG_EN_BMSK |
			HWIO_GCC_RESET_DEBUG_ENABLE_ALLOW_SW_BASED_RST_DBG_EN_BMSK;

	if (config->acc_ctl_rst)
		data |= HWIO_GCC_RESET_DEBUG_ENABLE_ACCESS_RESET_FIRST_PASS_EN_BMSK;

	if (config->dbg_ctl_rst)
		data |= HWIO_GCC_RESET_DEBUG_ENABLE_DEBUG_RESET_FIRST_PASS_EN_BMSK;

	mmio_write_32(HWIO_GCC_RESET_DEBUG_ENABLE_ADDR, data);
	INFO("SDI: GCC_RESET_DEBUG_ENABLE = 0x%x\n", data);

	/* Clear GCC_RESET_STATUS */
	mmio_write_32(HWIO_GCC_RESET_STATUS_ADDR, HWIO_GCC_RESET_STATUS_RMSK);
	mmio_write_32(HWIO_GCC_RESET_STATUS_ADDR, 0xF);

	/* Configure Reset FSM Control */
	mmio_clrbits_32(HWIO_GCC_RESET_FSM_CTRL_ADDR,
			HWIO_GCC_RESET_FSM_CTRL_FIRST_PASS_COMPLETE_BMSK);

	if (config->timeout1_en)
		mmio_setbits_32(HWIO_GCC_RESET_FSM_CTRL_ADDR,
				HWIO_GCC_RESET_FSM_CTRL_FIRST_PASS_TMR_EN_BMSK);

	/* Configure timeout values */
	if (config->timeout1_en) {
		uint32_t timeout_val = CONVERT_TO_CYCLES(config->timeout1, GCC_XO_CLOCK_HZ);

		mmio_write_32(HWIO_GCC_FIRST_PASS_TIMEOUT_ADDR, timeout_val);
		INFO("SDI: First pass timeout=%u ms\n", config->timeout1);
	}

	/* Program delay timers */
	mmio_write_32(HWIO_GCC_WIND_DOWN_TIMER_ADDR,
		      CONVERT_TO_CYCLES(config->wind_down_dly, GCC_SLEEP_CLOCK_HZ));
	mmio_write_32(HWIO_GCC_SAVE_CONTENTS_TIMER_ADDR,
		      CONVERT_TO_CYCLES(config->save_data_dly, GCC_SLEEP_CLOCK_HZ));
	mmio_write_32(HWIO_GCC_STOP_CAPTURE_TIMER_ADDR,
		      CONVERT_TO_CYCLES(config->stop_trace_dly, GCC_SLEEP_CLOCK_HZ));
	mmio_write_32(HWIO_GCC_PREPARE_FOR_RESET_TIMER_ADDR,
		      CONVERT_TO_CYCLES(config->prep_ares_dly, GCC_SLEEP_CLOCK_HZ));

	INFO("SDI: All timer delays configured\n");

	/* QDSS CoreSight lock/unlock sequence */
	if (!config->dbg_ctl_rst) {
		mmio_write_32(HWIO_QDSS_QDSS_CS_QDSSCSR_LOCKACCESS_ADDR, QDSS_CORESIGHT_UNLOCK);
		dsbsy();
		mmio_write_32(HWIO_QDSS_QDSS_CS_QDSSCSR_LOCKACCESS_ADDR, QDSS_CORESIGHT_LOCK);
		INFO("SDI: QDSS CoreSight lock sequence applied\n");
	}

	/* Configure PMIC for warm reset on PS_HOLD assertion */
	/*
	 * SDI needs a warm reset so that DDR and IMEM survive and BL31 is
	 * re-entered through its warm reset handler, unlike the hard reset
	 * that qti_pmic_prepare_reset() programs for PSCI SYSTEM_RESET.
	 */
	if (qti_pmic_prepare_reset2(SYSDBG_RESET2_PMIC_WARM_RESET, 0U) != 0) {
		ERROR("SDI: PMIC warm reset configuration failed\n");
		return false;
	}
	INFO("SDI: PMIC PS_HOLD configuration complete\n");

	INFO("SDI: Target enable complete\n");
	return true;
}

/*
 * ============================================================================
 * Crash Path Functions (SDI PASS1)
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 *   - sysdbg/qsee/src/target/shikra/sysdbg_target.c
 *   - sysdbg/qsee/src/sysdbg_err.c
 * ============================================================================
 */

/*
 * IMEM Function Attribute
 */
#define SYSDBG_IMEM_FUNC	__section(".text_imem")

/*
 * IMEM Data Attribute
 */
#define SYSDBG_IMEM_DATA	__section(".data_imem")

/*
 * DDR Self-Refresh Enter Function Pointer
 *
 * Passed from U-Boot SPL via BL31 parameters.
 * Used to put DDR back into self-refresh before triggering reset.
 *
 * Phase 1: NULL (stubbed)
 * Phase 2: Will be populated from BL31 boot parameters
 */
static SYSDBG_IMEM_DATA bool (*ddr_self_refresh_enter)(void);

/*
 * Alternate IMEM Stack Pointer
 *
 * Used when putting DDR back into self-refresh.
 * Must switch from DDR stack to IMEM stack before DDR enters self-refresh.
 */
static SYSDBG_IMEM_DATA uint64_t alt_imem_stack_ptr;

/* External Functions */
extern uint64_t bl31_get_ddr_enter_self_refresh_addr(void);
extern uint64_t sysdbg_stack;	/* From sysdbg_tfa_bruin.S */

/*
 * __set_stack - Set stack pointer
 *
 * ARMv8 inline assembly to set SP.
 */
static inline void __set_stack(uint64_t *stack_ptr)
{
	__asm__ volatile("mov sp, %0" : : "r"(stack_ptr));
}

/*
 * first_pass_complete - Signal first pass complete and trigger reset
 *
 * **MUST EXECUTE FROM IMEM**
 *
 * Ported from TZ: sysdbg/qsee/src/target/shikra/sysdbg_target.c
 *
 * This function:
 * 1. Switches from DDR stack to IMEM stack
 * 2. Puts DDR back into self-refresh
 * 3. Writes to GCC_RESET_FSM_CTRL to signal first pass complete
 * 4. Waits for hardware reset
 *
 * This function never returns - hardware resets SoC.
 */
static SYSDBG_IMEM_FUNC __dead2 void first_pass_complete(void)
{
	/* Switch from DDR stack to IMEM stack */
	if (alt_imem_stack_ptr != 0) {
		__set_stack((uint64_t *)alt_imem_stack_ptr);
	}

	/*
	 * Put DDR back into self-refresh
	 * Phase 1: Using stub (no actual DDR operation)
	 */
	if (ddr_self_refresh_enter) {
		ddr_self_refresh_enter();
	}

	/* Signal first pass complete */
	mmio_write_32(HWIO_GCC_RESET_FSM_CTRL_ADDR,
		      mmio_read_32(HWIO_GCC_RESET_FSM_CTRL_ADDR) |
		      HWIO_GCC_RESET_FSM_CTRL_FIRST_PASS_COMPLETE_BMSK);

	/* Wait for hardware reset */
	while (1) {
		wfi();
	}
}

/*
 * sysdbg_wait_for_reset - Secondary core wait loop
 *
 * **MUST EXECUTE FROM IMEM**
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/qsee/src/target/shikra/sysdbg_target.c:129
 *
 * Secondary cores call this function to wait in an infinite loop for the
 * hardware reset triggered by first_pass_complete() on the primary core.
 *
 * This function executes after DDR has been put back into self-refresh,
 * so it must be in IMEM.
 *
 * This function never returns - hardware resets SoC.
 */
SYSDBG_IMEM_FUNC __dead2 void sysdbg_wait_for_reset(void)
{
	/*
	 * Wait for hardware reset in low-power mode.
	 * Primary core will trigger reset via first_pass_complete().
	 */
	while (1) {
		wfi();
	}
}

/*
 * sysdbg_target_reset_soc - Target-specific SoC reset implementation
 *
 * **MUST EXECUTE FROM IMEM**
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/qsee/src/target/shikra/sysdbg_target.c:199
 *
 * Called by sysdbg_reset_soc() wrapper at end of PASS1 debug collection.
 * Sets up IMEM stack and calls first_pass_complete() to trigger reset.
 *
 * This function never returns - SoC resets to enter PASS2.
 */
SYSDBG_IMEM_FUNC void sysdbg_target_reset_soc(void)
{
	/* Get DDR self-refresh enter function if not already cached */
	if (!ddr_self_refresh_enter) {
		ddr_self_refresh_enter = (bool (*)(void))SYSDBG_DDR_SDI_EXIT_ADDR;
	}

	/* Set alternate IMEM stack pointer */
	alt_imem_stack_ptr = (uint64_t)&sysdbg_stack;

	/* Trigger first pass complete (never returns) */
	first_pass_complete();
}

/*
 * sysdbg_save_shared_data - Save shared data from BL31
 *
 * Ported from TZ: qsee/src/target/shikra/sysdbg_target.c
 */
void sysdbg_save_shared_data(sysdbg_shared_data *shared_data)
{
	if (!shared_data)
		return;

	ddr_self_refresh_enter = (bool (*)(void))shared_data->enter_ddr_self_refresh_addr;
	alt_imem_stack_ptr = shared_data->mon_imem_stack_ptr;
}
