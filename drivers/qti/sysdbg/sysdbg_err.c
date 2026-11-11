/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <sysdbg_err.h>
#include <sysdbg_log.h>
#include <sysdbg_target.h>

void SysdbgResetSoC(void)
{
	SysdbgLogDeInit();
	SysdbgTargetResetSoc();
}

void sysdbg_err_fatal_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
	bool cold_boot;

	SYSDBG_LOG_EVENT(FATAL_ERROR, arg0, arg1, arg2);
	(void)arg3;

	SysdbgLogDeInit();

	cold_boot = SysdbgTargetResetStatus();

	if (!cold_boot) {
		SysdbgResetSoC();
	}
}
