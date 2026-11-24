/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/delay_timer.h>
#include <drivers/qti/rimps/rimps.h>
#include <drivers/qti/cprh/cprh.h>
#include <drivers/qti/pcu/pcu.h>
#include <drivers/qti/vpp/vpp.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <platform_def.h>
#include <qti_plat.h>

#define APSS_CPU_IPM_REG_BASE		0x0F800000U
#define APSS_CPU_IPM_REG_OFFSET		0x10000U
#define APSS_CPU_IPM_REG(core)		(APSS_CPU_IPM_REG_BASE + \
					 ((core) * APSS_CPU_IPM_REG_OFFSET))

#define CPU_HEAD_SWITCH_CTL(core)	(APSS_CPU_IPM_REG(core) + 0x08U)
#define CPU_SEQ_FORCE_PWR_CTL_EN(core)	(APSS_CPU_IPM_REG(core) + 0x1cU)
#define CPU_SEQ_FORCE_PWR_CTL_VAL(core)	(APSS_CPU_IPM_REG(core) + 0x20U)
#define CPU_PCHANNEL_FSM_CTL(core)	(APSS_CPU_IPM_REG(core) + 0x44U)

/* QTI_CORE_PWRDN_EN_MASK happens to be same across all CPUs */
#define QTI_CORE_PWRDN_EN_MASK		1

/* cpu power control happens to be same across all CPUs */
DEFINE_RENAME_SYSREG_RW_FUNCS(cpu_pwrctrl_val, S3_0_C15_C2_7)

/*
 * plat_qti_pwr_psci_init - PSCI backend init hook.
 *
 * vpp_init() reads the device's fused calibration data and computes the
 * ceil/floor voltage, target quotient and OSM LUT plan for every rail/clock
 * domain; it must run before anything that consumes that plan (the CPRH
 * closed-loop AVS controller).
 *
 * The secondary-core reset vector (RVBAR) is programmed by
 * qti_sec_core_remap(), invoked from plat_setup_psci_ops() ahead of this hook.
 * The Silver/Gold/L3 CPU PLLs must be locked and outputting before the
 * PCU/RIMPS hardware sequencers that drive CPU/L3 power-collapse have their
 * opcode programs loaded, since those opcode sequences assume a running PLL
 * to power back down and up.
 *
 * rimps_dcvs_init() loads the CPUCP firmware and enables the L3/Silver/
 * Gold OSM domains, issuing each one's initial perf-state request; it runs
 * after the PLLs are up and before cprh_init(), since cprh_init() starts the
 * CPR closed-loop AVS controller, which assumes the clock domains it
 * monitors are already enabled and being voted on.
 */
int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	(void)warmboot_entry;

	vpp_init();
	cprh_init();

	rimps_cpu_pll_init();
	rimps_dcvs_init();
	pcu_sequencer_init();

	return PSCI_E_SUCCESS;
}

/*
 * plat_qti_pwr_domain_on - power on a secondary core using the raw APSS IPM
 * reset sequence.
 */
int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	(void)mpidr;

	/* Program skew between en_few and en_rest. */
	mmio_write_32(CPU_HEAD_SWITCH_CTL(core_pos), 0x28U);

	/* Clear power-control enables. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_EN(core_pos), 0x0U);

	/* Close the core logic head switch. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x642U);
	udelay(2);

	/* Deassert core memory and logic clamp. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x402U);

	/* Deassert core memory slp_nret_n and slp_ret_n. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x40aU);
	udelay(4);
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x40eU);
	udelay(4);

	/* Assert and deassert wl_en_clk. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x50eU);
	udelay(2);
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x40eU);

	/* Deassert clock off. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x40cU);
	udelay(2);

	/* Assert the core P-channel power-up request. */
	mmio_write_32(CPU_PCHANNEL_FSM_CTL(core_pos), 0x1U);

	/* Deassert core reset. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x43cU);

	/* Deassert the core P-channel power-up request. */
	mmio_write_32(CPU_PCHANNEL_FSM_CTL(core_pos), 0x0U);

	/* Assert OSM core active. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x443cU);

	/* Assert CPU_PWRDUP. */
	mmio_write_32(CPU_HEAD_SWITCH_CTL(core_pos), 0x428U);

	return PSCI_E_SUCCESS;
}

/*
 * plat_qti_pwr_domain_on_finish - per-core setup once it has come online.
 * plat_cluster_init_wrapper() is called here so that the first CPU in each
 * cluster to come online initializes the DSU cluster registers.
 */
void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)core_pos;
	(void)states;

	plat_qti_gic_pcpu_init();
	plat_cluster_init_wrapper();
}

void plat_qti_set_cpupwrctlr_val(void)
{
	unsigned long val;

	val = read_cpu_pwrctrl_val();
	val |= QTI_CORE_PWRDN_EN_MASK;
	write_cpu_pwrctrl_val(val);

	isb();
}

/*
 * pcu_apply_pwr_state - select the PCU hardware sequence for the requested
 * OFF-level power state, ahead of the WFI that hands control to the PCU/
 * RIMPS sequencers. Only QTI_LOCAL_STATE_OFF is wired to a PCU mode; deeper
 * CPU standby (QTI_LOCAL_STATE_STB) stays a plain WFI retention, same as
 * before this driver existed.
 */
static void pcu_apply_pwr_state(const uint8_t *states)
{
	if (states[QTI_PWR_LVL0] == QTI_LOCAL_STATE_OFF) {
		pcu_cpu_select_mode(plat_my_core_pos(), PCU_PWR_IDX_PWR_COLLAPSE);
	}

	if (states[QTI_PWR_LVL1] == QTI_LOCAL_STATE_OFF) {
		pcu_l3_select_mode(PCU_PWR_IDX_PWR_COLLAPSE);
	}
}

/*
 * The native port only supports CPU-level standby (WFI retention) and PCU
 * power-collapse. Firmware LPM node handling (RSC/PDC/RPM) is not ported.
 */
void plat_qti_pwr_domain_off(const uint8_t *states)
{
	pcu_apply_pwr_state(states);
}

void plat_qti_pwr_domain_suspend(const uint8_t *states)
{
	pcu_apply_pwr_state(states);
}

void plat_qti_pwr_domain_suspend_finish(const uint8_t *states)
{
	if (states[QTI_PWR_LVL0] == QTI_LOCAL_STATE_OFF) {
		pcu_cpu_select_mode(plat_my_core_pos(), PCU_PWR_IDX_RUN);
	}

	if (states[QTI_PWR_LVL1] == QTI_LOCAL_STATE_OFF) {
		pcu_l3_select_mode(PCU_PWR_IDX_RUN);
	}
}

void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle)
{
	(void)id;
	(void)handle;
}

const unsigned int *plat_qti_pm_idle_states(void)
{
	static const unsigned int idle_states[] = {
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_STB,
				       PSTATE_TYPE_STANDBY),
		qti_make_pwrstate_lvl1(QTI_LOCAL_STATE_OFF, QTI_LOCAL_STATE_OFF,
				       PSTATE_TYPE_POWERDOWN),
		0,
	};

	return idle_states;
}
