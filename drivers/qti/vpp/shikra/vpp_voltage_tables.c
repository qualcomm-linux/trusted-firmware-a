/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * THIS FILE IS TRANSCRIBED FROM AN AUTO-GENERATED VOLTAGE PLAN SOURCE FILE.
 *
 * Generated per-rail (Silver/L3/Gold) OSM LUT, KV, fuse-adjustment, and
 * virtual-adjustment calibration tables for shikra, selected by
 * chip_ver/speed_bin/cpr_local_rev/vpid. Plan-selection logic that indexes
 * these tables lives in vpp_voltage_plan.c.
 */

#include <stdint.h>

#include "vpp_fuse_plan.h"
#include "vpp_voltage_plan.h"

/*---------------------------------------------------------------------------
 * Voltage Settings
 *--------------------------------------------------------------------------*/

/* OLV/CLV adjustment members are populated from virtual_adj_table during
 * plan selection; omit their zero defaults from these initializers.
 */
static vpp_voltage_settings_t volt_setting_l3_v1[] = {
	/*               vlvl,     freq,  en, vc, ref_mV, ceil_mV, floor_mV, olv_fuse_pt, olv_interp, quot_fuse_pt, quot_interp,              fused_olv,                fused_quot,            quot_offset,         ro_select */
	{     VPP_VLVL_LOWSVS,   556800,   1,  0,    644,     644,      588,           1,          0,            1,           0,   CPR12_TARG_VOLT_LSVS,       CPR12_QUOT_MIN_LSVS,                      0,  CPR12_LSVS_ROSEL},
	{        VPP_VLVL_SVS,   806400,   1,  1,    728,     728,      588,           1,          0,            1,           0,    CPR12_TARG_VOLT_SVS,        CPR12_QUOT_MIN_SVS, CPR12_QUOT_OFFSET_SVS,   CPR12_SVS_ROSEL},
	{     VPP_VLVL_SVS_L1,   940800,   1,  2,    800,     800,      674,           0,          1,            0,           1,                      0,                         0,                      0,                 0},
	{        VPP_VLVL_NOM,  1209600,   1,  3,    872,     872,      704,           1,          0,            1,           0,    CPR12_TARG_VOLT_NOM,        CPR12_QUOT_MIN_NOM, CPR12_QUOT_OFFSET_NOM,   CPR12_NOM_ROSEL},
	{     VPP_VLVL_NOM_L1,  1344000,   1,  4,    928,     928,      724,           0,          1,            0,           1,                      0,                         0,                      0,                 0},
	{        VPP_VLVL_TUR,  1363200,   1,  5,    988,     988,      724,           1,          0,            1,           0,    CPR12_TARG_VOLT_TUR,        CPR12_QUOT_MIN_TUR, CPR12_QUOT_OFFSET_TUR,   CPR12_TUR_ROSEL},
	{VPP_VLVL_INVALID}
};

static vpp_voltage_settings_t volt_setting_silver_v1[] = {
	/*               vlvl,     freq,  en, vc, ref_mV, ceil_mV, floor_mV, olv_fuse_pt, olv_interp, quot_fuse_pt, quot_interp,              fused_olv,                fused_quot,            quot_offset,         ro_select */
	{     VPP_VLVL_LOWSVS,   768000,   1,  0,    644,     644,      588,           1,          0,            1,           0,   CPR10_TARG_VOLT_LSVS,       CPR10_QUOT_MIN_LSVS,                      0,  CPR10_LSVS_ROSEL},
	{        VPP_VLVL_SVS,  1017600,   1,  1,    728,     728,      588,           1,          0,            1,           0,    CPR10_TARG_VOLT_SVS,        CPR10_QUOT_MIN_SVS, CPR10_QUOT_OFFSET_SVS,   CPR10_SVS_ROSEL},
	{     VPP_VLVL_SVS_L1,  1094400,   1,  2,    800,     800,      674,           0,          1,            0,           1,                      0,                         0,                      0,                 0},
	{        VPP_VLVL_NOM,  1497600,   1,  3,    872,     872,      704,           1,          0,            1,           0,    CPR10_TARG_VOLT_NOM,        CPR10_QUOT_MIN_NOM, CPR10_QUOT_OFFSET_NOM,   CPR10_NOM_ROSEL},
	{     VPP_VLVL_NOM_L1,  1612800,   1,  4,    928,     928,      724,           0,          1,            0,           1,                      0,                         0,                      0,                 0},
	{        VPP_VLVL_TUR,  1804800,   1,  5,    988,     988,      724,           1,          0,            1,           0,    CPR10_TARG_VOLT_TUR,        CPR10_QUOT_MIN_TUR, CPR10_QUOT_OFFSET_TUR,   CPR10_TUR_ROSEL},
	{      VPP_VLVL_BOOST,  2208000,   1,  6,    988,     988,      724,           1,          0,            1,           0,  CPR10_TARG_VOLT_BOOST,      CPR10_QUOT_MIN_BOOST, CPR10_QUOT_OFFSET_BOOST, CPR10_BOOST_ROSEL},
	{VPP_VLVL_INVALID}
};

