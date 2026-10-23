/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include <arch_helpers.h>
#include <qti_plat.h>
#include <drivers/arm/css/scmi.h>

/*
 * Reset state encoding for the NCC CL4 workaround:
 *   Bit 31 = IMPL_RESET (implementation-defined reset)
 *   Bits 30:0 = CL4 (architecture-specific sleep state 4)
 */
#define NCC_RESET_STATE_CL4	(SCMI_RESET_STATE_IMPL | 0x4U)

void *qti_scmi_get_channel(void);

/* INTU base = HWIO_APSS_HM_BASE(0x17000000) + 0x840000  (NORDV2 SConscript) */
#define APSS_INTU_BASE			0x17840000U

/* Per-SPI type arrays (wildcat_ncc_tzbsp_hwio.h, +4*n stride) */
#define APSS_SET_LEVEL_BASE		(APSS_INTU_BASE + 0x3000U)	/* 0x17843000 */
#define APSS_CLR_EDGE_BASE		(APSS_INTU_BASE + 0x3400U)	/* 0x17843400 */
#define APSS_MRG_SET_LEVEL_BASE		(APSS_INTU_BASE + 0x4000U)	/* 0x17844000 */
#define APSS_MRG_CLR_EDGE_BASE		(APSS_INTU_BASE + 0x4400U)	/* 0x17844400 */
#define APSS_MRG_DIRECT_SPI_CFG		(APSS_INTU_BASE + 0x5C00U)	/* 0x17845C00 */

#define NUM_SPI_WORDS            57U
#define NUM_MERGER_SPI_WORDS     6U
#define NUM_DIRECT_SPI_WORDS     6U

/*
 * SPI_CONFIGURATION_DATA_NORDV2 (57 words) - verbatim from
 * wildcat_ncc_cpuss_aarch64_sysini.spp (Revision nordschleife_v2.0_p3q3r85_MTO,
 * matches IPcat IRQ map id 3711). Word 8 bit 9 = 0 -> UFS SPI 265 = LEVEL.
 */
static const uint32_t nordv2_spi_type[NUM_SPI_WORDS] = {
	0x001F8000U, 0x0000A000U, 0x00000000U, 0x0C000220U, 0x88000000U, 0x1004300CU, 0x00000890U, 0x00140000U,
	0x20608000U, 0x00FC080AU, 0x02000000U, 0x03000000U, 0x20180000U, 0x00009C13U, 0xE000801FU, 0xE0060000U,
	0x00010000U, 0x00000000U, 0xEA000000U, 0x00000001U, 0x01823000U, 0x0C000001U, 0x00000100U, 0x00000006U,
	0x08000000U, 0x0C000000U, 0x00048001U, 0x00000000U, 0x000007F6U, 0x8FC80000U, 0xFFFFFFEFU, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
	0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000080U, 0x00000000U, 0x00000000U, 0xDA000500U,
	0x00000002U,
};

/* SPI_MERGER_LOGIC_CONFIGURATION_DATA_NORD_SECA (6 words) */
static const uint32_t nord_seca_merger_type[6] = {
	0x00000024U, 0x7F000000U, 0x0FFF0000U, 0x00000000U, 0x1A000010U, 0xE025B400U,
};

/* DIRECT_SPI_CONFIGURATION_DATA_NORD_SECA (6 words) */
static const uint32_t nord_seca_direct_cfg[6] = {
	0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU,
};

void plat_irq_config(void)
{
	/* Main SPI edge/level type (covers UFS 265, geni 615, RSC 61).
	 * BIS CLR_EDGE = data ; BIC SET_LEVEL = ~data */
	configure_irq_type(APSS_CLR_EDGE_BASE, APSS_SET_LEVEL_BASE,
			   nordv2_spi_type, NUM_SPI_WORDS);

	/* Merger-logic SPI type (NORD_SECA). */
	configure_irq_type(APSS_MRG_CLR_EDGE_BASE, APSS_MRG_SET_LEVEL_BASE,
			   nord_seca_merger_type, NUM_MERGER_SPI_WORDS);

	/* Merger direct-SPI passthrough config (all 0xFFFFFFFF = passthrough). */
	configure_irq_array(APSS_MRG_DIRECT_SPI_CFG,
			    nord_seca_direct_cfg, NUM_DIRECT_SPI_WORDS);
}

