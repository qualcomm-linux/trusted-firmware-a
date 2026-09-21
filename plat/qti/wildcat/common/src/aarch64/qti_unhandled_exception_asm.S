/**********************************************************************
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *********************************************************************/

#include <asm_macros.S>

	.globl	plat_report_unhandled_exception
	.globl	plat_report_unhandled_interrupt

	/* -----------------------------------------------------
	 * Unified platform handler for unhandled exceptions.
	 *
	 * x0: exception reason code, set by the common exception path
	 * TPIDRRO_EL0: original x0 at the point of exception
	 * -----------------------------------------------------
	 */
func plat_report_unhandled_exception
	msr	TPIDR_EL3, x30
	b	report_unhandled_exception
endfunc plat_report_unhandled_exception

	/* -----------------------------------------------------
	 * Unified platform handler for unhandled interrupts.
	 *
	 * x0: interrupt reason code, set by the common exception path
	 * TPIDRRO_EL0: original x0 at the point of exception
	 * -----------------------------------------------------
	 */
func plat_report_unhandled_interrupt
	msr	TPIDR_EL3, x30
	b	report_unhandled_interrupt
endfunc plat_report_unhandled_interrupt
