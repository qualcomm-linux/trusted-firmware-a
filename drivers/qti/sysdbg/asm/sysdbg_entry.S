/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * SDI (System Debug Image) EL3 entry, reset-path chain:
 *   plat_reset_handler (qti_helpers.S) -> sysdbg_entry_handler ->
 *   system_debug_reset_handler -> system_debug_reset_entry (sysdbg_main.c)
 *
 * Two deliberate design choices versus a hawi-family downstream reference,
 * both documented in the porting session's plan under "Section 0":
 *
 * 1. Dropped: the downstream secure-monitor's execution-state-flag and
 *    boot-term-cookie notifications, and its private EL3 fault vectors +
 *    fatal-error handler. Those wrote to hawi-specific shared-IMEM
 *    addresses with no reader here, and installing a second private
 *    VBAR_EL3 here would conflict with the vector table this BL31 already
 *    owns (PLAT_OVERRIDE_EXCEPTION_HANDLERS / qti_unhandled_exception_asm.S).
 *    Any fault during the SDI pass falls through to the existing
 *    plat_panic_handler / bl31qtilib_panic() path instead (qti_helpers.S).
 * 2. The downstream reference's CPU-logical-number helper has no equivalent
 *    here; substituted with plat_my_core_pos() (qti_helpers.S), which
 *    returns the same value - this core's logical index - via the same
 *    x0-in/x0-out convention.
 *
 * The stack carve-outs (sysdbg_stack 0x1800, sysdbg_reset_stack 0x3C00)
 * and per-core offset arithmetic are ported unchanged, including the
 * source quirk where system_debug_reset_handler computes the
 * sysdbg_reset_stack offset but never switches SP to it - the C call
 * chain genuinely keeps running on sysdbg_stack the whole time even in
 * the reference. Preserved as-is per explicit "no behavioral change"
 * direction, not corrected.
 */

#include <asm_macros.S>

	.globl	sysdbg_entry_handler
	.globl	system_debug_reset_handler
	.extern	plat_my_core_pos
	.extern	system_debug_reset_entry

	/* ---------------------------------------------------------------
	 * void sysdbg_entry_handler(void)
	 * Reached only when sysdbg_reset_check() (Layer 2) returns
	 * non-zero. Does not return to its caller - calls
	 * system_debug_reset_handler and, eventually, the pass-1-complete
	 * park loop in sysdbg_target.c.
	 * ---------------------------------------------------------------
	 */
func sysdbg_entry_handler
	clrex

	bl	plat_my_core_pos
	mov	x1, #0x200
	mul	x0, x0, x1

	/* Switch to the per-core slice of sysdbg_stack (0x1800 total). */
	ldr	x1, =sysdbg_stack
	sub	x1, x1, x0
	mov	sp, x1

	/* The SDI pass ends by parking the core; it does not return. */
	no_ret	system_debug_reset_handler
endfunc sysdbg_entry_handler

	/* ---------------------------------------------------------------
	 * void system_debug_reset_handler(void)
	 * ---------------------------------------------------------------
	 */
func system_debug_reset_handler
	bl	plat_my_core_pos
	mov	x1, #0x500
	mul	x0, x0, x1

	/*
	 * Computed but not switched to - ported unchanged from source,
	 * see file header. system_debug_reset_entry runs on sysdbg_stack.
	 */
	ldr	x1, =sysdbg_reset_stack
	sub	x1, x1, x0

	no_ret	system_debug_reset_entry
endfunc system_debug_reset_handler

	.p2align 7
sysdbg_stack_space:
	.space 0x1800
sysdbg_stack:

	.p2align 7
sysdbg_reset_stack_space:
	.space 0x3C00
sysdbg_reset_stack:
