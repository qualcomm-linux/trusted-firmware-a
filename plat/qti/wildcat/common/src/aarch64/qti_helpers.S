/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/gicv3.h>
#include <drivers/console.h>

#include <platform_def.h>

	.globl	plat_my_core_pos
	.weak	plat_qti_core_pos_by_mpidr
	.globl	plat_reset_handler
	.globl	plat_panic_handler
	.extern	bl31qtilib_spd_panic_handler
	.extern  sysdbg_reset_check
	.extern  sysdbg_entry_handler

	/* --------------------------------------------------------------------
	 *  unsigned int plat_qti_core_pos_by_mpidr(uint64_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function:
	 *  CorePos = (ClusterId * Cluster0_CoreCount) + CoreId
	 *    ClusterId = MPIDR_EL1[23:15]
	 *    CoreId    = MPIDR_EL1[15:8]
	 *  Clobbers: x0, x1, x2.
	 *
	 *  !! IMPORTANT NOTE
	 *  !! qteed_el3_err_fatal currently depends on this only
	 *  !! clobbering registers x0--x2
	 * --------------------------------------------------------------------
	 */
func plat_qti_core_pos_by_mpidr
	/* cluster_id = mpidr >> MPIDR_AFF2_SHIFT */
	lsr	x1, x0, #MPIDR_AFF2_SHIFT
	/* cluster_id &= MPIDR_AFFLVL_MASK */
	and	x1, x1, #MPIDR_AFFLVL_MASK

	/* core_index = mpidr >> MPIDR_AFF1_SHIFT */
	lsr	x2, x0, #MPIDR_AFF1_SHIFT
	/* core_index &= MPIDR_AFFLVL_MASK */
	and	x2, x2, #MPIDR_AFFLVL_MASK

	mov	x0, #PLAT_CORE_COUNT_PER_CLUSTER
	/* core_index += (PLAT_CLUSTER0_CORE_COUNT * cluster_id) */
	madd	x0, x0, x1, x2
	ret
endfunc plat_qti_core_pos_by_mpidr

	/* --------------------------------------------------------------------
	 * void plat_panic_handler(void)
	 * calls SDI and reset system
	 * --------------------------------------------------------------------
	 */
func plat_panic_handler
	b	bl31qtilib_spd_panic_handler
endfunc plat_panic_handler
	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_qti_core_pos_by_mpidr()
	 *  definition to get the index of the calling CPU
	 *  Clobbers: x0 & x1.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_qti_core_pos_by_mpidr
endfunc plat_my_core_pos

func plat_reset_handler
	/* save the lr */
	mov	x18, x30
	bl	plat_set_my_stack
	/* Invoke SDI flow if in pass-1 */
	bl      sysdbg_reset_check
	cmp     x0, #0
	/* sysdbg_entry_handler does not return - enters debug mode */
	bne     sysdbg_entry_handler
	/* Execute CPUSS boot set up on every core. */
	bl	plat_qti_cpu_boot_setup

	ret	x18
endfunc plat_reset_handler
