/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Generic virtual-corner interpolation engine: turns the per-rail fused
 * open-loop voltage/target-quotient fix points and KV (quotient-per-mV)
 * plan into a ceil/floor voltage and target quotient for every virtual
 * corner CPRH needs to program.
 */

#include <stdint.h>

#include "vpp_cfg.h"
#include "vpp_internal.h"

#ifndef CALCULATE_KV
#define CALCULATE_KV 0
#endif

#define VPP_OLV_FUSED(t)	((t)->olv_ref_mV + (int16_t)(t)->fused_olv())

#define VPP_CLV_FUSE_ADJ(t)						\
	((vpp_prop->prop.aging_margin_enable && vpp_prop->cpr_local_rev) ? \
	 (t)->clv_fuse_adj_table->aging_on_clv_fuse_adj :		\
	 (t)->clv_fuse_adj_table->aging_off_clv_fuse_adj)

#define VPP_OLV_FUSE_ADJ(t)						\
	(vpp_prop->prop.aging_margin_enable ?				\
	 (t)->olv_fuse_adj_table->aging_on_olv_fuse_adj :		\
	 (t)->olv_fuse_adj_table->aging_off_olv_fuse_adj)

/*
 * Aging adjustment is always 0: the reference implementation computes this
 * by actively sweeping the rail voltage through a live CPR aging-measurement
 * cycle on first use (power/cprh/common/cprh_aging.c), which this port does
 * not carry over. Every corner is treated as cpr_local_rev == 0 for aging
 * purposes.
 */
#define VPP_OLV_AGING_ADJ(rail)		0

static vpp_fuse_adj_t *vpp_get_fuse_adj_table(vpp_fuse_adj_t *fuse_adj_table,
					       vpp_vlvl_t vlvl)
{
	uint8_t idx = 0;
	vpp_fuse_adj_t *fuse_adj = NULL;

	if (fuse_adj_table != NULL) {
		while (fuse_adj_table[idx].vlvl != VPP_VLVL_INVALID) {
			if (fuse_adj_table[idx].vlvl == vlvl) {
				fuse_adj = &fuse_adj_table[idx];
				break;
			}
			idx++;
		}
	}

	return fuse_adj;
}

static uint32_t vpp_get_kv(vpp_kv_t *kv_table, vpp_vlvl_t vlvl, uint32_t ro)
{
	uint8_t idx;
	uint32_t kv = 0;

	if (kv_table) {
		for (idx = 0; kv_table[idx].min_vlvl != VPP_VLVL_INVALID; idx++) {
			if (vlvl > kv_table[idx].min_vlvl &&
			    vlvl <= kv_table[idx].max_vlvl) {
				kv = kv_table[idx].kv[ro];
				break;
			}
		}

		/* if lowest corner is not a fusing point */
		if (!kv) {
			for (idx = 0; kv_table[idx].min_vlvl != VPP_VLVL_INVALID; idx++) {
				if (vlvl <= kv_table[idx].min_vlvl ||
				    kv_table[idx].max_vlvl >= vlvl) {
					kv = kv_table[idx].kv[ro];
					break;
				}
			}
		}
	}

	VPP_CORE_VERIFY(kv);
	return kv;
}

/* Recompute KV from two fused olv/quot points, overwriting the generated kv tables. */
static void vpp_calculate_kv(vpp_internal_t *vpp, uint8_t volt_count)
{
	vpp_voltage_settings_t *volt_setting;
	vpp_vlvl_t vlvl = VPP_VLVL_OFF;
	uint16_t fused_olv = 0;
	uint16_t fused_quot = 0;
	uint32_t kv;
	uint8_t ro = 0;
	uint8_t idx, idx2, idx1;
	vpp_kv_t *kv_table = vpp->kv;

	if (!kv_table || !CALCULATE_KV)
		return;

	for (idx = volt_count; idx-- > 0; ) {
		volt_setting = &vpp->voltage_setting[idx];

		if (!(volt_setting->olv_fused_point && volt_setting->quot_fused_point))
			continue;

		if (fused_olv) {
			/* calculate kv with 2nd fused point */
			if ((uint8_t)volt_setting->fused_ro() != ro)
				kv = (uint32_t)(1000 * (int16_t)volt_setting->fused_quot_offset()) /
				     (fused_olv - VPP_OLV_FUSED(volt_setting));
			else
				kv = (uint32_t)(1000 * (fused_quot - (int16_t)volt_setting->fused_quot())) /
				     (fused_olv - VPP_OLV_FUSED(volt_setting));

			for (idx2 = 0; kv_table[idx2].min_vlvl != VPP_VLVL_INVALID; idx2++) {
				if (volt_setting->vlvl >= kv_table[idx2].min_vlvl &&
				    vlvl <= kv_table[idx2].max_vlvl &&
				    kv >= kv_table[idx2].kv_min[ro] &&
				    kv <= kv_table[idx2].kv_max[ro]) {
					for (idx1 = 0; idx1 < 16; idx1++)
						kv_table[idx2].kv[idx1] = kv;
				}
			}
		}

		fused_olv = VPP_OLV_FUSED(volt_setting);
		fused_quot = (int16_t)volt_setting->fused_quot();
		vlvl = volt_setting->vlvl;
		ro = (uint8_t)volt_setting->fused_ro();
	}
}

