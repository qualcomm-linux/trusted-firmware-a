/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * System Debug - Reset Handler Entry (DDR Execution)
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/qsee/src/sysdbg_entry.s
 *
 * This file contains the reset handler that executes from DDR after
 * DDR has been brought out of self-refresh.
 *
 * Executes from DDR (default .text section).
 */

#include <arch.h>
#include <asm_macros.S>

/*
 * Assembly macros for stack operations
 *
 * PUSH2 - Push 2 registers on descending stack
 */
	.macro PUSH2 stack, reg1, reg2
	stp	\reg1, \reg2, [\stack, #-0x10]!
	.endm

/*
 * POP2 - Pop 2 registers from descending stack
 */
	.macro POP2 stack, reg1, reg2
	ldp	\reg1, \reg2, [\stack], #0x10
	.endm

	/*
	 * External function declarations
	 */
	.extern	system_debug_reset_entry
	.extern	sysdbg_err_fatal_handler
	.extern	sysdbg_save_shared_data

	/*
	 * Exported symbols
	 */
	.globl	system_debug_reset_handler
	.globl	sysdbg_vector_table

	/*
	 * Default .text section (DDR execution)
	 */
	.section .text, "ax"
	.p2align 3

/*
 * system_debug_reset_handler - Main reset handler in DDR
 *
 * This function executes from DDR after DDR has been brought out of
 * self-refresh by system_debug_bl31_entry().
 *
 * Must execute from DDR (not IMEM).
 *
 * Called from: sysdbg_entry_handler() in sysdbg_imem_entry.S after
 *              system_debug_bl31_entry() returns.
 *
 * Tasks:
 *   (1) Save shared data structure
 *   (2) Setup DDR stack (dual stack mechanism to avoid clobbering)
 *   (3) Call system_debug_reset_entry() for main crash dump logic
 *
 * The dual stack mechanism alternates between two stacks to prevent
 * corruption if multiple cores hit the crash path.
 *
 * In TZ: system_debug_reset_handler()
 * In TFA: system_debug_reset_handler() (name unchanged)
 */
	.type	system_debug_reset_handler, %function
system_debug_reset_handler:
	/*
	 * Save frame pointer and link register on current (IMEM) stack
	 */
	PUSH2	sp, x30, x29

	/*
	 * Save shared data structure
	 * x0 = shared data from bl31_get_sysdbg_shared_data()
	 */
	mov	x1, x0
	bl	sysdbg_save_shared_data

	/*
	 * Re-use exception vector table set by sysdbg_entry_handler
	 * (Already set VBAR_EL3 in sysdbg_imem_entry.S)
	 */

	/*
	 * Setup DDR stack using dual stack mechanism
	 *
	 * This alternates between two stacks to avoid clobbering if
	 * multiple cores hit crash path simultaneously.
	 */
	ldr	x0, =stack_counter
	ldr	x1, [x0]		/* Load counter value (0 or 1) */
	cbnz	x1, setup_stack1	/* If 1, use stack1 */

	/*
	 * Use stack0
	 */
	ldr	x2, =sysdbg_reset_stack0
	POP2	sp, x29, x30		/* Restore from IMEM stack */
	mov	sp, x2			/* Switch to DDR stack0 */
	PUSH2	sp, x30, x29		/* Save on DDR stack */
	b	jump_sysdbg

setup_stack1:
	/*
	 * Use stack1
	 */
	ldr	x2, =sysdbg_reset_stack1
	POP2	sp, x29, x30		/* Restore from IMEM stack */
	mov	sp, x2			/* Switch to DDR stack1 */
	PUSH2	sp, x30, x29		/* Save on DDR stack */

jump_sysdbg:
	/*
	 * Toggle stack counter for next time (0 <-> 1)
	 */
	eor	x1, x1, #1
	str	x1, [x0]

	/*
	 * Call main reset entry point
	 *
	 * This function calls system_debug_exit() to trigger the reset.
	 */
	bl	system_debug_reset_entry

	/*
	 * Restore frame pointer and link register
	 */
	POP2	sp, x29, x30

	ret

	.size	system_debug_reset_handler, . - system_debug_reset_handler

/*
 * Exception Vector Table
 *
 * Handles exceptions during crash path execution.
 * Set in VBAR_EL3 by sysdbg_entry_handler in sysdbg_imem_entry.S.
 *
 * All exceptions branch to error handler which:
 * - Captures exception context (ELR, FAR, ESR)
 * - Writes exception cookie
 * - Attempts to put DDR back in self-refresh
 * - Triggers reset
 */
	.p2align 7
sysdbg_vector_table:

/*
 * Current EL with SP0 - Synchronous exception
 */
sysdbg_exception_handler_elx_sync:
	mrs	x0, ELR_EL3		/* Exception return address */
	mrs	x1, FAR_EL3		/* Fault address */
	mrs	x2, ESR_EL3		/* Exception syndrome */
	b	sysdbg_err_fatal_handler

/*
 * Current EL with SP0 - IRQ
 */
	.p2align 7
sysdbg_exception_handler_elx_irq:
	b	sysdbg_exception_handler_elx_sync

/*
 * Current EL with SP0 - FIQ
 */
	.p2align 7
sysdbg_exception_handler_elx_fiq:
	b	sysdbg_exception_handler_elx_sync

/*
 * Current EL with SP0 - SError
 */
	.p2align 7
sysdbg_exception_handler_elx_syserr:
	b	sysdbg_exception_handler_elx_sync

/*
 * TODO: Complete vector table with all 16 entries
 *
 * ARMv8 requires 16 vector table entries (4 exception levels × 4 exception types)
 * For Phase 1, we only implement the essential ones above.
 * Phase 2 should add the remaining entries if needed.
 */

/*
 * Data Section (DDR)
 */
	.p2align 3

/*
 * Stack counter for dual stack mechanism
 * Toggles between 0 and 1 to alternate stacks
 */
stack_counter:
	.space	0x8

	.p2align 7

/*
 * DDR Stack 0 (2KB)
 */
sysdbg_reset_stack_space0:
	.space	0x800
sysdbg_reset_stack0:

/*
 * DDR Stack 1 (2KB)
 */
sysdbg_reset_stack_space1:
	.space	0x800
sysdbg_reset_stack1:
