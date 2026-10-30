/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <drivers/qti/vpp/vpp.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define QFPROM_CORR_CALIB_ROW4_LSB	0x01B44278U
#define QFPROM_CORR_CALIB_ROW4_MSB	0x01B4427CU
#define QFPROM_CORR_CALIB_ROW5_LSB	0x01B44280U
#define QFPROM_CORR_CALIB_ROW5_MSB	0x01B44284U
#define QFPROM_CORR_CALIB_ROW6_LSB	0x01B44288U
#define QFPROM_CORR_CALIB_ROW6_MSB	0x01B4428CU
#define QFPROM_CORR_CALIB_ROW7_LSB	0x01B44290U
#define QFPROM_CORR_CALIB_ROW7_MSB	0x01B44294U
#define QFPROM_CORR_CALIB_ROW8_LSB	0x01B44298U
#define QFPROM_CORR_CALIB_ROW8_MSB	0x01B4429CU
#define QFPROM_CORR_CALIB_ROW9_LSB	0x01B442A0U
#define QFPROM_CORR_CALIB_ROW9_MSB	0x01B442A4U

#define VPP_APC_VOLTAGE_VALUE_MASK	0x1fU
#define VPP_APC_SIGN_MASK		0x20U
#define VPP_APC_OLV_PACKING_FACTOR	8
#define VPP_CLV_QUOT_OFFSET_PACK_FACTOR	5
#define VPP_INTERPOLATION_FACTOR	1000000U

typedef int32_t (*fuse_cb_t)(void);

typedef struct {
	uint32_t freq_khz;
	uint8_t clk_src;
	uint8_t max_core_count;
	uint8_t lval;
	uint16_t ref_mv;
	uint16_t floor_range_mv;
	uint8_t ro;
	fuse_cb_t fused_olv;
	fuse_cb_t fused_quot;
	fuse_cb_t fused_quot_offset;
	vpp_cprh_t cprh;
} vpp_voltage_setting_t;

static uint32_t vpp_field(uintptr_t addr, uint32_t mask, uint32_t shift)
{
	return (mmio_read_32(addr) & mask) >> shift;
}

static int32_t vpp_olv(uint32_t val)
{
	int32_t sign = ((val & VPP_APC_SIGN_MASK) != 0U) ? -1 : 1;

	return (int32_t)((val & VPP_APC_VOLTAGE_VALUE_MASK) *
			 VPP_APC_OLV_PACKING_FACTOR) * sign;
}

static int32_t cpr10_targ_volt_lsvs(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW8_LSB, 0x0003f000U, 12));
}

static int32_t cpr10_targ_volt_svs(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW8_MSB, 0x3f000000U, 24));
}

static int32_t cpr10_targ_volt_nom(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW9_LSB, 0x07e00000U, 21));
}

static int32_t cpr10_targ_volt_tur(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW9_MSB, 0x0000003fU, 0));
}

static int32_t cpr10_targ_volt_boost(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW9_MSB, 0x00000fc0U, 6));
}

static int32_t cpr11_targ_volt_lsvs(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW5_MSB, 0xfc000000U, 26));
}

static int32_t cpr11_targ_volt_svs(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW6_LSB, 0x3f000000U, 24));
}

static int32_t cpr11_targ_volt_nom(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW6_MSB, 0x3f000000U, 24));
}

static int32_t cpr11_targ_volt_tur(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW7_LSB, 0x3f000000U, 24));
}

static int32_t cpr11_targ_volt_boost(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW7_MSB, 0x0000003fU, 0));
}

static int32_t cpr12_targ_volt_lsvs(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW4_MSB, 0x3f000000U, 24));
}

static int32_t cpr12_targ_volt_svs(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW5_LSB, 0x3f000000U, 24));
}

static int32_t cpr12_targ_volt_nom(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW5_MSB, 0x0000003fU, 0));
}

static int32_t cpr12_targ_volt_tur(void)
{
	return vpp_olv(vpp_field(QFPROM_CORR_CALIB_ROW5_MSB, 0x00000fc0U, 6));
}

static int32_t cpr10_quot_min_lsvs(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW7_MSB, 0xfff00000U, 20);
}

static int32_t cpr10_quot_min_svs(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW8_LSB, 0x00000fffU, 0);
}

static int32_t cpr10_quot_min_nom(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW8_MSB, 0x00000fffU, 0);
}

static int32_t cpr10_quot_min_tur(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW8_MSB, 0x00fff000U, 12);
}

static int32_t cpr10_quot_min_boost(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW9_LSB, 0x001ffe00U, 9);
}

static int32_t cpr11_quot_min_lsvs(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW6_LSB, 0x00000fffU, 0);
}

static int32_t cpr11_quot_min_svs(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW6_LSB, 0x00fff000U, 12);
}

