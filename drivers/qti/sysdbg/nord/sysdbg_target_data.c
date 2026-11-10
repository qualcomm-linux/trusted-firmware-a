/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <sysdbg_main.h>
#include <sysdbg_mem_dump.h>
#include <sysdbg_reset.h>
#include <sysdbg_target.h>
#include <sysdbg_timer.h>

/*
 * Pass sequence: ported from hawi's sysdbg_target_data.c. The first NULL
 * fcn_info entry ends the sequence; entries after it are ignored.
 */
static const sysdbg_pass_data_type sysdbg_pass_data = {
	.function_profiler = SysdbgGetTimestamp,
	.pass_sequence = {
		.fcn_info = {
			{ { true }, SysdbgHWInit },
			{ { true }, SysdbgRecoverRegisters },
			{ { true }, SysdbgMemoryDumpInit },
			{ { true }, SysdbgCacheEnable },
			{ { true }, NULL },
		},
	},
};

/*
 * Target configuration. On Nord, SOCCP owns the reset-debug enables
 * (reset_dbgen, PMIC resin / tsense / wdog debug, timeouts and the
 * per-subsystem dbgen bits), so TF-A only keeps what its own SDI path reads:
 * reset_dbgen (SysdbgTargetEnable) and icache_en/dcache_en
 * (SysdbgCacheEnable). Values are unchanged from the hawi-derived defaults.
 */
static const sysdbg_config_type sysdbg_config_data = {
	.reset_dbgen = 0,
	.icache_en = 1,
	.dcache_en = 0,
};

/* Register dump configuration - no default clocked/accessible registers
 * populated (matches hawi's terminating-only entry).
 */
static register_type sysdbg_register_data[] = {
	[0].register_addr = 0,
};

const mem_config_type *Sysdbg_MemConfig(void)
{
	return NULL;
}

const sysdbg_config_type *Sysdbg_Config(void)
{
	return &sysdbg_config_data;
}

const sysdbg_pass_data_type *Sysdbg_PassConfig(void)
{
	return &sysdbg_pass_data;
}

void *Sysdbg_RegConfig(void)
{
	return (void *)sysdbg_register_data;
}
