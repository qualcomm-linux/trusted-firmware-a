/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

#include <platform_def.h>

	.globl	plat_my_core_pos
	.globl	plat_qti_core_pos_by_mpidr
	.globl	plat_reset_handler
#ifdef QTI_USE_SYSDBG
	.extern	sysdbg_reset_check
	.extern	sysdbg_entry_handler
#endif

	/* -----------------------------------------------------
	 *  unsigned int plat_qti_core_pos_by_mpidr(uint64_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function:
	 *  CorePos = (ClusterId * 4) + CoreId
	 *  - In ARM v8   (MPIDR_EL1[24]=0)
	 *    ClusterId = MPIDR_EL1[15:8]
	 *    CoreId    = MPIDR_EL1[7:0]
	 *  - In ARM v8.1 (MPIDR_EL1[24]=1)
	 *    ClusterId = MPIDR_EL1[23:16]
	 *    CoreId    = MPIDR_EL1[15:8]
	 *  Clobbers: x0 & x1.
	 * -----------------------------------------------------
	 */
func plat_qti_core_pos_by_mpidr
	mrs	x1, mpidr_el1
	tst	x1, #MPIDR_MT_MASK
	beq	plat_qti_core_pos_by_mpidr_no_mt
	/* Right shift mpidr by one affinity level when MT=1. */
	lsr	x0, x0, #MPIDR_AFFINITY_BITS
plat_qti_core_pos_by_mpidr_no_mt:
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_qti_core_pos_by_mpidr

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_qti_calc_core_pos()
	 *  definition to get the index of the calling CPU
	 *  Clobbers: x0 & x1.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_qti_core_pos_by_mpidr
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 * Platform reset handler invoked from the common EL3
	 * reset path.
	 *
	 * Platforms may extend this handler with reset-time
	 * processing that must execute before normal BL31 runtime
	 * initialization is resumed.
	 *
	 * SDI: if this reset is a debug (crash) reset, branch to
	 * the SDI entry handler, which does not return.
	 * -----------------------------------------------------
	 */
func plat_reset_handler
#ifdef QTI_USE_SYSDBG
	/* save the lr */
	mov	x18, x30
	/* NULL frame pointer = bottom of stack */
	mov	x29, xzr

	bl	sysdbg_reset_check
	cbz	x0, 1f
	b	sysdbg_entry_handler
1:
	ret	x18
#else
	ret
#endif
endfunc plat_reset_handler
