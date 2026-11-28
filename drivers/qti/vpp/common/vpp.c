/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * VPP top-level: fuse readout, voltage-plan selection, and virtual-corner
 * interpolation for every rail/clock domain, merged into the public vpp_t
 * plan clients fetch through vpp_get_plan().
 */

#include <stdint.h>
#include <string.h>

#include <drivers/qti/vpp/vpp.h>

#include "vpp_voltage_plan.h"

static vpp_device_t vpp_device;

static void vpp_init_osm_cprh_plan(vpp_t *vpp, vpp_internal_t *voltage_plan)
{
	uint16_t cnt;
	vpp_voltage_settings_t *vc_setting;

	for (cnt = 0; voltage_plan->osm_setting[cnt].vlvl != VPP_VLVL_INVALID; cnt++) {
		vc_setting = &voltage_plan->voltage_setting[voltage_plan->osm_setting[cnt].vc];
		memcpy(&vpp->osm[cnt], &voltage_plan->osm_setting[cnt], sizeof(vpp_osm_t));
		memcpy(&vpp->cprh[cnt], &vc_setting->cprh, sizeof(vpp_cprh_t));
		vpp->cnt++;
	}
}

const vpp_device_t *vpp_get_device_info(void)
{
	return &vpp_device;
}

const vpp_t *vpp_get_plan(const char *rail, const char *clk)
{
	uint32_t idx;

	for (idx = 0; idx < VPP_CLK_TOTAL; idx++) {
		if (!strcmp(vpp_table[idx].rail_name, rail) &&
		    !strcmp(vpp_table[idx].clk_name, clk)) {
			return &vpp_table[idx];
		}
	}

	return NULL;
}

void vpp_init(void)
{
	vpp_internal_t voltage_plan;
	uint32_t idx;

	vpp_init_device_info(&vpp_device);

	for (idx = 0; idx < VPP_CLK_TOTAL; idx++) {
		/* pick the voltage plan matching this device's fused chip_ver/speed_bin/rev */
		vpp_init_voltage_plan(&vpp_table[idx], &voltage_plan, &vpp_device);
		/* init mapping of virtual corner and voltage/quotient data */
		vpp_init_virtual_corner(&voltage_plan, &vpp_device);
		/* merge osm & cprh data for client's use */
		vpp_init_osm_cprh_plan(&vpp_table[idx], &voltage_plan);
	}
}
