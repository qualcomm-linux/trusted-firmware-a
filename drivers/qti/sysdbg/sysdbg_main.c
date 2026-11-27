/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <common/debug.h>
#include <arch_helpers.h>
#include <plat/common/platform.h>
#include <drivers/qti/sysdbg/sysdbg_main.h>
#include <drivers/qti/sysdbg/sysdbg_reset.h>
#include <drivers/qti/sysdbg/sysdbg_target.h>
#ifdef SYSDBG_DCC
#include <drivers/qti/sysdbg/sysdbg_dcc.h>
#endif
#include <drivers/qti/sysdbg/sysdbg_crumbs.h>
#include <drivers/qti/sysdbg/sysdbg_log.h>

int system_debug_main(void)
{
	bool status = true;

	NOTICE("SDI: Cold boot initialization started\n");

	status &= sysdbg_cold_boot_init();
	if (!status) {
		ERROR("SDI: Cold boot init failed\n");
		return SYSDBG_ERROR;
	}

	sysdbg_log_init();

	INFO("SDI: Collecting target information\n");
	status &= sysdbg_target_info_update();

	INFO("SDI: Enabling debug features\n");
	status &= sysdbg_target_enable();

	#ifdef SYSDBG_DCC
	INFO("SDI: Initializing DCC\n");
	status &= sysdbg_dcc_init();
	#endif

	if (!status) {
		ERROR("SDI: Cold boot initialization incomplete\n");
		return SYSDBG_ERROR;
	}

	NOTICE("SDI: Cold boot initialization complete\n");
	return SYSDBG_SUCCESS;
}

/*===========================================================================
 * Crash Path Functions (SDI PASS1)
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/qsee/src/sysdbg_main.c
 *===========================================================================
 */

/* External functions */
extern void sysdbg_reset_soc(void);

/*
 * get_current_cpu_seq - Get current CPU sequential number
 *
 * Ported from TZ: qsee/src/sysdbg_system.c
 *
 * In TZ: uses tzbsp_cur_cpu_num()
 * In TFA: uses plat_my_core_pos()
 */
uint32_t get_current_cpu_seq(subsys_type subsystem)
{
	(void)subsystem;
	return (uint32_t)plat_my_core_pos();
}

/*
 * sysdbg_err_fatal_handler - Fatal error handler
 *
 * Ported from TZ: qsee/src/sysdbg_err.c
 *
 * TODO Phase 2: Implement full error handling
 */
void sysdbg_err_fatal_handler(void)
{
	/* Phase 1 stub: infinite WFI loop */
	while (1) {
		wfi();
	}
}

/*
 * system_debug_exit - Exit crash path and trigger reset
 *
 * Core 0 waits for all cores to complete, then triggers reset.
 * Other cores wait in WFI.
 */
void system_debug_exit(uint32_t cpu)
{
	static volatile bool set_complete_flag;

	if (cpu == 0U) {
		set_complete_flag = true;

		/* Core 0: Trigger SoC reset (never returns) */
		sysdbg_reset_soc();
	}

	/* Other cores: Wait for reset */
	while (!set_complete_flag) {
		wfi();
	}

	/* Should never reach here */
	while (1) {
		wfi();
	}
}

/*
 * system_debug_reset_entry - Main crash path entry point
 *
 * Ported from TZ: sysdbg/qsee/src/sysdbg_main.c
 *
 * Called from system_debug_reset_handler() in sysdbg_entry.S, after DDR has
 * been taken out of self refresh. For now it only exits and triggers the reset
 * to the next pass.
 */
void system_debug_reset_entry(void)
{
	system_debug_exit(get_current_cpu_seq(SUBSYS_APPS));
}
