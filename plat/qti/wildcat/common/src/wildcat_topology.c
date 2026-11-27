/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <common/debug.h>
#include <drivers/qti/chipinfo/chipinfo.h>
#include <lib/utils_def.h>

#include <platform_def.h>
#include <wildcat_plat.h>

#ifndef QTI_CPU_TOPOLOGY_FROM_FUSES
#define QTI_CPU_TOPOLOGY_FROM_FUSES	0
#endif

/*
 * Default (fully populated) topology. With QTI_CPU_TOPOLOGY_FROM_FUSES this
 * is trimmed at PSCI setup time to drop the CPU cores fused off on this SKU.
 */
static unsigned char qti_power_domain_tree_desc[] = PLAT_TREE_DESC;

#if QTI_CPU_TOPOLOGY_FROM_FUSES
/* Per-cluster bitmask of fused-off cores excluded from the PSCI topology. */
static uint32_t qti_disabled_core_mask[PLAT_CLUSTER_COUNT];

/*
 * Read the per-cluster disabled-core masks reported by XBL and remove those
 * cores from the PSCI power domain tree, so that PSCI (e.g. the "all CPUs ON"
 * check for PSCI_SET_SUSPEND_MODE(OSI)) only accounts for cores that exist.
 *
 * PSCI requires linear core indices to be dense and grouped by cluster, and
 * the core index is derived from MPIDR as (cluster * cores_per_cluster + core)
 * by plat_my_core_pos(). The two only agree when every disabled core comes
 * after every enabled core in that order (e.g. last core(s) of the last
 * cluster). Any other layout would require an MPIDR remap table, so it is
 * rejected and the full topology is kept.
 */
static void qti_apply_cpu_fuses(void)
{
	const uint32_t valid = (uint32_t)(BIT_32(PLAT_CORE_COUNT_PER_CLUSTER) - 1U);
	uint32_t mask[PLAT_CLUSTER_COUNT] = { 0U };
	uint32_t cluster, core, cores, num_cores = 0U, num_clusters = 0U;
	bool seen_disabled = false;
	bool any_disabled = false;

	for (cluster = 0U; cluster < PLAT_CLUSTER_COUNT; cluster++) {
		if (chipinfo_get_disabled_cpus(cluster, &mask[cluster]) !=
		    CHIPINFO_SUCCESS) {
			mask[cluster] = 0U;
		}

		if ((mask[cluster] & ~valid) != 0U) {
			WARN("Cluster %u: ignoring disabled-core bits 0x%x\n",
			     cluster, mask[cluster] & ~valid);
			mask[cluster] &= valid;
		}

		if (mask[cluster] != 0U) {
			NOTICE("Cluster %u: disabled core mask 0x%x\n",
			       cluster, mask[cluster]);
			any_disabled = true;
		}
	}

	if (!any_disabled) {
		return;
	}

	/* Enabled cores must form a contiguous prefix of the linear order. */
	for (cluster = 0U; cluster < PLAT_CLUSTER_COUNT; cluster++) {
		for (core = 0U; core < PLAT_CORE_COUNT_PER_CLUSTER; core++) {
			if ((mask[cluster] & BIT_32(core)) != 0U) {
				seen_disabled = true;
			} else if (seen_disabled) {
				WARN("Unsupported disabled-core layout, keeping full CPU topology\n");
				return;
			}
		}
	}

	/* Core 0 of cluster 0 is the boot core and must be present. */
	if ((mask[0] & BIT_32(0)) != 0U) {
		WARN("Boot core reported disabled, keeping full CPU topology\n");
		return;
	}

	for (cluster = 0U; cluster < PLAT_CLUSTER_COUNT; cluster++) {
		cores = PLAT_CORE_COUNT_PER_CLUSTER -
			(uint32_t)__builtin_popcount(mask[cluster]);
		qti_power_domain_tree_desc
			[PLAT_CLUSTER_CORE_COUNT_START_IDX + cluster] =
			(unsigned char)cores;
		if (cores != 0U) {
			num_clusters = cluster + 1U;
		}
		num_cores += cores;
		qti_disabled_core_mask[cluster] = mask[cluster];
	}

	/* Trailing clusters with no enabled cores are dropped entirely. */
	qti_power_domain_tree_desc[PLAT_CLUSTER_COUNT_IDX] =
		(unsigned char)num_clusters;

	NOTICE("PSCI topology: %u cores in %u clusters\n",
	       num_cores, num_clusters);
}

bool plat_qti_is_core_disabled(unsigned int core_pos)
{
	unsigned int cluster = core_pos / PLAT_CORE_COUNT_PER_CLUSTER;
	unsigned int core = core_pos % PLAT_CORE_COUNT_PER_CLUSTER;

	if (cluster >= PLAT_CLUSTER_COUNT) {
		return true;
	}

	return (qti_disabled_core_mask[cluster] & BIT_32(core)) != 0U;
}
#endif /* QTI_CPU_TOPOLOGY_FROM_FUSES */

/*******************************************************************************
 * This function returns the ARM default topology tree information.
 *
 * It is called once from psci_setup(), which runs after bl31_platform_setup()
 * has initialised ChipInfo, so the SKU's fuse information is available here.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
#if QTI_CPU_TOPOLOGY_FROM_FUSES
	qti_apply_cpu_fuses();
#endif
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