static vpp_voltage_settings_t volt_setting_gold_v1[] = {
	/*               vlvl,     freq,  en, vc, ref_mV, ceil_mV, floor_mV, olv_fuse_pt, olv_interp, quot_fuse_pt, quot_interp,              fused_olv,                fused_quot,            quot_offset,         ro_select */
	{     VPP_VLVL_LOWSVS,   768000,   1,  0,    644,     644,      588,           1,          0,            1,           0,   CPR11_TARG_VOLT_LSVS,       CPR11_QUOT_MIN_LSVS,                      0,  CPR11_LSVS_ROSEL},
	{        VPP_VLVL_SVS,  1017600,   1,  1,    728,     728,      588,           1,          0,            1,           0,    CPR11_TARG_VOLT_SVS,        CPR11_QUOT_MIN_SVS, CPR11_QUOT_OFFSET_SVS,   CPR11_SVS_ROSEL},
	{     VPP_VLVL_SVS_L1,  1190400,   1,  2,    800,     800,      674,           0,          1,            0,           1,                      0,                         0,                      0,                 0},
	{        VPP_VLVL_NOM,  1497600,   1,  3,    872,     872,      704,           1,          0,            1,           0,    CPR11_TARG_VOLT_NOM,        CPR11_QUOT_MIN_NOM, CPR11_QUOT_OFFSET_NOM,   CPR11_NOM_ROSEL},
	{     VPP_VLVL_NOM_L1,  1708800,   1,  4,    928,     928,      756,           0,          1,            0,           1,                      0,                         0,                      0,                 0},
	{        VPP_VLVL_TUR,  1900800,   1,  5,    988,     988,      796,           1,          0,            1,           0,    CPR11_TARG_VOLT_TUR,        CPR11_QUOT_MIN_TUR, CPR11_QUOT_OFFSET_TUR,   CPR11_TUR_ROSEL},
	{      VPP_VLVL_BOOST,  2208000,   1,  6,    988,     988,      796,           1,          0,            1,           0,  CPR11_TARG_VOLT_BOOST,      CPR11_QUOT_MIN_BOOST, CPR11_QUOT_OFFSET_BOOST, CPR11_BOOST_ROSEL},
	{VPP_VLVL_INVALID}
};

vpp_voltage_settings_t *volt_setting_table[1][3] = {
	{
		volt_setting_silver_v1,
		volt_setting_l3_v1,
		volt_setting_gold_v1,
	},
};

/*---------------------------------------------------------------------------
 * Virtual (CPR local revision) Adjustments
 *--------------------------------------------------------------------------*/

/* Rows are {vlvl, freq_kHz, OLV adjustment, CLV adjustment}; omitted trailing
 * adjustment fields are zero.
 */
static vpp_virtual_adj_t virtual_adj_silver_v1_rev0_vpid1[] = {
	{ VPP_VLVL_LOWSVS,   768000},
	{    VPP_VLVL_SVS,  1017600},
	{ VPP_VLVL_SVS_L1,  1094400},
	{    VPP_VLVL_NOM,  1497600},
	{ VPP_VLVL_NOM_L1,  1612800},
	{    VPP_VLVL_TUR,  1804800},
	{  VPP_VLVL_BOOST,  2208000},
	{VPP_VLVL_INVALID}
};

