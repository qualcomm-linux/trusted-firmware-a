/*
 * Copyright (c) 2023 ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <arch.h>
#include <stdbool.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <bl31qtilib_spd_agnostic.h>
#include <platform_def.h>
#include <qti_interrupt_props.h>
#include <qti_plat.h>

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	int core_linear_index;
	int cluster_id;

	cluster_id = (mpidr >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK;
	core_linear_index = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	core_linear_index += (PLAT_CORE_COUNT_PER_CLUSTER * cluster_id);

	if (core_linear_index < PLATFORM_CORE_COUNT) {
		return core_linear_index;
	}

	return -1;
}

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

/*
 * Function : find_cluster_id_by_mpidr
 * This function provides the cluster id to which the provided
 * mpidr belongs. Returns cluster id by reading aff1 bits in mpidr.
 * This function overrides the find_cluster_id definition in common
 * folder. This is because the Affinity bits depicting cluster id in
 * MPIDR for Hamoa are different compared to previous chipsets
 */
unsigned int find_cluster_id_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id;

	cluster_id = (mpidr >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK;
	return cluster_id;
}

void plat_error_handler(int error)
{
	bl31qtilib_spd_plat_error_handler(error);
	panic();
}

void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle)
{
	(void)handle;

	ERROR("EL3 unhandled interrupt %u\n", id);
}

#define QTI_FILTERED_INTERRUPT_PROPS_MAX			\
	((ARRAY_SIZE(qti_interrupt_props) > 0U) ?		\
	 ARRAY_SIZE(qti_interrupt_props) : 1U)

static interrupt_prop_t
qti_filtered_interrupt_props[QTI_FILTERED_INTERRUPT_PROPS_MAX];

static bool plat_qti_should_route_int_to_el3(enum qti_interrupt_owner owner)
{
	switch (owner) {
	case QTI_INTR_OWNER_EL3:
		return true;
	case QTI_INTR_OWNER_SECURE_DISPATCHER:
		return !bl31qtilib_spd_owns_interrupts();
	default:
		return false;
	}
}

const interrupt_prop_t *plat_qti_get_interrupt_props(unsigned int *num_props)
{
	unsigned int out = 0U;

	assert(num_props != NULL);

	for (unsigned int i = 0U; i < ARRAY_SIZE(qti_interrupt_props); i++) {
		if (!plat_qti_should_route_int_to_el3(
			    qti_interrupt_props[i].owner)) {
			continue;
		}

		assert(out < ARRAY_SIZE(qti_filtered_interrupt_props));
		qti_filtered_interrupt_props[out] = qti_interrupt_props[i].prop;
		out++;
	}

	*num_props = out;
	return qti_filtered_interrupt_props;
}
