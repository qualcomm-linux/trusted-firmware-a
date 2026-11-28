/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Shikra device initialization, APM lookup, voltage-plan selection, and
 * virtual-corner validation.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <drivers/qti/chipinfo/chipinfo.h>

#include "vpp_cfg.h"
#include "vpp_fuse_hwio.h"
#include "vpp_voltage_plan.h"

#define VPP_CPR_LOCAL_RC	VPP_HWIO_INM(QFPROM_CORR_CALIB_ROW3_MSB, CPR_LOCAL_RC)
#define VPP_SPEED_BIN		VPP_HWIO_INM(QFPROM_CORR_PTE_ROW3_LSB, SPEED_BIN)
#define VPP_PTE_MCN_IDENTIFIER	VPP_HWIO_INM(QFPROM_CORR_PTE_ROW3_LSB, PTE_MCN_IDENTIFIER)
#define VPP_VP_ID		VPP_HWIO_INM(QFPROM_CORR_QC_SPARE_REG20_LSB, SW_SPT_LOCAL_RAILS_VPID)

void vpp_init_device_info(vpp_device_t *vpp_device_info)
{
	vpp_device_info->prop.acd_enable = VPP_ACD_ON;
	vpp_device_info->prop.aging_enable = VPP_AGING_ON;
	vpp_device_info->prop.aging_margin_enable = VPP_AGING_MARGIN_ON;
	vpp_device_info->prop.avg_enable = VPP_AVG_ON;
	vpp_device_info->prop.apm_auto_enable = VPP_AUTO_APM;
	vpp_device_info->speed_bin = VPP_SPEED_BIN;
	vpp_device_info->cpr_local_rev = VPP_CPR_LOCAL_RC;
	vpp_device_info->chip_ver = chipinfo_get_chip_version();
	vpp_device_info->pte_mcn_identifier = VPP_PTE_MCN_IDENTIFIER;
	vpp_device_info->vpid = VPP_VP_ID;
}

vpp_clk_enum vpp_get_clk_enum(const char *clk)
{
	if (!strcmp(clk, "Silver"))
		return VPP_CLK_SILVER;
	else if (!strcmp(clk, "L3"))
		return VPP_CLK_L3;
	else if (!strcmp(clk, "Gold"))
		return VPP_CLK_GOLD;

	return VPP_CLK_TOTAL;
}

/* Public vpp context shared with clients. */
/* Static storage initializes cnt to zero; vpp_init() fills it from the plan. */
vpp_t vpp_table[VPP_CLK_TOTAL] = {
	{
		.rail_name = "APC",
		.clk_name = "Silver",
	},
	{
		.rail_name = "APC",
		.clk_name = "L3",
	},
	{
		.rail_name = "APC",
		.clk_name = "Gold",
	},
};

/* Apply the fused CPR local revision's virtual adjustment onto the voltage-setting table. */
static void vpp_update_virtual_adj(vpp_clk_enum clk, vpp_device_t *vpp_device_info)
{
	bool isflag = false;
	uint8_t idx1, idx2;

	uint8_t rc = vpp_device_info->cpr_local_rev;

	VPP_CORE_VERIFY(rc < VPP_MAX_CPR_LOCAL_RC);

	uint8_t chip_ver = (vpp_device_info->chip_ver >> 16) - 1;
	uint8_t speed_bin = vpp_device_info->speed_bin;

	VPP_CORE_VERIFY(speed_bin < VPP_MAX_SPEED_BIN);

	vpp_virtual_adj_t *viradj = &virtual_adj_table[chip_ver][clk][rc][0];
	vpp_voltage_settings_t *volt_setting = &volt_setting_table[chip_ver][clk][0];

	for (idx1 = 0; volt_setting->vlvl != VPP_VLVL_INVALID; idx1++) {
		volt_setting = &volt_setting_table[chip_ver][clk][idx1];
		for (idx2 = 0; viradj->vlvl != VPP_VLVL_INVALID; idx2++) {
			viradj = &virtual_adj_table[chip_ver][clk][rc][idx2];
			if (volt_setting->vlvl == viradj->vlvl &&
			    volt_setting->freq_kHz == viradj->freq_kHz) {
				volt_setting->olv_adj_acd_mV = viradj->olv_adj_acd_mV;
				volt_setting->clv_adj_acd_mV = viradj->clv_adj_acd_mV;
				isflag = true;
				break;
			}
		}
		VPP_CORE_VERIFY(isflag);
		isflag = false;
	}
}

