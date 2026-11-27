/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * System Debug - BL31 Entry Handler (Bruin/Shikra Architecture)
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/monitor/src/sysdbg_mon_bruin.s
 *
 * This file contains the main crash reset entry point and helper functions.
 * Must execute from IMEM as DDR is in self-refresh at entry.
 */

#include <arch.h>
#include <asm_macros.S>
#include <sysdbg_hwio.h>

/* SYSTEM_SHAREDIMEM_BASE (execution state flag) comes from sysdbg_crumbs.h */
#include <drivers/qti/sysdbg/sysdbg_crumbs.h>
#define TZBSP_SHARED_IMEM_TZ_AREA_BASE		(SYSTEM_SHAREDIMEM_BASE + 0x734)
#define TZBSP_SHARED_EXECUTION_PASS_STATE	(TZBSP_SHARED_IMEM_TZ_AREA_BASE + 0x74)

/* SCR_EL3 default value */
#define SCR_EL3_DEFAULT				(SCR_NS_BIT | SCR_RW_BIT | SCR_SMD_BIT)

/* SPSR_EL3 value for EL3h mode with all exceptions masked
 * Using TFA SPSR_64 macro: SPSR_64(el=3, sp=1, daif=0xF)
 * Equivalent to TZ: SPSR_MODE_EL3h | SPSR_I_BIT | SPSR_F_BIT | SPSR_A_BIT | SPSR_D_BIT
 */
#define SPSR_EL3_VALUE				SPSR_64(MODE_EL3, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS)

	/*
	 * External function declarations
	 */
	.extern	sysdbg_save_ctxt_arm
	.extern	system_debug_bl31_entry
	.extern	system_debug_reset_handler
	.extern	bl31_get_sysdbg_shared_data
	.extern	sysdbg_save_err
	.extern	sysdbg_cpu_ctxt_regs
	.extern	sysdbg_vector_table

	/*
	 * Exported functions
	 */
	.globl	sysdbg_entry_handler
	.globl	sysdbg_save_ctxt_done
	.globl	sysdbg_set_execution_state
	.globl	sysdbg_check_execution_state
	.globl	sysdbg_err_fatal

	/*
	 * Place all code in IMEM section - must execute before DDR available
	 */
	.section .text_imem, "ax"
	.p2align 3

/*
 * sysdbg_entry_handler - Main crash reset entry point
 *
 * This function is called when a crash/debug reset is detected.
 * It saves CPU state, sets up the crash path environment, and brings
 * DDR out of self-refresh.
 *
 * Must execute from IMEM since DDR is in self-refresh (not accessible).
 *
 * Called from: plat_reset_handler (qti_helpers.S) after sysdbg_reset_check()
 *              returns 1 indicating crash reset.
 *
 * Tasks:
 *   (1) Save registers that will be clobbered
 *   (2) Set execution state flag in SHARED_IMEM
 *   (3) Setup exception vector table
 *   (4) Save all CPU context (GP registers, system registers)
 *   (5) Setup IMEM stack
 *   (6) Call system_debug_bl31_entry() to bring DDR out of self-refresh
 *   (7) Continue to system_debug_reset_handler() for debug collection
 *
 * This function does not return normally - it ends with SoC reset.
 */
	.type	sysdbg_entry_handler, %function
sysdbg_entry_handler:
	/*
	 * Clear exclusive monitor
	 */
	clrex

	/*
	 * Save registers we're about to clobber
	 * Use thread ID registers as temporary storage since they're
	 * not used in EL3 crash path.
	 */
	msr	TPIDR_EL0, x0		/* Save x0 */
	msr	TPIDR_EL1, x1		/* Save x1 */
	msr	TPIDRRO_EL0, x30	/* Save x30 (link register) */

	/*
	 * Set execution state flag in SHARED_IMEM
	 *
	 * This flag is used by the exception vector table to determine
	 * whether we're in debug first pass or normal operation.
	 *
	 * 0 = Normal Operation
	 * 1 = Secure Watchdog Pass 1 (crash path)
	 *
	 * This is needed because vector table is shared to reduce IMEM usage.
	 */
	mov	x0, #1
	bl	sysdbg_set_execution_state

	/*
	 * Set up exception vector table for crash path
	 */
	ldr	x1, =sysdbg_vector_table
	msr	VBAR_EL3, x1

	/*
	 * Load address of CPU context buffer
	 * This buffer will hold all saved registers
	 */
	ldr	x0, =sysdbg_cpu_ctxt_regs

	/*
	 * Save all CPU context (registers, system state)
	 * This preserves the state at time of crash for later analysis
	 */
	bl	sysdbg_save_ctxt_arm

	/*
	 * Entry point after context save completes
	 * Label used by sysdbg_save_ctxt_arm to return here
	 */
	.type	sysdbg_save_ctxt_done, %function
