/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * NCC (Oryon) CPU-specific BL31 setup for QTI platforms.
 *
 * This file contains CPU-specific initialisation for the Qualcomm NCC
 * (Next-generation Custom CPU / Oryon) architecture. It is selected by
 * setting QTI_NCC_CPU := 1 in the platform.mk file.
 *
 * Responsibilities:
 *   - EL3 GIC system-register interface initialisation (qti_el3_sys_regs_init)
 *   - Boot-core CL4 sleep-state reset workaround via SCMI Reset Domain
 *     Management protocol (qti_ncc_cpu_reset_sync)
 *   - plat_qti_cpu_boot_setup entry point called by wildcat_bl31_setup.c
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>

/* Forward declaration – provided by plat/qti/common/src/qti_scmi_doorbell.c */
void *qti_scmi_get_channel(void);

/*
 * Reset state encoding for the NCC CL4 workaround:
 *   Bit 31 = IMPL_RESET (implementation-defined reset)
 *   Bits 30:0 = CL4 (architecture-specific sleep state 4)
 */
#define NCC_RESET_STATE_CL4	(SCMI_RESET_STATE_IMPL | 0x4U)

/*
 * qti_el3_sys_regs_init - Initialise EL3 system registers for NCC CPUs.
 *
 * Configures the GIC system-register interface and interrupt-control
 * registers at EL3 for the NCC (Oryon) CPU architecture.
 */
void qti_el3_sys_regs_init(void)
{
	/* EL3 SRE: enable system-register interface for EL3 */
	write_icc_sre_el3(0x9U | read_icc_sre_el3());

	/* Set PMHE & IDbits to 24 bits */
	write_icc_ctlr_el3(0xCC40U);

	/* EL1 SRE: enable system-register interface for EL1 */
	write_icc_sre_el1(0x1U | read_icc_sre_el1());

	/* Enable Group 0 interrupts at EL1 */
	write_icc_igrpen0_el1(1U);
}

/* Execute WFI. Core enters into LPM based on the LPM config */
void execute_wfi(void)
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
static int qti_ncc_cpu_reset_sync(uint64_t mpidr)
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
 * plat_qti_cpu_boot_setup - NCC CPU per-core EL3 register initialisation.
 *
 * Called from plat_reset_handler() for every CPU (boot core and secondaries).
 * Must only perform operations that are safe to run on every core reset.
 */
void plat_qti_cpu_boot_setup(void)
{
	qti_el3_sys_regs_init();
}

/*
 * plat_qti_cpu_boot_cluster_reset - NCC boot-core CL4 sleep-state workaround.
 *
 * Called once from bl31_early_platform_setup() on the boot core only,
 * before secondary cores are brought up.  Sends an SCMI Reset Domain
 * request to CPUCP to apply the CL4 sleep-state reset workaround.
 */
void plat_qti_cpu_boot_cluster_reset(void)
{
	int ret;

	ret = qti_ncc_cpu_reset_sync(read_mpidr());
	if (ret != 0) {
		WARN("NCC: boot-core reset sync returned %d\n", ret);
	}
}