void vpp_init_voltage_plan(vpp_t *vpp, vpp_internal_t *voltage_plan, vpp_device_t *vpp_device_info)
{
	uint8_t rc = vpp_device_info->cpr_local_rev;
	uint8_t chip_ver_idx = (vpp_device_info->chip_ver >> 16) - 1;
	uint8_t speed_bin = vpp_device_info->speed_bin;
	vpp_clk_enum clk = vpp_get_clk_enum(vpp->clk_name);

	VPP_CORE_VERIFY(speed_bin < VPP_MAX_SPEED_BIN);
	VPP_CORE_VERIFY(rc < VPP_MAX_CPR_LOCAL_RC);

	voltage_plan->rail_name = vpp->rail_name;
	voltage_plan->clk_name = vpp->clk_name;

	vpp_update_virtual_adj(clk, vpp_device_info);
	voltage_plan->voltage_setting = volt_setting_table[chip_ver_idx][clk];
	voltage_plan->osm_setting = osm_table[chip_ver_idx][clk][speed_bin][rc];
	voltage_plan->kv = kv_table[chip_ver_idx][clk];
	voltage_plan->fuse_adj = fuse_adj_table[chip_ver_idx][clk][rc];
}

void vpp_get_apm_vc(const char *rail, const char *clk, uint8_t *apm_vc,
		    uint16_t *apm_threshold_mV, uint16_t *apm_intermediate_mV,
		    uint8_t *is_required)
{
	const vpp_t *vpp;

	vpp = vpp_get_plan(rail, clk);
	VPP_CORE_VERIFY(vpp);

	/* reserve highest vc + 1 for the APM intermediate crossover voltage */
	*apm_vc = vpp->cprh[vpp->cnt - 1].vc + 1;
	*apm_threshold_mV = VPP_APM_THRESHOLD_VOLTAGE;
	*apm_intermediate_mV = VPP_APM_INTERMEDIATE_VOLTAGE;
	*is_required = VPP_AUTO_APM ? APM_AUTO_MODE : APM_SW_MODE;
}

static uint32_t vpp_round_up(uint32_t value, uint32_t multiplier)
{
	if (value % multiplier == 0)
		return value;

	return value + multiplier - (value % multiplier);
}

void vpp_validate_voltage_plan(vpp_internal_t *vpp, const vpp_device_t *vpp_prop)
{
	(void)vpp;
	(void)vpp_prop;
	/* no workaround */
}

void vpp_validate_virtual_corner(vpp_internal_t *vpp, uint8_t volt_setting_count)
{
	vpp_voltage_settings_t *setting;
	vpp_voltage_settings_t *prev_setting;
	uint8_t idx;

	/* SLT-margin handling is omitted; the reference falls back to is_slt_plat=0
	 * when the DAL PlatformInfo SLT key is absent.
	 */
	for (idx = 0; idx < volt_setting_count; idx++) {
		setting = &vpp->voltage_setting[idx];

		/* cap floor/ceil if out of range */
		if (setting->cprh.ceil_mV > setting->initial_ceil_mV) {
			setting->cprh.ceil_mV = setting->initial_ceil_mV;
		} else if (setting->cprh.ceil_mV < setting->initial_floor_mV) {
			setting->cprh.ceil_mV = setting->initial_floor_mV;
		}

		if (setting->cprh.floor_mV < setting->initial_floor_mV) {
			setting->cprh.floor_mV = setting->initial_floor_mV;
		} else if (setting->cprh.floor_mV > setting->cprh.ceil_mV) {
			setting->cprh.floor_mV = setting->cprh.ceil_mV;
		}

		/* round up to a multiple of the PMIC step size */
		setting->cprh.ceil_mV = vpp_round_up(setting->cprh.ceil_mV,
						     VPP_PMIC_STEP_SIZE);
		setting->cprh.floor_mV = vpp_round_up(setting->cprh.floor_mV,
						      VPP_PMIC_STEP_SIZE);

		if (idx > 0) {
			prev_setting = &vpp->voltage_setting[idx - 1];

			/* monotonic check on open-loop/ceiling voltage */
			if (prev_setting->cprh.ceil_mV > setting->cprh.ceil_mV)
				setting->cprh.ceil_mV = prev_setting->cprh.ceil_mV;

			/* monotonic check on floor voltage */
			if (prev_setting->cprh.floor_mV > setting->cprh.floor_mV)
				setting->cprh.floor_mV = prev_setting->cprh.floor_mV;

			/* monotonic check on target quotient */
			if (prev_setting->cprh.ro == setting->cprh.ro &&
			    prev_setting->cprh.target_quot > setting->cprh.target_quot) {
				setting->cprh.target_quot = prev_setting->cprh.target_quot;
			}
		}
	}
}
