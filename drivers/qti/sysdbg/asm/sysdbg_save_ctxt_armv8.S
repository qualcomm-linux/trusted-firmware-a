/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * System Debug - ARM Context Save (ARMv8-A)
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/monitor/src/arch/sysdbg_mon_arm.s
 *
 * This file saves all CPU architectural state (general purpose registers
 * and system registers) to a buffer in IMEM for crash dump analysis.
 *
 * Must execute from IMEM as DDR is in self-refresh at entry.
 */

#include <arch.h>
#include <asm_macros.S>

/*
 * Assembly macros for context save
 *
 * Ported from TZ sysdbg.inc - implements ascending stack operations
 * for saving register pairs.
 */

/*
 * PUSH8A - Push 8 registers using ascending stack
 * Stack grows upward (incrementing addresses)
 */
	.macro PUSH8A stack, reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8
	add	\stack, \stack, #64
	stp	\reg1, \reg2, [\stack, #-64]
	stp	\reg3, \reg4, [\stack, #-48]
	stp	\reg5, \reg6, [\stack, #-32]
	stp	\reg7, \reg8, [\stack, #-16]
	.endm

/*
 * PUSH4A - Push 4 registers using ascending stack
 */
	.macro PUSH4A stack, reg1, reg2, reg3, reg4
	add	\stack, \stack, #32
	stp	\reg1, \reg2, [\stack, #-32]
	stp	\reg3, \reg4, [\stack, #-16]
	.endm

	/*
	 * External symbol - return point in sysdbg_imem_entry.S
	 */
	.extern	sysdbg_save_ctxt_done

	/*
	 * Exported function
	 */
	.globl	sysdbg_save_ctxt_arm

	/*
	 * Place in IMEM section - must execute before DDR available
	 */
	.section .text_imem, "ax"
	.p2align 3

/*
 * sysdbg_save_ctxt_arm - Save ARM architectural context
 *
 * Saves all CPU state to buffer for crash dump analysis:
 * - General purpose registers x0-x29
 * - Link register x30
 * - Stack pointers for all exception levels
 * - Exception return addresses (ELR) for all exception levels
 * - Saved program status registers (SPSR) for all exception levels
 * - Current exception level
 * - CPU identification register (MIDR)
 *
 * Must execute from IMEM since DDR is in self-refresh (not accessible).
 *
 * Parameters:
 *   x0 - Pointer to context save buffer (sysdbg_cpu_ctxt_regs)
 *        This is an ascending stack - we write from low to high addresses
 *
 * Returns: Does not return normally - branches to sysdbg_save_ctxt_done
 *
 * Register State on Entry:
 *   x0 - Saved in TPIDR_EL0 by sysdbg_entry_handler
 *   x1 - Saved in TPIDR_EL1 by sysdbg_entry_handler
 *   x30 - Saved in TPIDRRO_EL0 by sysdbg_entry_handler
 *
 * In TZ: sysdbg_save_ctxt_arm()
 * In TFA: sysdbg_save_ctxt_arm() (name unchanged)
 */
	.type	sysdbg_save_ctxt_arm, %function
sysdbg_save_ctxt_arm:
	/*
	 * x0 points to context buffer (acts as ascending stack pointer)
	 * We'll use PUSH8A/PUSH4A macros to store registers
	 */

	/*
	 * Recover x1 from temporary storage
	 * It was saved by sysdbg_entry_handler before calling us
	 */
	mrs	x1, TPIDR_EL1

	/*
	 * Save general purpose registers x0-x28
	 * Note: x0 still holds buffer pointer, x1 recovered from TPIDR_EL1
	 */
	PUSH8A	x0, x0, x1, x2, x3, x4, x5, x6, x7
	/* x8 used below for system registers */
	PUSH8A	x0, x9, x10, x11, x12, x13, x14, x15, x16
	PUSH8A	x0, x17, x18, x19, x20, x21, x22, x23, x24
	PUSH4A	x0, x25, x26, x27, x28

	/*
	 * Recover saved registers from temporary storage
	 * x30 (link register) and original x0 were saved by sysdbg_entry_handler
	 */
	mrs	x1, TPIDRRO_EL0		/* x1 = saved x30 */
	mrs	x2, TPIDR_EL0		/* x2 = saved x0 */

	/*
	 * Collect system registers for crash analysis
	 */
	mrs	x3, CURRENTEL		/* Current exception level */
	add	x4, sp, #0		/* Current stack pointer (EL3) */
	mrs	x5, ELR_EL3		/* Exception link register EL3 */
	mrs	x6, SPSR_EL3		/* Saved program status EL3 */
	mrs	x7, SP_EL2		/* Stack pointer EL2 */
	mrs	x8, ELR_EL2		/* Exception link register EL2 */
	mrs	x9, SPSR_EL2		/* Saved program status EL2 */
	mrs	x10, SP_EL1		/* Stack pointer EL1 */
	mrs	x11, ELR_EL1		/* Exception link register EL1 */
	mrs	x12, SPSR_EL1		/* Saved program status EL1 */
	mrs	x13, SP_EL0		/* Stack pointer EL0 */
	mrs	x14, MIDR_EL1		/* CPU ID register */
	mov	x15, #0			/* Reserved / L2MERRSR placeholder */

	/*
	 * TZ code has commented-out logic to read CPUMERRSR_EL1 and L2MERRSR_EL1
	 * for Kryo Gold CPUs. These are CPU error status registers.
	 * For Phase 1, we skip this as it's CPU-specific debug data.
	 *
	 * Original TZ logic (commented out in TZ too):
	 *   UBFX x14, x14, #MIDR_PARTNUM_BITFLDSTART, #MIDR_PARTNUM_BITFLDWIDTH
	 *   CMP x14, #KRYOB_GOLD_CPU_PARTNUM
	 *   CSEL x14, x14, x15, NE
	 *   BEQ skip_l1merr
	 *   MRS x14, CPUMERRSR_EL1
	 * skip_l1merr:
	 *   MRS x15, L2MERRSR_EL1
	 *
	 * We preserve x14 (MIDR) and set x15=0 for now.
	 */

	/*
	 * Save frame pointer (x29), recovered registers, and system state
	 */
	PUSH8A	x0, x29, x1, x2, x3, x4, x5, x6, x7
	PUSH8A	x0, x8, x9, x10, x11, x12, x13, x14, x15

	/*
	 * Context save complete - branch to next phase
	 * This does not return - continues in sysdbg_entry_handler
	 */
	bl	sysdbg_save_ctxt_done

	.size	sysdbg_save_ctxt_arm, . - sysdbg_save_ctxt_arm
