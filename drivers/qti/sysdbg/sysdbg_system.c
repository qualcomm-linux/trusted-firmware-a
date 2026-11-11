/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/spinlock.h>
#include <lib/utils_def.h>

#include <sysdbg_system.h>
#include <sysdbg_target.h>

static spinlock_t mem_lock;
static spinlock_t q6_lock;

spinlock_t *get_mem_lock(void)
{
	return &mem_lock;
}

spinlock_t *get_q6_lock(void)
{
	return &q6_lock;
}

uint32_t sysdbg_get_corecount(subsys_type subsystem)
{
	uint32_t corecount = 0;
	uint32_t i = 0;

	SysdbgTargetInfoUpdate();

	if (!(subsystem < SUBSYS_MAX)) {
		return 0;
	}

	while ((i < MAX_CLUSTERS) &&
	       (target_info.subsys[subsystem].clusters[i].cpuname != UNKNOWN_CPU)) {
		corecount += target_info.subsys[SUBSYS_APPS].clusters[i].cores;
		i++;
	}

	return corecount;
}

uint32_t sysdbg_get_cluster_corecount(subsys_type subsystem, uint32_t cluster)
{
	uint32_t clusters;

	SysdbgTargetInfoUpdate();

	if (!(subsystem < SUBSYS_MAX)) {
		return 0;
	}

	clusters = ARRAY_SIZE(target_info.subsys[subsystem].clusters);

	if (cluster < clusters) {
		return target_info.subsys[subsystem].clusters[cluster].cores;
	}

	return 0;
}

uint32_t sysdbg_get_clusternum(subsys_type subsystem, uint32_t cpu)
{
	uint32_t cluster = 0;
	uint32_t count = 0;
	uint32_t clusters;
	uint32_t i;

	SysdbgTargetInfoUpdate();

	clusters = ARRAY_SIZE(target_info.subsys[subsystem].clusters);

	for (i = 0; i < (clusters - 1); i++) {
		count += target_info.subsys[subsystem].clusters[i].cores;

		if ((cpu >= target_info.subsys[subsystem].clusters[i].cores) &&
		    (cpu < (count + target_info.subsys[subsystem].clusters[i + 1].cores))) {
			cluster = i + 1;
		}
	}

	return cluster;
}

subsys_type get_current_subsys(void)
{
	return SUBSYS_APPS;
}

uint32_t get_current_cluster(subsys_type subsystem)
{
	(void)subsystem;
	return 0;
}

uint32_t get_current_cpu_seq(subsys_type subsystem)
{
	(void)subsystem;
	return 0;
}

uint32_t xlate_cluscpu_seqcpu(subsys_type subsystem, uint32_t cpu, uint32_t cluster)
{
	uint32_t i;
	uint32_t seqcpu = cpu;

	SysdbgTargetInfoUpdate();

	for (i = 0; i < cluster; i++) {
		seqcpu += target_info.subsys[subsystem].clusters[i].cores;
	}

	return seqcpu;
}