sysdbg_save_ctxt_done:

	/*
	 * Setup IMEM stack for crash path execution
	 * Switch from whatever stack we had to dedicated IMEM stack
	 */
	ldr	x0, =sysdbg_stack
	mov	sp, x0

	/*
	 * Call main entry point - brings DDR out of self-refresh
	 *
	 * CRITICAL: After this call returns, DDR is accessible.
	 * Before this call, only IMEM/OCIMEM can be used.
	 *
	 * In TZ: system_debug_mon_entry()
	 * In TFA: system_debug_bl31_entry()
	 */
	bl	system_debug_bl31_entry

	/*
	 * DDR is now accessible!
	 * Setup EL3 system registers for proper state
	 */

	/* Set SCR_EL3 to default value */
	mov	x0, #SCR_EL3_DEFAULT
	msr	SCR_EL3, x0

	/* Set SPSR_EL3 for EL3h mode with interrupts masked */
	mov	x0, #SPSR_EL3_VALUE
	msr	SPSR_EL3, x0

	/*
	 * Build the shared data (DDR self-refresh entry and IMEM stack) and
	 * pass its address to system_debug_reset_handler() in x0, which hands
	 * it to sysdbg_save_shared_data().
	 */
	ldr	x0, =sysdbg_stack
	bl	bl31_get_sysdbg_shared_data

	/*
	 * Call reset handler - performs debug data collection
	 *
	 * This function:
	 * 1. Sets up DDR stack
	 * 2. Calls system_debug_reset_entry()
	 * 3. Executes pass functions (or stubs for Phase 1)
	 * 4. Calls system_debug_exit()
	 * 5. Triggers SoC reset
	 *
	 * This function NEVER RETURNS - system resets to PASS2
	 */
	bl	system_debug_reset_handler

	/*
	 * Should never reach here - system_debug_reset_handler() resets SoC
	 * If we do reach here, hang in infinite loop
	 */
1:
	wfi
	b	1b

	.size	sysdbg_entry_handler, . - sysdbg_entry_handler

/*
 * sysdbg_set_execution_state - Set execution state flag
 *
 * Writes execution state flag to SHARED_IMEM. This flag indicates
 * whether we're in normal operation or crash/debug path.
 *
 * Parameters:
 *   x0 - State value (0 = normal, 1 = crash path)
 *
 * Returns: None
 *
 * Registers modified: x1
 *
 * In TZ: mon_sysdbg_set_execution_state()
 * In TFA: sysdbg_set_execution_state()
 */
	.type	sysdbg_set_execution_state, %function
sysdbg_set_execution_state:
	ldr	x1, =TZBSP_SHARED_EXECUTION_PASS_STATE
	str	x0, [x1]
	ret
	.size	sysdbg_set_execution_state, . - sysdbg_set_execution_state

/*
 * sysdbg_check_execution_state - Read execution state flag
 *
 * Reads execution state flag from SHARED_IMEM.
 *
 * Parameters: None
 *
 * Returns:
 *   x0 - State value (0 = normal, 1 = crash path)
 *
 * Registers modified: x0
 *
 * In TZ: mon_sysdbg_check_execution_state()
 * In TFA: sysdbg_check_execution_state()
 */
	.type	sysdbg_check_execution_state, %function
sysdbg_check_execution_state:
	ldr	x0, =TZBSP_SHARED_EXECUTION_PASS_STATE
	ldr	x0, [x0]
	ret
	.size	sysdbg_check_execution_state, . - sysdbg_check_execution_state

/*
 * sysdbg_err_fatal - Fatal error handler
 *
 * **MUST EXECUTE FROM IMEM**
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/monitor/src/sysdbg_mon_bruin.s
 * (exception vector calls mon_sysdbg_save_err)
 *
 * Handles fatal errors and exceptions during crash path execution.
 * This is the assembly entry point that calls the C handler.
 *
 * Called from: Exception vector table (set in sysdbg_entry_handler)
 *
 * Tasks:
 *   1. Call C handler sysdbg_save_err() to handle exception
 *   2. If C handler returns (should not), hang
 *
 * This function never returns - system resets.
 */
	.type	sysdbg_err_fatal, %function
sysdbg_err_fatal:
	/*
	 * Call C handler to save error state and trigger reset
	 *
	 * sysdbg_save_err() will:
	 *   - Save PIMEM registers
	 *   - Write EXCEPTION cookie
	 *   - Put DDR in self-refresh
	 *   - Trigger reset
	 */
	bl	sysdbg_save_err

	/*
	 * Should never reach here - sysdbg_save_err() resets SoC
	 * If we do reach here, hang in infinite loop
	 */
1:
	wfi
	b	1b
	.size	sysdbg_err_fatal, . - sysdbg_err_fatal

/*
 * Data section - IMEM data
 *
 * Data used during crash path before DDR is available.
 */
	.section .data_imem, "aw"
	.p2align 7

/*
 * CPU Context Save Buffer
 *
 * Buffer to save all CPU registers during crash.
 * Size: ~2KB (enough for all GP + system registers)
 */
	.globl	sysdbg_cpu_ctxt_regs
sysdbg_cpu_ctxt_regs:
	.space	0x800		/* 2KB for CPU context */

/*
 * IMEM Stack
 *
 * Stack used during crash path before DDR is available.
 * 256 bytes should be sufficient for IMEM phase.
 */
	.globl	sysdbg_stack
sysdbg_stack_space:
	.space	0x100		/* 256 bytes stack space */
sysdbg_stack:			/* Stack grows down, so label at end */
