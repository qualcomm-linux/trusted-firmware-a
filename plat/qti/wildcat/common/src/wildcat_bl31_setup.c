/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>

#include <qti_plat.h>

void plat_qti_cpu_boot_setup(void)
{
	write_icc_sre_el3(read_icc_sre_el3() |
			  (ICC_SRE_SRE_BIT | ICC_SRE_EN_BIT));
	write_icc_ctlr_el3(ICC_CTLR_EL3_PMHE_BIT);
	write_icc_sre_el1(read_icc_sre_el1() | ICC_SRE_SRE_BIT);
	write_icc_igrpen0_el1(IGRPEN1_EL1_ENABLE_G0_BIT);
}

void plat_qti_bl31_setup_post(void)
{
}

void __dead2 qti_panic(void)
{
	panic();
}

int sysdbg_reset_check(void)
{
	return 0;
}

void sysdbg_entry_handler(void)
{
}
