/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>
#include <lib/spinlock.h>

#include "qti_plat.h"

#include <ckdom_config.h>

static spinlock_t clkdom_lock;

static int clkdom_enable(enum clock_domain_id id)
{
	int ret;

	spin_lock(&clkdom_lock);

	if (clkdom_init_status[id] != 0U) {
		spin_unlock(&clkdom_lock);
		return SCMI_E_SUCCESS;
	}

	ret = scmi_clock_config_set(qti_scmi_get_channel(), id,
				   SCMI_CLOCK_CONFIG_SET_ENABLE, 0U);
	if (ret == SCMI_E_SUCCESS) {
		clkdom_init_status[id] = 1U;
	} else {
		ERROR("Failed to enable clock domain %u (%d)\n", id, ret);
	}

	spin_unlock(&clkdom_lock);
	return ret;
}

void clkdom_init(void)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int core_pos = plat_qti_core_pos_by_mpidr(mpidr);
	unsigned int core_mask;
	enum clock_domain_id cpu_id = CD_MAX;
	enum clock_domain_id l3_id = CD_MAX;

	if (core_pos >= (sizeof(core_mask) * 8U)) {
		ERROR("Invalid linear core position %u\n", core_pos);
		return;
	}

	core_mask = 1U << core_pos;
	for (unsigned int i = 0U; i < CD_MAX; i++) {
		if ((clkdom_cpumasks[i].mask & core_mask) != 0U) {
			cpu_id = clkdom_cpumasks[i].id;
			break;
		}
	}

	if (cpu_id == CD_MAX) {
		ERROR("No clock domain found for core position %u\n", core_pos);
		return;
	}

	for (unsigned int i = 0U; i < ARRAY_SIZE(clkdom_parent_maps); i++) {
		if (clkdom_parent_maps[i].cpu_id == cpu_id) {
			l3_id = clkdom_parent_maps[i].l3_id;
			break;
		}
	}

	if (l3_id == CD_MAX) {
		ERROR("No L3 parent found for clock domain %u\n", cpu_id);
		return;
	}

	if (clkdom_enable(l3_id) != SCMI_E_SUCCESS ||
		clkdom_enable(cpu_id) != SCMI_E_SUCCESS) {
		ERROR("Clock domain initialization failed for core %u\n", core_pos);
	}
}