static void vpp_init_base_val(vpp_internal_t *vpp, uint8_t volt_count,
			       const vpp_device_t *vpp_prop)
{
	uint8_t idx;
	uint8_t fused_ro = 0;
	uint16_t fused_olv = 0;
	uint16_t fused_quot = 0;
	uint16_t ceil_floor_range = 0;
	int16_t fuse_olv_adj = 0;
	int16_t fuse_clv_adj = 0;
	uint32_t kv = 0;
	vpp_voltage_settings_t *volt_setting;

	vpp_calculate_kv(vpp, volt_count);

	/* loop from fmax to fmin */
	for (idx = volt_count; idx-- > 0; ) {
		volt_setting = &vpp->voltage_setting[idx];

		/* initialize fused open-loop voltage & adjustment on fused point */
		if (volt_setting->olv_fused_point) {
			volt_setting->olv_fuse_adj_table =
				vpp_get_fuse_adj_table(vpp->fuse_adj, volt_setting->vlvl);
			VPP_CORE_VERIFY(volt_setting->olv_fuse_adj_table);
			fuse_olv_adj = VPP_OLV_FUSE_ADJ(volt_setting);
			fused_olv = VPP_OLV_FUSED(volt_setting);
			ceil_floor_range = volt_setting->olv_fuse_adj_table->ceil_floor_range_mV;
		}
		/* initialize fused quotient & adjustment on fused point */
		if (volt_setting->quot_fused_point) {
			volt_setting->clv_fuse_adj_table =
				vpp_get_fuse_adj_table(vpp->fuse_adj, volt_setting->vlvl);
			VPP_CORE_VERIFY(volt_setting->clv_fuse_adj_table);
			fuse_clv_adj = VPP_CLV_FUSE_ADJ(volt_setting);
			fused_quot = (int16_t)volt_setting->fused_quot();
			fused_ro = (uint8_t)volt_setting->fused_ro();
		}

		/* initialize voltages & quotient based on the fused value & adjustments */
		kv = vpp_get_kv(vpp->kv, volt_setting->vlvl, fused_ro);
		volt_setting->cprh.ceil_mV = fused_olv + fuse_olv_adj +
			volt_setting->olv_adj_acd_mV + VPP_OLV_AGING_ADJ(vpp->rail_name);
		volt_setting->cprh.floor_mV = volt_setting->cprh.ceil_mV - ceil_floor_range;
		volt_setting->cprh.vc = volt_setting->vc;
		volt_setting->cprh.ro = fused_ro;
		volt_setting->cprh.target_quot = fused_quot +
			(int32_t)kv * (fuse_clv_adj + volt_setting->clv_adj_acd_mV +
				       VPP_OLV_AGING_ADJ(vpp->rail_name)) / 1000;
	}
}

