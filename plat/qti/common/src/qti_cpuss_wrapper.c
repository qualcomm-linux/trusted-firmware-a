/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <lib/spinlock.h>

/* Platform-specific raw cluster DSU register writes.
 * Must be called before MMU is enabled. */
void plat_cluster_init(void);

/* Per-cluster init bitmask: bit N = cluster N fully initialized.
 * Can be cleared from LPM exit to allow re-initialization per cluster. */
static uint32_t plat_cluster_init_done;

/* Spinlock protecting plat_cluster_init_done read-modify-write.
 * Ensures only one CPU per cluster runs plat_cluster_init(). */
static spinlock_t cluster_init_lock;

/* plat_cluster_init_wrapper() - Once-per-cluster DSU initialization. */
void plat_cluster_init_wrapper(void)
{
	uint32_t cluster_id = (read_mpidr_el1() >> MPIDR_AFF2_SHIFT) &
			       MPIDR_AFFLVL_MASK;
	uint32_t bit = 1U << cluster_id;

	spin_lock(&cluster_init_lock);

	if (!(plat_cluster_init_done & bit)) {
		plat_cluster_init();            /* DSU register writes + dsb sy */
		plat_cluster_init_done |= bit;  /* mark done AFTER init completes */
	}

	spin_unlock(&cluster_init_lock);
}