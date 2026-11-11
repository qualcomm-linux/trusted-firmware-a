/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.globl	plat_report_unhandled_exception
	.globl	plat_report_unhandled_interrupt

	/* -----------------------------------------------------
	 * Unified platform handler for unhandled exceptions.
	 *
	 * The common EL3 exception path has already prepared the
	 * minimal state needed by the crash reporting path before
	 * branching here:
	 *
	 *   x0          = exception reason code
	 *   TPIDRRO_EL0 = original x0 captured at exception entry
	 *
	 * This platform hook preserves the incoming x30 in TPIDR_EL3
	 * before tail-branching to the common unhandled-exception
	 * reporter. The common reporter will then build the crash
	 * buffer, dump the CPU state, and enter the platform panic
	 * handler. This path does not return.
	 *
	 *   TPIDR_EL3   = x30 observed on entry to this hook
	 * -----------------------------------------------------
	 */
func plat_report_unhandled_exception
	msr	TPIDR_EL3, x30
	b	report_unhandled_exception
endfunc plat_report_unhandled_exception

	/* -----------------------------------------------------
	 * Unified platform handler for unhandled interrupts.
	 *
	 * The common interrupt vector has already saved the original
	 * x0 in TPIDRRO_EL0 and replaced x0 with the interrupt reason
	 * code before reaching this hook:
	 *
	 *   x0          = interrupt reason code
	 *   TPIDRRO_EL0 = original x0 captured at interrupt entry
	 *
	 * This hook mirrors the exception path by saving the incoming
	 * x30 in TPIDR_EL3, then tail-branches to the common interrupt
	 * crash reporter. The reporter records the remaining crash
	 * context and enters the platform panic handler. This path
	 * does not return.
	 *
	 *   TPIDR_EL3   = x30 observed on entry to this hook
	 * -----------------------------------------------------
	 */
func plat_report_unhandled_interrupt
	msr	TPIDR_EL3, x30
	b	report_unhandled_interrupt
endfunc plat_report_unhandled_interrupt
