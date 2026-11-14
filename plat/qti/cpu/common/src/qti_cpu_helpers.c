/*
 * Copyright (c) 2023-2025, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2023-2025, Qualcomm Technologies, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Common CPU Helper functions */

#include <arch_helpers.h>

/*
 * Function : find_cluster_id
 * This function provides the cluster id to which the core belongs
 * Returns cluster id by reading aff1 bits in mpidr.
 * This function overrides the find_cluster_id definition in common
 * folder. This is because the Affinity bits depicting cluster id in
 * MPIDR for Pakala are different compared to previous chipsets
 */
unsigned int find_cluster_id(void)
{
	unsigned long cluster_id;
	unsigned long mpidr;

	mpidr = read_mpidr_el1();
	cluster_id = (mpidr >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK;
	return cluster_id;
}
