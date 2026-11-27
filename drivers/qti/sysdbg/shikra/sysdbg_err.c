/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * System Debug - Error Handling and Logging
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/qsee/src/sysdbg_err.c
 * trustzone_images/core/systemdebug/log/src/sysdbg_log.c
 *
 * This file contains error handling functions and logging management.
 * Executes from DDR (default .text section).
 */

#include <stdbool.h>
#include <stdint.h>
#include <lib/mmio.h>
#include <drivers/qti/sysdbg/sysdbg_crumbs.h>

/*
 * External functions
 */
extern void sysdbg_target_reset_soc(void);  /* From sysdbg_target.c (IMEM) */

/*
 * sysdbg_log_deinit - De-initialize logging subsystem
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/log/src/sysdbg_log.c:168
 *
 * Called before SoC reset to cleanly shut down logging.
 *
 * This function writes a completion cookie to SHARED_IMEM to indicate
 * that PASS1 debug collection has completed successfully. This cookie
 * is used by boot software and analysis tools to determine system state.
 *
 * In TZ, this function also:
 * - Flushes any pending log entries to shared memory
 * - Marks log buffer as complete
 * - Disables further logging
 *
 * For TFA Phase 1: Writes completion cookie (minimal implementation).
 * For TFA Phase 2: Add full logging flush when logging infrastructure is ported.
 */
void sysdbg_log_deinit(void)
{
	/*
	 * Write PASS1 completion cookie to SHARED_IMEM
	 *
	 * This indicates to boot software that:
	 * 1. PASS1 debug collection completed successfully
	 * 2. System is about to reset to PASS2
	 * 3. Debug data has been saved to memory
	 *
	 * Address: SYSTEM_SHAREDIMEM_BASE + SYSDBG_PASS1COMPL_OFFSET
	 * Value: SYSDBG_PASS1_SUCCESS_COOKIE (0xDEADD00D)
	 */
	volatile uint32_t *sysdbg_complete_cookie =
		(uint32_t *)(SYSTEM_SHAREDIMEM_BASE + SYSDBG_PASS1COMPL_OFFSET);

	mmio_write_32((uintptr_t)sysdbg_complete_cookie, SYSDBG_PASS1_SUCCESS_COOKIE);

	/*
	 * TODO Phase 2: Add logging infrastructure
	 *
	 * When logging is fully ported, this function should also:
	 * 1. Flush any pending log entries from buffer to SHARED_IMEM
	 * 2. Mark log buffer index as final (no more entries)
	 * 3. Disable further logging operations
	 * 4. Save log buffer pointer to SHARED_IMEM for analysis tools
	 */
}

/*
 * sysdbg_reset_soc - Wrapper for SoC reset
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/qsee/src/sysdbg_err.c:91
 *
 * This is the wrapper function called from system_debug_exit().
 * It performs cleanup operations before triggering the actual hardware reset.
 *
 * Call chain:
 *   system_debug_exit() [sysdbg_main.c]
 *     -> sysdbg_reset_soc() [THIS FUNCTION - sysdbg_err.c, DDR]
 *       -> sysdbg_target_reset_soc() [sysdbg_target.c, IMEM]
 *         -> first_pass_complete() [sysdbg_target.c, IMEM]
 *           -> [Hardware Reset]
 *
 * This function never returns - SoC resets to enter PASS2.
 */
void sysdbg_reset_soc(void)
{
	/* De-init the log */
	sysdbg_log_deinit();

	/* Target SOC reset */
	sysdbg_target_reset_soc();
}
