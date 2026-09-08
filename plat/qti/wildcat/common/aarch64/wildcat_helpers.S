/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

#include <platform_def.h>

	.globl	plat_my_core_pos
	.globl	plat_qti_core_pos_by_mpidr

	/* --------------------------------------------------------------------
	 *  unsigned int plat_qti_core_pos_by_mpidr(uint64_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function:
	 *  CorePos = (ClusterId * PLAT_CORE_COUNT_PER_CLUSTER) + CoreId
	 *    ClusterId = MPIDR_EL1[23:16]  (AFF2)
	 *    CoreId    = MPIDR_EL1[15:8]   (AFF1)
	 *  Clobbers: x0, x1, x2.
	 * --------------------------------------------------------------------
	 */
func plat_qti_core_pos_by_mpidr
	lsr	x1, x0, #MPIDR_AFF2_SHIFT	// cluster_id = mpidr >> MPIDR_AFF2_SHIFT
	and	x1, x1, #MPIDR_AFFLVL_MASK	// cluster_id &= MPIDR_AFFLVL_MASK

	lsr	x2, x0, #MPIDR_AFF1_SHIFT	// core_index = mpidr >> MPIDR_AFF1_SHIFT
	and	x2, x2, #MPIDR_AFFLVL_MASK	// core_index &= MPIDR_AFFLVL_MASK

	mov	x0, #PLAT_CORE_COUNT_PER_CLUSTER
	madd    x0, x0, x1, x2			// core_index += (PLAT_CORE_COUNT_PER_CLUSTER * cluster_id)
	ret
endfunc plat_qti_core_pos_by_mpidr

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_qti_calc_core_pos()
	 *  definition to get the index of the calling CPU
	 *  Clobbers: x0, x1, x2.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_qti_core_pos_by_mpidr
endfunc plat_my_core_pos