static void vpp_olv_interpolate(vpp_internal_t *vpp, uint8_t volt_setting_count,
				 const vpp_device_t *vpp_prop)
{
	uint8_t idx, idx_hi, idx_lo;
	uint32_t olv_hi, olv_lo, slope, fmax;
	vpp_voltage_settings_t *volt_setting_hi, *volt_setting_lo;

	/* search for first fused point from fmax */
	for (idx_hi = volt_setting_count; idx_hi-- > 0; ) {
		volt_setting_hi = &vpp->voltage_setting[idx_hi];
		if (!volt_setting_hi->olv_fused_point)
			continue;

		/* search for the next fused point */
		for (idx_lo = idx_hi; idx_lo-- > 0; ) {
			volt_setting_lo = &vpp->voltage_setting[idx_lo];
			if (!volt_setting_lo->olv_fused_point)
				continue;

			/*
			 * Slope between the two points is based on
			 * fused_olv + FUSE_ADJ(), after removing the VC
			 * margin added in vpp_init_base_val().
			 */
			olv_hi = VPP_OLV_FUSED(volt_setting_hi) + VPP_OLV_FUSE_ADJ(volt_setting_hi);
			olv_lo = VPP_OLV_FUSED(volt_setting_lo) + VPP_OLV_FUSE_ADJ(volt_setting_lo);

			/* if lower point has higher OLV, use 0.0 slope */
			if (olv_hi < olv_lo)
				olv_hi = olv_lo;

			/* multiply by a large value as float is not supported */
			slope = ((olv_hi - olv_lo) * 1000000) /
				(volt_setting_hi->freq_kHz - volt_setting_lo->freq_kHz);

			/* add interpolated offset to the fused value between idx_lo and idx_hi */
			for (idx = idx_lo; idx < idx_hi; idx++) {
				if (!vpp->voltage_setting[idx].olv_interpolate)
					continue;

				fmax = volt_setting_hi->freq_kHz;
				vpp->voltage_setting[idx].cprh.ceil_mV -=
					(int16_t)(((fmax - vpp->voltage_setting[idx].freq_kHz) * slope) / 1000000);
				vpp->voltage_setting[idx].cprh.floor_mV -=
					(int16_t)(((fmax - vpp->voltage_setting[idx].freq_kHz) * slope) / 1000000);
			}
			break;
		}
	}
}

static void vpp_clv_interpolate(vpp_internal_t *vpp, uint8_t volt_setting_count,
				 const vpp_device_t *vpp_prop)
{
	uint8_t idx, idx_hi, idx_lo;
	int32_t quot_offset, quot_offset_adj;
	uint32_t kv, slope, fmax;
	vpp_voltage_settings_t *volt_setting_hi, *volt_setting_lo;

	/* search for first target quotient fuse point from fmax */
	for (idx_hi = volt_setting_count; idx_hi-- > 0; ) {
		volt_setting_hi = &vpp->voltage_setting[idx_hi];
		if (!volt_setting_hi->quot_fused_point)
			continue;

		/* search for the next fused point */
		for (idx_lo = idx_hi; idx_lo-- > 0; ) {
			volt_setting_lo = &vpp->voltage_setting[idx_lo];
			if (!volt_setting_lo->quot_fused_point)
				continue;

			kv = vpp_get_kv(vpp->kv, volt_setting_hi->vlvl, volt_setting_hi->cprh.ro);
			quot_offset_adj = VPP_CLV_FUSE_ADJ(volt_setting_hi) -
				VPP_CLV_FUSE_ADJ(volt_setting_lo);
			quot_offset = (volt_setting_hi->fused_quot_offset() * 1000 +
				       ((int32_t)kv * quot_offset_adj)) / 1000;

			/* if -ve quotient offset, use 0.0 slope */
			if (quot_offset < 0)
				quot_offset = 0;

			/* multiply by a large value as float is not supported */
			slope = (quot_offset * 1000000) /
				(volt_setting_hi->freq_kHz - volt_setting_lo->freq_kHz);

			/* add interpolated offset to the base value between idx_lo and idx_hi */
			for (idx = idx_lo; idx < idx_hi; idx++) {
				if (!vpp->voltage_setting[idx].quot_interpolate)
					continue;

				fmax = volt_setting_hi->freq_kHz;
				vpp->voltage_setting[idx].cprh.target_quot -=
					(int16_t)(((fmax - vpp->voltage_setting[idx].freq_kHz) * slope) / 1000000);
			}
			break;
		}
	}
}

static uint8_t vpp_get_volt_setting_count(const vpp_voltage_settings_t *table)
{
	uint8_t count = 0;

	while (table[count].vlvl != VPP_VLVL_INVALID)
		count++;

	return count;
}

void vpp_init_virtual_corner(vpp_internal_t *vpp, const vpp_device_t *vpp_prop)
{
	uint8_t volt_setting_count = vpp_get_volt_setting_count(vpp->voltage_setting);

	vpp_validate_voltage_plan(vpp, vpp_prop);
	/* initialize fused value + adjustments first for each frequency */
	vpp_init_base_val(vpp, volt_setting_count, vpp_prop);
	/* apply interpolation adjustment to floor & ceil voltages */
	vpp_olv_interpolate(vpp, volt_setting_count, vpp_prop);
	/* apply interpolation adjustment to target quotient */
	vpp_clv_interpolate(vpp, volt_setting_count, vpp_prop);
	vpp_validate_virtual_corner(vpp, volt_setting_count);
}