static int32_t cpr11_quot_min_nom(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW6_MSB, 0x00000fffU, 0);
}

static int32_t cpr11_quot_min_tur(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW6_MSB, 0x00fff000U, 12);
}

static int32_t cpr11_quot_min_boost(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW7_LSB, 0x00000fffU, 0);
}

static int32_t cpr12_quot_min_lsvs(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW4_LSB, 0xfff00000U, 20);
}

static int32_t cpr12_quot_min_svs(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW4_MSB, 0x00000fffU, 0);
}

static int32_t cpr12_quot_min_nom(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW4_MSB, 0x00fff000U, 12);
}

static int32_t cpr12_quot_min_tur(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW5_LSB, 0x00000fffU, 0);
}

static int32_t cpr10_quot_offset(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW7_MSB, 0x000ff000U, 12) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}

static int32_t cpr11_quot_offset(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW5_MSB, 0x03fc0000U, 18) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}

static int32_t cpr12_quot_offset(void)
{
	return (int32_t)vpp_field(QFPROM_CORR_CALIB_ROW4_LSB, 0x000ff000U, 12) *
	       VPP_CLV_QUOT_OFFSET_PACK_FACTOR;
}

static vpp_voltage_setting_t silver_settings[] = {
	{ 768000, 1, 4, 40, 644, 136, 13, cpr10_targ_volt_lsvs, cpr10_quot_min_lsvs },
	{ 1017600, 1, 4, 53, 728, 176, 13, cpr10_targ_volt_svs, cpr10_quot_min_svs, cpr10_quot_offset },
	{ .freq_khz = 1094400, .clk_src = 1, .max_core_count = 4, .lval = 57 },
	{ 1497600, 1, 4, 78, 872, 220, 13, cpr10_targ_volt_nom, cpr10_quot_min_nom, cpr10_quot_offset },
	{ .freq_khz = 1612800, .clk_src = 1, .max_core_count = 4, .lval = 84 },
	{ 1804800, 1, 4, 94, 988, 260, 7, cpr10_targ_volt_tur, cpr10_quot_min_tur, cpr10_quot_offset },
	{ 2208000, 1, 4, 115, 988, 260, 7, cpr10_targ_volt_boost, cpr10_quot_min_boost, cpr10_quot_offset },
};

static vpp_voltage_setting_t l3_settings[] = {
	{ 556800, 1, 4, 29, 644, 136, 13, cpr12_targ_volt_lsvs, cpr12_quot_min_lsvs },
	{ 806400, 1, 4, 42, 728, 172, 13, cpr12_targ_volt_svs, cpr12_quot_min_svs, cpr12_quot_offset },
	{ .freq_khz = 940800, .clk_src = 1, .max_core_count = 4, .lval = 49 },
	{ 1209600, 1, 4, 63, 872, 224, 13, cpr12_targ_volt_nom, cpr12_quot_min_nom, cpr12_quot_offset },
	{ .freq_khz = 1344000, .clk_src = 1, .max_core_count = 4, .lval = 70 },
	{ 1363200, 1, 4, 71, 988, 296, 7, cpr12_targ_volt_tur, cpr12_quot_min_tur, cpr12_quot_offset },
};

static vpp_voltage_setting_t gold_settings[] = {
	{ 768000, 1, 4, 40, 644, 132, 13, cpr11_targ_volt_lsvs, cpr11_quot_min_lsvs },
	{ 1017600, 1, 4, 53, 728, 164, 13, cpr11_targ_volt_svs, cpr11_quot_min_svs, cpr11_quot_offset },
	{ .freq_khz = 1190400, .clk_src = 1, .max_core_count = 4, .lval = 62 },
	{ 1497600, 1, 4, 78, 872, 204, 13, cpr11_targ_volt_nom, cpr11_quot_min_nom, cpr11_quot_offset },
	{ .freq_khz = 1708800, .clk_src = 1, .max_core_count = 4, .lval = 89 },
	{ 1900800, 1, 4, 99, 988, 192, 7, cpr11_targ_volt_tur, cpr11_quot_min_tur, cpr11_quot_offset },
	{ 2208000, 1, 4, 115, 988, 192, 7, cpr11_targ_volt_boost, cpr11_quot_min_boost, cpr11_quot_offset },
};

vpp_t qti_vpp_table[] = {
	{ .rail_name = "APC", .clk_name = "Silver" },
	{ .rail_name = "APC", .clk_name = "L3" },
	{ .rail_name = "APC", .clk_name = "Gold" },
};

const uint32_t qti_vpp_table_size = ARRAY_SIZE(qti_vpp_table);

