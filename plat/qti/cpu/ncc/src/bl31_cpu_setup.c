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
#include <bl31/bl31.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <qti_plat.h>
#include "cpucp_shared.h"

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

/*
 * Stubbed out as cpu boot setup is handled in CPUCP for NCC chipsets
 * Called from plat_reset_handler() for every CPU (boot core and secondaries).
 * Must only perform operations that are safe to run on every core reset.
 */
void plat_qti_cpu_boot_setup(void)
{
	return;
}

void apsec_set_warmboot_cookie(uint64_t entrypoint)
{
    volatile apsec_boot_cookie_t *cookie = (apsec_boot_cookie_t *)APSEC_CPUCP_BOOT_COOKIE_BASE;

    /* Ensure any previous magic is wiped out first */
    cookie->magic = APSEC_BOOT_COOKIE_INVALID;

    /* Write the actual target RVBAR address */
    cookie->rvbar_addr = entrypoint;

    /* Data Synchronization Barrier ensures the address hits memory before the magic tag */
    __asm__ volatile("dsb sy" ::: "memory");

    /* Write the magic token to signal to SCP that the data is ready and valid */
    cookie->magic = APSEC_BOOT_COOKIE_MAGIC;

    /* Final barrier to ensure SCP sees the valid magic before a doorbell/interrupt is fired */
    __asm__ volatile("dsb sy" ::: "memory");
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

	apsec_set_warmboot_cookie((uint64_t) bl31_warm_entrypoint);

	ret = qti_ncc_cpu_reset_sync(read_mpidr());
	if (ret != 0) {
		WARN("NCC: boot-core reset sync returned %d\n", ret);
	}
}
