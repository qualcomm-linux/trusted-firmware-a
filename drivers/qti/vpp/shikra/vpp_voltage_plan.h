/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Shikra voltage-plan selection and calibration-table interfaces.
 */

#ifndef VPP_VOLTAGE_PLAN_H
#define VPP_VOLTAGE_PLAN_H

#include <stdint.h>

#include "vpp_internal.h"

#define VPP_MAX_SPEED_BIN	3U
#define VPP_MAX_CPR_LOCAL_RC	2U

typedef enum {
	VPP_CLK_SILVER,
	VPP_CLK_L3,
	VPP_CLK_GOLD,
	VPP_CLK_TOTAL
} vpp_clk_enum;

extern vpp_t vpp_table[VPP_CLK_TOTAL];

void vpp_init_device_info(vpp_device_t *vpp_device_info);
vpp_clk_enum vpp_get_clk_enum(const char *clk);

/*
 * vpp_init_voltage_plan - select the generated voltage_setting/osm_setting/
 * fuse_adj/kv table pointers matching this device's fused chip_ver/
 * speed_bin/cpr_local_rev, into vpp_voltage_plan.
 */
void vpp_init_voltage_plan(vpp_t *vpp_context, vpp_internal_t *vpp_voltage_plan,
			    vpp_device_t *vpp_device_info);

/* Indexed [chip_ver][clk]. */
extern vpp_voltage_settings_t *volt_setting_table[1][3];
/* Indexed [chip_ver][clk][cpr_local_rev]. */
extern vpp_virtual_adj_t *virtual_adj_table[1][3][2];
/* Indexed [chip_ver][clk][speed_bin][cpr_local_rev]. */
extern vpp_osm_t *osm_table[1][3][3][2];
/* Indexed [chip_ver][clk]. */
extern vpp_kv_t *kv_table[1][3];
/* Indexed [chip_ver][clk][cpr_local_rev]. */
extern vpp_fuse_adj_t *fuse_adj_table[1][3][2];

#endif /* VPP_VOLTAGE_PLAN_H */
