/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>

#include <qti_plat.h>

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