static vpp_virtual_adj_t virtual_adj_silver_v1_rev1_vpid1[] = {
	{ VPP_VLVL_LOWSVS,   768000},
	{    VPP_VLVL_SVS,  1017600,  -4,  -4},
	{ VPP_VLVL_SVS_L1,  1094400,  -8,  -8},
	{    VPP_VLVL_NOM,  1497600, -12, -12},
	{ VPP_VLVL_NOM_L1,  1612800, -16, -16},
	{    VPP_VLVL_TUR,  1804800, -20, -20},
	{  VPP_VLVL_BOOST,  2208000},
	{VPP_VLVL_INVALID}
};

static vpp_virtual_adj_t virtual_adj_l3_v1_rev0_vpid1[] = {
	{ VPP_VLVL_LOWSVS,   556800},
	{    VPP_VLVL_SVS,   806400},
	{ VPP_VLVL_SVS_L1,   940800},
	{    VPP_VLVL_NOM,  1209600},
	{ VPP_VLVL_NOM_L1,  1344000},
	{    VPP_VLVL_TUR,  1363200},
	{VPP_VLVL_INVALID}
};

static vpp_virtual_adj_t virtual_adj_gold_v1_rev1_vpid1[] = {
	{ VPP_VLVL_LOWSVS,   768000},
	{    VPP_VLVL_SVS,  1017600,  -4,  -4},
	{ VPP_VLVL_SVS_L1,  1190400,  -8,  -8},
	{    VPP_VLVL_NOM,  1497600, -12, -12},
	{ VPP_VLVL_NOM_L1,  1708800, -16, -16},
	{    VPP_VLVL_TUR,  1900800, -20, -20},
	{  VPP_VLVL_BOOST,  2208000},
	{VPP_VLVL_INVALID}
};

static vpp_virtual_adj_t virtual_adj_l3_v1_rev1_vpid1[] = {
	{ VPP_VLVL_LOWSVS,   556800},
	{    VPP_VLVL_SVS,   806400,  -4,  -4},
	{ VPP_VLVL_SVS_L1,   940800,  -8,  -8},
	{    VPP_VLVL_NOM,  1209600,  -8,  -8},
	{ VPP_VLVL_NOM_L1,  1344000, -12, -12},
	{    VPP_VLVL_TUR,  1363200, -16, -16},
	{VPP_VLVL_INVALID}
};

static vpp_virtual_adj_t virtual_adj_gold_v1_rev0_vpid1[] = {
	{ VPP_VLVL_LOWSVS,   768000},
	{    VPP_VLVL_SVS,  1017600},
	{ VPP_VLVL_SVS_L1,  1190400},
	{    VPP_VLVL_NOM,  1497600},
	{ VPP_VLVL_NOM_L1,  1708800},
	{    VPP_VLVL_TUR,  1900800},
	{  VPP_VLVL_BOOST,  2208000},
	{VPP_VLVL_INVALID}
};

vpp_virtual_adj_t *virtual_adj_table[1][3][2] = {
	{
		{virtual_adj_silver_v1_rev0_vpid1, virtual_adj_silver_v1_rev1_vpid1},
		{virtual_adj_l3_v1_rev0_vpid1, virtual_adj_l3_v1_rev1_vpid1},
		{virtual_adj_gold_v1_rev0_vpid1, virtual_adj_gold_v1_rev1_vpid1},
	},
};

/*---------------------------------------------------------------------------
 * OSM Settings
 *--------------------------------------------------------------------------*/