/* Execute WFI. Core enters into LPM based on the LPM config */
static void execute_wfi(void)
{
	asm volatile("wfi");
}

/*
 * qti_ncc_cpu_reset_sync - Apply the NCC boot-core CL4 sleep-state reset
 * workaround via the ARM SCMI Reset Domain Management protocol.
 *
 * Context: single-core, called from bl31_early_platform_setup before
 * secondary cores are brought up.
 *
 * Mechanism:
 *   1. Encode the target core as domain_id = (AFF1 << 8) | AFF0.
 *   2. Send RESET_REQUEST (protocol 0x16, message 0x4) to CPUCP with:
 *        flags       = AUTONOMOUS_RESET_ACTION (synchronous, autonomous)
 *        reset_state = IMPL_RESET | CL4
 *   3. Enter WFI.  CPUCP applies the workaround and releases the core.
 *   4. Execution resumes from WFI+4 (the instruction after wfi).
 *
 * Returns 0 on success, or a negative SCMI error code on failure.
 */
int qti_ncc_cpu_reset_sync(uint64_t mpidr)
{
	void *ch;
	uint32_t domain_id;
	uint32_t flags;
	uint32_t reset_state;
	int ret;
	uint8_t core_id    = (uint8_t)MPIDR_AFFLVL0_VAL(mpidr);
	uint8_t cluster_id = (uint8_t)MPIDR_AFFLVL1_VAL(mpidr);

	ch = qti_scmi_get_channel();
	if (ch == NULL) {
		ERROR("NCC: failed to get SCMI channel\n");
		return -1;
	}

	/*
	 * Encode the target reset domain as CPUCP expects:
	 *   bits [15:8] = cluster index (MPIDR AFF1)
	 *   bits  [7:0] = core index within the cluster (MPIDR AFF0)
	 */
	domain_id = ((uint32_t)cluster_id << 8U) | (uint32_t)core_id;

	/*
	 * flags: Bit[0] = AUTONOMOUS_ACTION = 1 (platform drives the sequence),
	 *        Bit[2] = ASYNC_FLAG = 0 (synchronous).
	 * Per DEN0056F §3.8.2.6: Bit[0] = Autonomous Reset action.
	 */
	flags = SCMI_RESET_FLAG_SYNC | SCMI_RESET_FLAG_AUTONOMOUS;

	/*
	 * reset_state: IMPL_RESET | CL4.
	 * CPUCP interprets this as "apply the CL4 workaround for this core".
	 */
	reset_state = NCC_RESET_STATE_CL4;

	ret = scmi_reset_domain_request(ch, domain_id, flags, reset_state);
	if (ret != SCMI_E_SUCCESS) {
		ERROR("NCC: SCMI reset domain request failed: %d\n", ret);
		return ret;
	}

	/*
	 * SCMI command acknowledged.  Enter WFI; CPUCP applies the workaround
	 * and releases the core.  Execution resumes here (WFI+4).
	 */
	execute_wfi();

	return 0;
}

/*
 * plat_cpuss_config - Platform CPUSS-specific configuration.
 *
 * Called from bl31_platform_setup() in wildcat_bl31_setup.c.
 *
 * Configures SPI interrupt edge/level types for the nord platform.
 *
 * Note: The boot-core CL4 sleep-state reset workaround (previously
 * apps_reset_sync) has been moved to plat_qti_cpu_boot_setup() in
 * plat/qti/cpu/ncc/src/bl31_cpu_setup.c, which is called earlier from
 * bl31_early_platform_setup().
 */
void plat_cpuss_config(void)
{
	/* SPI interrupt edge/level configuration */
	plat_irq_config();
}