static void init_base_values(vpp_voltage_setting_t *settings,
			     uint32_t setting_count)
{
	uint8_t fused_ro = 0U;
	uint16_t fused_olv = 0U;
	uint16_t fused_quot = 0U;
	uint16_t floor_range = 0U;

	for (uint32_t i = setting_count; i-- > 0U;) {
		vpp_voltage_setting_t *setting = &settings[i];

		if (setting->fused_olv != NULL) {
			fused_olv = (uint16_t)((int32_t)setting->ref_mv +
					       setting->fused_olv());
			floor_range = setting->floor_range_mv;
		}

		if (setting->fused_quot != NULL) {
			fused_quot = (uint16_t)setting->fused_quot();
			fused_ro = setting->ro;
		}

		setting->cprh.vc = i;
		setting->cprh.ceil_mV = fused_olv;
		setting->cprh.floor_mV = fused_olv - floor_range;
		setting->cprh.target_quot = fused_quot;
		setting->cprh.ro = fused_ro;
	}
}

static void interpolate_olv(vpp_voltage_setting_t *settings, uint32_t count)
{
	for (uint32_t hi = count; hi-- > 0U;) {
		if (settings[hi].fused_olv == NULL) {
			continue;
		}

		for (uint32_t lo = hi; lo-- > 0U;) {
			uint32_t slope;

			if (settings[lo].fused_olv == NULL) {
				continue;
			}

			if (settings[hi].cprh.ceil_mV < settings[lo].cprh.ceil_mV) {
				settings[hi].cprh.ceil_mV = settings[lo].cprh.ceil_mV;
				settings[hi].cprh.floor_mV = settings[lo].cprh.floor_mV;
			}

			slope = ((settings[hi].cprh.ceil_mV -
				  settings[lo].cprh.ceil_mV) *
				 VPP_INTERPOLATION_FACTOR) /
				(settings[hi].freq_khz - settings[lo].freq_khz);

			for (uint32_t i = lo; i < hi; i++) {
				uint32_t delta;

				if (settings[i].fused_olv != NULL) {
					continue;
				}

				delta = ((settings[hi].freq_khz -
					  settings[i].freq_khz) * slope) /
					VPP_INTERPOLATION_FACTOR;
				settings[i].cprh.ceil_mV -= (uint16_t)delta;
				settings[i].cprh.floor_mV -= (uint16_t)delta;
			}
			break;
		}
	}
}

static void interpolate_quot(vpp_voltage_setting_t *settings, uint32_t count)
{
	for (uint32_t hi = count; hi-- > 0U;) {
		if (settings[hi].fused_quot == NULL) {
			continue;
		}

		for (uint32_t lo = hi; lo-- > 0U;) {
			int32_t quot_offset;
			uint32_t slope;

			if (settings[lo].fused_quot == NULL) {
				continue;
			}

			if (settings[hi].fused_quot_offset != NULL) {
				quot_offset = settings[hi].fused_quot_offset();
			} else {
				quot_offset = (int32_t)settings[hi].cprh.target_quot -
					      (int32_t)settings[lo].cprh.target_quot;
			}
			if (quot_offset < 0) {
				quot_offset = 0;
			}

			slope = ((uint32_t)quot_offset * VPP_INTERPOLATION_FACTOR) /
				(settings[hi].freq_khz - settings[lo].freq_khz);

			for (uint32_t i = lo; i < hi; i++) {
				uint32_t delta;

				if (settings[i].fused_quot != NULL) {
					continue;
				}

				delta = ((settings[hi].freq_khz -
					  settings[i].freq_khz) * slope) /
					VPP_INTERPOLATION_FACTOR;
				settings[i].cprh.target_quot -= (uint16_t)delta;
			}
			break;
		}
	}
}

static void publish_plan(uint32_t table_idx, vpp_voltage_setting_t *settings,
			 uint32_t setting_count)
{
	init_base_values(settings, setting_count);
	interpolate_olv(settings, setting_count);
	interpolate_quot(settings, setting_count);

	qti_vpp_table[table_idx].cnt = setting_count;
	for (uint32_t i = 0; i < setting_count; i++) {
		qti_vpp_table[table_idx].osm[i].clk_src = settings[i].clk_src;
		qti_vpp_table[table_idx].osm[i].max_core_count = settings[i].max_core_count;
		qti_vpp_table[table_idx].osm[i].lval = settings[i].lval;
		qti_vpp_table[table_idx].osm[i].vc = settings[i].cprh.vc;
		qti_vpp_table[table_idx].cprh[i] = settings[i].cprh;
	}
}

void qti_vpp_init(void)
{
	static bool initialized;

	if (initialized) {
		return;
	}

	publish_plan(0, silver_settings, ARRAY_SIZE(silver_settings));
	publish_plan(1, l3_settings, ARRAY_SIZE(l3_settings));
	publish_plan(2, gold_settings, ARRAY_SIZE(gold_settings));

	initialized = true;
}