static vpp_osm_t osm_gold_v1_sb1_rev0[] = {
	/*              vlvl,     freq, lval, div, src, cc, droop, dcvs_lval, cc_lval, memacc, vc,  acd_acdcr, acd_acdsscr, acd_avg_cfg_2 */
	{    VPP_VLVL_LOWSVS,   768000,   40,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,  1017600,   53,   0,   1,  4,     0,         0,       0,      0,  1, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_SVS_L1,  1190400,   62,   0,   1,  4,     0,         0,       0,      0,  2, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_NOM,  1497600,   78,   0,   1,  4,     0,         0,       0,      0,  3, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_NOM_L1,  1708800,   89,   0,   1,  4,     0,         0,       0,      0,  4, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_TUR,  1900800,   99,   0,   1,  4,     0,         0,       0,      0,  5, 0x882F5FFD,       0x9F1,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_silver_v1_sb1_rev1[] = {
	{    VPP_VLVL_LOWSVS,   768000,   40,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,  1017600,   53,   0,   1,  4,     0,         0,       0,      0,  1, 0x802D5FFD,       0x961,           0x0},
	{    VPP_VLVL_SVS_L1,  1094400,   57,   0,   1,  4,     0,         0,       0,      0,  2, 0x802C5FFD,       0x961,           0x0},
	{       VPP_VLVL_NOM,  1497600,   78,   0,   1,  4,     0,         0,       0,      0,  3, 0xA02B5FFD,       0x961,           0x0},
	{    VPP_VLVL_NOM_L1,  1612800,   84,   0,   1,  4,     0,         0,       0,      0,  4, 0x802B5FFD,       0x961,           0x0},
	{       VPP_VLVL_TUR,  1804800,   94,   0,   1,  4,     0,         0,       0,      0,  5, 0x802B5FFD,       0x961,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_l3_v1_sb0_rev0[] = {
	{    VPP_VLVL_LOWSVS,   556800,   29,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,   806400,   42,   0,   1,  4,     0,         0,       0,      0,  1, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_SVS_L1,   940800,   49,   0,   1,  4,     0,         0,       0,      0,  2, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_NOM,  1209600,   63,   0,   1,  4,     0,         0,       0,      0,  3, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_NOM_L1,  1344000,   70,   0,   1,  4,     0,         0,       0,      0,  4, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_TUR,  1363200,   71,   0,   1,  4,     0,         0,       0,      0,  5, 0x882F5FFD,       0x9F1,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_silver_v1_sb0_rev1[] = {
	{    VPP_VLVL_LOWSVS,   768000,   40,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,  1017600,   53,   0,   1,  4,     0,         0,       0,      0,  1, 0x802D5FFD,       0x961,           0x0},
	{    VPP_VLVL_SVS_L1,  1094400,   57,   0,   1,  4,     0,         0,       0,      0,  2, 0x802C5FFD,       0x961,           0x0},
	{       VPP_VLVL_NOM,  1497600,   78,   0,   1,  4,     0,         0,       0,      0,  3, 0xA02B5FFD,       0x961,           0x0},
	{    VPP_VLVL_NOM_L1,  1612800,   84,   0,   1,  4,     0,         0,       0,      0,  4, 0x802B5FFD,       0x961,           0x0},
	{       VPP_VLVL_TUR,  1804800,   94,   0,   1,  4,     0,         0,       0,      0,  5, 0x802B5FFD,       0x961,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_silver_v1_sb2_rev0[] = {
	{    VPP_VLVL_LOWSVS,   768000,   40,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,  1017600,   53,   0,   1,  4,     0,         0,       0,      0,  1, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_SVS_L1,  1094400,   57,   0,   1,  4,     0,         0,       0,      0,  2, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_NOM,  1497600,   78,   0,   1,  4,     0,         0,       0,      0,  3, 0x882F5FFD,       0x9F1,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_l3_v1_sb1_rev1[] = {
	{    VPP_VLVL_LOWSVS,   556800,   29,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,   806400,   42,   0,   1,  4,     0,         0,       0,      0,  1, 0xA02D5FFD,       0x961,           0x0},
	{    VPP_VLVL_SVS_L1,   940800,   49,   0,   1,  4,     0,         0,       0,      0,  2, 0xA02C5FFD,       0x961,           0x0},
	{       VPP_VLVL_NOM,  1209600,   63,   0,   1,  4,     0,         0,       0,      0,  3, 0x802C5FFD,       0x961,           0x0},
	{    VPP_VLVL_NOM_L1,  1344000,   70,   0,   1,  4,     0,         0,       0,      0,  4, 0xA02B5FFD,       0x961,           0x0},
	{       VPP_VLVL_TUR,  1363200,   71,   0,   1,  4,     0,         0,       0,      0,  5, 0xA02B5FFD,       0x961,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_silver_v1_sb1_rev0[] = {
	{    VPP_VLVL_LOWSVS,   768000,   40,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,  1017600,   53,   0,   1,  4,     0,         0,       0,      0,  1, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_SVS_L1,  1094400,   57,   0,   1,  4,     0,         0,       0,      0,  2, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_NOM,  1497600,   78,   0,   1,  4,     0,         0,       0,      0,  3, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_NOM_L1,  1612800,   84,   0,   1,  4,     0,         0,       0,      0,  4, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_TUR,  1804800,   94,   0,   1,  4,     0,         0,       0,      0,  5, 0x882F5FFD,       0x9F1,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_l3_v1_sb2_rev1[] = {
	{    VPP_VLVL_LOWSVS,   556800,   29,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,   806400,   42,   0,   1,  4,     0,         0,       0,      0,  1, 0xA02D5FFD,       0x961,           0x0},
	{    VPP_VLVL_SVS_L1,   940800,   49,   0,   1,  4,     0,         0,       0,      0,  2, 0xA02C5FFD,       0x961,           0x0},
	{       VPP_VLVL_NOM,  1209600,   63,   0,   1,  4,     0,         0,       0,      0,  3, 0x802C5FFD,       0x961,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_gold_v1_sb0_rev0[] = {
	{    VPP_VLVL_LOWSVS,   768000,   40,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,  1017600,   53,   0,   1,  4,     0,         0,       0,      0,  1, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_SVS_L1,  1190400,   62,   0,   1,  4,     0,         0,       0,      0,  2, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_NOM,  1497600,   78,   0,   1,  4,     0,         0,       0,      0,  3, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_NOM_L1,  1708800,   89,   0,   1,  4,     0,         0,       0,      0,  4, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_TUR,  1900800,   99,   0,   1,  4,     0,         0,       0,      0,  5, 0x882F5FFD,       0x9F1,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_l3_v1_sb2_rev0[] = {
	{    VPP_VLVL_LOWSVS,   556800,   29,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,   806400,   42,   0,   1,  4,     0,         0,       0,      0,  1, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_SVS_L1,   940800,   49,   0,   1,  4,     0,         0,       0,      0,  2, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_NOM,  1209600,   63,   0,   1,  4,     0,         0,       0,      0,  3, 0x882F5FFD,       0x9F1,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_silver_v1_sb2_rev1[] = {
	{    VPP_VLVL_LOWSVS,   768000,   40,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,  1017600,   53,   0,   1,  4,     0,         0,       0,      0,  1, 0x802D5FFD,       0x961,           0x0},
	{    VPP_VLVL_SVS_L1,  1094400,   57,   0,   1,  4,     0,         0,       0,      0,  2, 0x802C5FFD,       0x961,           0x0},
	{       VPP_VLVL_NOM,  1497600,   78,   0,   1,  4,     0,         0,       0,      0,  3, 0xA02B5FFD,       0x961,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_l3_v1_sb1_rev0[] = {
	{    VPP_VLVL_LOWSVS,   556800,   29,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,   806400,   42,   0,   1,  4,     0,         0,       0,      0,  1, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_SVS_L1,   940800,   49,   0,   1,  4,     0,         0,       0,      0,  2, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_NOM,  1209600,   63,   0,   1,  4,     0,         0,       0,      0,  3, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_NOM_L1,  1344000,   70,   0,   1,  4,     0,         0,       0,      0,  4, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_TUR,  1363200,   71,   0,   1,  4,     0,         0,       0,      0,  5, 0x882F5FFD,       0x9F1,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_l3_v1_sb0_rev1[] = {
	{    VPP_VLVL_LOWSVS,   556800,   29,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,   806400,   42,   0,   1,  4,     0,         0,       0,      0,  1, 0xA02D5FFD,       0x961,           0x0},
	{    VPP_VLVL_SVS_L1,   940800,   49,   0,   1,  4,     0,         0,       0,      0,  2, 0xA02C5FFD,       0x961,           0x0},
	{       VPP_VLVL_NOM,  1209600,   63,   0,   1,  4,     0,         0,       0,      0,  3, 0x802C5FFD,       0x961,           0x0},
	{    VPP_VLVL_NOM_L1,  1344000,   70,   0,   1,  4,     0,         0,       0,      0,  4, 0xA02B5FFD,       0x961,           0x0},
	{       VPP_VLVL_TUR,  1363200,   71,   0,   1,  4,     0,         0,       0,      0,  5, 0xA02B5FFD,       0x961,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_gold_v1_sb1_rev1[] = {
	{    VPP_VLVL_LOWSVS,   768000,   40,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,  1017600,   53,   0,   1,  4,     0,         0,       0,      0,  1, 0x802D5FFD,       0x961,           0x0},
	{    VPP_VLVL_SVS_L1,  1190400,   62,   0,   1,  4,     0,         0,       0,      0,  2, 0x802C5FFD,       0x961,           0x0},
	{       VPP_VLVL_NOM,  1497600,   78,   0,   1,  4,     0,         0,       0,      0,  3, 0xA02B5FFD,       0x961,           0x0},
	{    VPP_VLVL_NOM_L1,  1708800,   89,   0,   1,  4,     0,         0,       0,      0,  4, 0x802B5FFD,       0x961,           0x0},
	{       VPP_VLVL_TUR,  1900800,   99,   0,   1,  4,     0,         0,       0,      0,  5, 0x802B5FFD,       0x961,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_gold_v1_sb2_rev1[] = {
	{    VPP_VLVL_LOWSVS,   768000,   40,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,  1017600,   53,   0,   1,  4,     0,         0,       0,      0,  1, 0x802D5FFD,       0x961,           0x0},
	{    VPP_VLVL_SVS_L1,  1190400,   62,   0,   1,  4,     0,         0,       0,      0,  2, 0x802C5FFD,       0x961,           0x0},
	{       VPP_VLVL_NOM,  1497600,   78,   0,   1,  4,     0,         0,       0,      0,  3, 0xA02B5FFD,       0x961,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_gold_v1_sb2_rev0[] = {
	{    VPP_VLVL_LOWSVS,   768000,   40,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,  1017600,   53,   0,   1,  4,     0,         0,       0,      0,  1, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_SVS_L1,  1190400,   62,   0,   1,  4,     0,         0,       0,      0,  2, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_NOM,  1497600,   78,   0,   1,  4,     0,         0,       0,      0,  3, 0x882F5FFD,       0x9F1,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_gold_v1_sb0_rev1[] = {
	{    VPP_VLVL_LOWSVS,   768000,   40,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,  1017600,   53,   0,   1,  4,     0,         0,       0,      0,  1, 0x802D5FFD,       0x961,           0x0},
	{    VPP_VLVL_SVS_L1,  1190400,   62,   0,   1,  4,     0,         0,       0,      0,  2, 0x802C5FFD,       0x961,           0x0},
	{       VPP_VLVL_NOM,  1497600,   78,   0,   1,  4,     0,         0,       0,      0,  3, 0xA02B5FFD,       0x961,           0x0},
	{    VPP_VLVL_NOM_L1,  1708800,   89,   0,   1,  4,     0,         0,       0,      0,  4, 0x802B5FFD,       0x961,           0x0},
	{       VPP_VLVL_TUR,  1900800,   99,   0,   1,  4,     0,         0,       0,      0,  5, 0x802B5FFD,       0x961,           0x0},
	{VPP_VLVL_INVALID}
};

static vpp_osm_t osm_silver_v1_sb0_rev0[] = {
	{    VPP_VLVL_LOWSVS,   768000,   40,   0,   1,  4,     0,         0,       0,      0,  0, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_SVS,  1017600,   53,   0,   1,  4,     0,         0,       0,      0,  1, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_SVS_L1,  1094400,   57,   0,   1,  4,     0,         0,       0,      0,  2, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_NOM,  1497600,   78,   0,   1,  4,     0,         0,       0,      0,  3, 0x882F5FFD,       0x9F1,           0x0},
	{    VPP_VLVL_NOM_L1,  1612800,   84,   0,   1,  4,     0,         0,       0,      0,  4, 0x882F5FFD,       0x9F1,           0x0},
	{       VPP_VLVL_TUR,  1804800,   94,   0,   1,  4,     0,         0,       0,      0,  5, 0x882F5FFD,       0x9F1,           0x0},
	{VPP_VLVL_INVALID}
};

vpp_osm_t *osm_table[1][3][3][2] = {
	{
		{
			{osm_silver_v1_sb0_rev0, osm_silver_v1_sb0_rev1},
			{osm_silver_v1_sb1_rev0, osm_silver_v1_sb1_rev1},
			{osm_silver_v1_sb2_rev0, osm_silver_v1_sb2_rev1},
		},
		{
			{osm_l3_v1_sb0_rev0, osm_l3_v1_sb0_rev1},
			{osm_l3_v1_sb1_rev0, osm_l3_v1_sb1_rev1},
			{osm_l3_v1_sb2_rev0, osm_l3_v1_sb2_rev1},
		},
		{
			{osm_gold_v1_sb0_rev0, osm_gold_v1_sb0_rev1},
			{osm_gold_v1_sb1_rev0, osm_gold_v1_sb1_rev1},
			{osm_gold_v1_sb2_rev0, osm_gold_v1_sb2_rev1},
		},
	},
};

/*---------------------------------------------------------------------------
 * KV Data
 *--------------------------------------------------------------------------*/

static vpp_kv_t kv_gold_v1[] = {
	{ VPP_VLVL_LOWSVS,     VPP_VLVL_SVS, {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 2380,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 1380,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 3380,    0,    0} },
	{    VPP_VLVL_SVS,     VPP_VLVL_NOM, {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 2340,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 1340,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 3340,    0,    0} },
	{    VPP_VLVL_NOM,     VPP_VLVL_TUR, {   0,    0,    0,    0,    0,    0,    0, 1890,    0,    0,    0,    0,    0,    0,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,  890,    0,    0,    0,    0,    0,    0,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0, 2890,    0,    0,    0,    0,    0,    0,    0,    0} },
	{    VPP_VLVL_TUR,   VPP_VLVL_BOOST, {   0,    0,    0,    0,    0,    0,    0, 1780,    0,    0,    0,    0,    0,    0,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,  780,    0,    0,    0,    0,    0,    0,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0, 2780,    0,    0,    0,    0,    0,    0,    0,    0} },
	{VPP_VLVL_INVALID}
};

static vpp_kv_t kv_l3_v1[] = {
	{ VPP_VLVL_LOWSVS,     VPP_VLVL_SVS, {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 2390,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 1390,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 3390,    0,    0} },
	{    VPP_VLVL_SVS,     VPP_VLVL_NOM, {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 2350,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 1350,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 3350,    0,    0} },
	{    VPP_VLVL_NOM,     VPP_VLVL_TUR, {   0,    0,    0,    0,    0,    0,    0, 1900,    0,    0,    0,    0,    0,    0,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,  900,    0,    0,    0,    0,    0,    0,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0, 2900,    0,    0,    0,    0,    0,    0,    0,    0} },
	{VPP_VLVL_INVALID}
};

static vpp_kv_t kv_silver_v1[] = {
	{ VPP_VLVL_LOWSVS,     VPP_VLVL_SVS, {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 2380,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 1380,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 3380,    0,    0} },
	{    VPP_VLVL_SVS,     VPP_VLVL_NOM, {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 2340,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 1340,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 3340,    0,    0} },
	{    VPP_VLVL_NOM,     VPP_VLVL_TUR, {   0,    0,    0,    0,    0,    0,    0, 1890,    0,    0,    0,    0,    0,    0,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,  890,    0,    0,    0,    0,    0,    0,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0, 2890,    0,    0,    0,    0,    0,    0,    0,    0} },
	{    VPP_VLVL_TUR,   VPP_VLVL_BOOST, {   0,    0,    0,    0,    0,    0,    0, 1780,    0,    0,    0,    0,    0,    0,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0,  780,    0,    0,    0,    0,    0,    0,    0,    0},
	                                     {   0,    0,    0,    0,    0,    0,    0, 2780,    0,    0,    0,    0,    0,    0,    0,    0} },
	{VPP_VLVL_INVALID}
};

vpp_kv_t *kv_table[1][3] = {
	{
		kv_silver_v1,
		kv_l3_v1,
		kv_gold_v1,
	},
};

/*---------------------------------------------------------------------------
 * Fuse Adjustments
 *--------------------------------------------------------------------------*/

static vpp_fuse_adj_t fuse_adj_silver_v1_rev0_vpid1[] = {
	{ VPP_VLVL_LOWSVS,   0, -20,   0, -20,   0,  24},
	{    VPP_VLVL_SVS,   0, -20,   0, -20,   0,  24},
	{    VPP_VLVL_NOM,  -4, -36,  -4, -36,   0,  32},
	{    VPP_VLVL_TUR, -20, -52, -20, -52,   0,  32},
	{  VPP_VLVL_BOOST,   0,   0,   0,   0,   0,  32},
	{VPP_VLVL_INVALID}
};

static vpp_fuse_adj_t fuse_adj_silver_v1_rev1_vpid1[] = {
	{ VPP_VLVL_LOWSVS,   8, -34,  -7, -49, -10,  24},
	{    VPP_VLVL_SVS,   4, -36, -11, -51, -10,  24},
	{    VPP_VLVL_NOM,  10, -24,  -5, -39, -10,  32},
	{    VPP_VLVL_TUR,  22,  -8,   7, -23, -10,  32},
	{  VPP_VLVL_BOOST,   0,   0, -15, -15, -10,  32},
	{VPP_VLVL_INVALID}
};

static vpp_fuse_adj_t fuse_adj_l3_v1_rev0_vpid1[] = {
	{ VPP_VLVL_LOWSVS, -16, -40, -16, -40,   0,  24},
	{    VPP_VLVL_SVS, -16, -40, -16, -40,   0,  24},
	{    VPP_VLVL_NOM, -24, -56, -24, -56,   0,  32},
	{    VPP_VLVL_TUR, -80, -112, -80, -112,  0,  32},
	{VPP_VLVL_INVALID}
};

static vpp_fuse_adj_t fuse_adj_gold_v1_rev1_vpid1[] = {
	{ VPP_VLVL_LOWSVS,  -4,  -4, -19, -19, -10,   0},
	{    VPP_VLVL_SVS,  -8,  -8, -23, -23, -10,   0},
	{    VPP_VLVL_NOM,   8,   8,  -7,  -7, -10,   0},
	{    VPP_VLVL_TUR,  17,  17,   2,   2, -10,   0},
	{  VPP_VLVL_BOOST,   0,   0, -15, -15, -10,   0},
	{VPP_VLVL_INVALID}
};

static vpp_fuse_adj_t fuse_adj_l3_v1_rev1_vpid1[] = {
	{ VPP_VLVL_LOWSVS,   8, -34,  -7, -49, -10,  24},
	{    VPP_VLVL_SVS,   4, -36, -11, -51, -10,  24},
	{    VPP_VLVL_NOM,  10, -26,  -5, -41, -10,  32},
	{    VPP_VLVL_TUR,   8, -18,  -7, -33, -10,  32},
	{VPP_VLVL_INVALID}
};

static vpp_fuse_adj_t fuse_adj_gold_v1_rev0_vpid1[] = {
	{ VPP_VLVL_LOWSVS,   0, -20,   0, -20,   0,   0},
	{    VPP_VLVL_SVS,   0, -20,   0, -20,   0,   0},
	{    VPP_VLVL_NOM,  -4, -36,  -4, -36,   0,   0},
	{    VPP_VLVL_TUR,  -8, -40,  -8, -40,   0,   0},
	{  VPP_VLVL_BOOST,   0,   0,   0,   0,   0,   0},
	{VPP_VLVL_INVALID}
};

vpp_fuse_adj_t *fuse_adj_table[1][3][2] = {
	{
		{fuse_adj_silver_v1_rev0_vpid1, fuse_adj_silver_v1_rev1_vpid1},
		{fuse_adj_l3_v1_rev0_vpid1, fuse_adj_l3_v1_rev1_vpid1},
		{fuse_adj_gold_v1_rev0_vpid1, fuse_adj_gold_v1_rev1_vpid1},
	},
};
