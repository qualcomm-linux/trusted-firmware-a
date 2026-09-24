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

#include <clkdom_config.h>

static spinlock_t clkdom_lock;

static int clkdom_enable(enum plat_clock_domain_id id)
{
	int ret = SCMI_E_SUCCESS;

	spin_lock(&clkdom_lock);

	if (plat_clkdom_init_status[id] != 0U) {
		ret = scmi_clock_config_set(qti_scmi_get_channel(), id,
                                   SCMI_CLOCK_CONFIG_SET_ENABLE, 0U);
        	if (ret == SCMI_E_SUCCESS) {
                	plat_clkdom_init_status[id] = 1U;
        	} else {
                	ERROR("Failed to enable clock domain %u (%d)\n", id, ret);
        	}
	}

	spin_unlock(&clkdom_lock);
	return ret;
}

int cpuss_clocks_enable(void)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int core_pos = plat_qti_core_pos_by_mpidr(mpidr);
	unsigned int core_mask = 1U << core_pos;
	enum plat_clock_domain_id cpu_id = CD_MAX;
	enum plat_clock_domain_id l3_id = CD_MAX;
	int ret;

	for (unsigned int i = 0U; i < ARRAY_SIZE(plat_clkdom_parent_maps); i++) {
		if ((plat_clkdom_parent_maps[i].core_mask & core_mask) != 0U) {
			cpu_id = plat_clkdom_parent_maps[i].cpu_id;
			l3_id = plat_clkdom_parent_maps[i].l3_id;
			break;
		}
	}

	if ((cpu_id == CD_MAX) || (l3_id == CD_MAX)) {
		ERROR("No clock domain mapping found for core %u\n", core_pos);
		return SCMI_E_INVALID_PARAM;
	}

	ret = clkdom_enable(l3_id);
	if(ret != SCMI_E_SUCCESS) {
                ERROR("L3 Clock domain initialization failed for core %u\n", core_pos);
        }

	ret = clkdom_enable(cpu_id);
        if(ret != SCMI_E_SUCCESS) {
                ERROR("CPU Clock domain initialization failed for core %u\n", core_pos);
        }

	return ret;
}
