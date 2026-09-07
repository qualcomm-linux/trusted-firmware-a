/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020 The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Changes from Qualcomm Technologies, Inc. are provided under the following
 * license: Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>

#include <platform_def.h>
#include <qti_plat.h>

const unsigned char qti_power_domain_tree_desc[] = PLAT_TREE_DESC;

/*******************************************************************************
 * This function returns the ARM default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return qti_power_domain_tree_desc;
}

uint32_t plat_qti_get_cluster_id_from_logical_cpu_num(uint32_t cpu_num)
{
	uint32_t core_start = 0;
	uint32_t cluster_count =
		qti_power_domain_tree_desc[PLAT_CLUSTER_COUNT_IDX];

	for (uint32_t cluster_id = 0; cluster_id < cluster_count;
	     cluster_id++) {
		uint32_t cores_in_cluster = qti_power_domain_tree_desc
			[PLAT_CLUSTER_CORE_COUNT_START_IDX + cluster_id];
		uint32_t core_end = core_start + cores_in_cluster - 1;

		if (cpu_num >= core_start && cpu_num <= core_end) {
			return cluster_id;
		}
		core_start = core_end + 1;
	}

	return QTI_INVALID_CLUSTER_ID;
}

uint32_t plat_qti_get_core_id_from_logical_cpu_num(uint32_t cpu_num)
{
	uint32_t core_start = 0;
	uint32_t cluster_count =
		qti_power_domain_tree_desc[PLAT_CLUSTER_COUNT_IDX];

	for (uint32_t cluster_id = 0; cluster_id < cluster_count;
	     cluster_id++) {
		uint32_t cores_in_cluster = qti_power_domain_tree_desc
			[PLAT_CLUSTER_CORE_COUNT_START_IDX + cluster_id];
		uint32_t core_end = core_start + cores_in_cluster - 1;

		if (cpu_num >= core_start && cpu_num <= core_end) {
			return cpu_num - core_start;
		}
		core_start = core_end + 1;
	}

	return QTI_INVALID_CPU_ID;
}

uint32_t plat_qti_logical_cpu_num_to_mpidr(uint32_t cpu_num)
{
	uint32_t cluster_id =
		plat_qti_get_cluster_id_from_logical_cpu_num(cpu_num);
	uint32_t core_id = plat_qti_get_core_id_from_logical_cpu_num(cpu_num);

	if ((cluster_id == QTI_INVALID_CLUSTER_ID) ||
	    (core_id == QTI_INVALID_CPU_ID)) {
		return QTI_INVALID_CPU_ID;
	}

	/*
	 * In NCC targets, affinity does not depend on the MT bit of MPIDR
	 * register instead it is fixed, AFF2 for cluster_id and AFF1 for
	 * core_id
	 */

#ifdef QTI_NCC_CPU
	return ((cluster_id << MPIDR_AFF2_SHIFT) |
		(core_id << MPIDR_AFF1_SHIFT));
#else
	if ((read_mpidr_el1() & MPIDR_MT_MASK) == MPIDR_MT_MASK) {
		return ((cluster_id << MPIDR_AFF2_SHIFT) |
			(core_id << MPIDR_AFF1_SHIFT));
	} else {
		return ((cluster_id << MPIDR_AFF1_SHIFT) |
			(core_id << MPIDR_AFF0_SHIFT));
	}
#endif
}
