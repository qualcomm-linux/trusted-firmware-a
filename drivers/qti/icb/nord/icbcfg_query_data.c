/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * ICB configuration query data for the Nord (SA8797P / NordAU) platform.
 * Provides icbcfg_info, consumed by the ICB configuration driver.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/chipinfo/chipinfo.h>
#include <lib/utils_def.h>

#include "icbcfg_hwio.h"
#include "icbcfg_query.h"

/*---------------------------------------------------------------------------*/
/* NORD                                                                      */
/*---------------------------------------------------------------------------*/
static struct icbcfg_data icbcfg_data_nord_v1[] = {
	ICBCFG_HWIO_DW(A1_NOC_AGGRE_NOC_ANOC_1_QTB500_QTB500_QTBOVRUARCHREG_LOW, 0x8f000000U),
	ICBCFG_HWIO_DW(A1_NOC_AGGRE_NOC_ANOC_1_QTB500_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
	ICBCFG_HWIO_DW(A1_NOC_TILEAGGRE_NOC_ANOC_1_TILE_QTB500_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
	ICBCFG_HWIO_DW(A2_NOC_AGGRE_NOC_ANOC_2_QTB500_QTB500_QTBOVRUARCHREG_LOW, 0x8f000000U),
	ICBCFG_HWIO_DW(A2_NOC_AGGRE_NOC_ANOC_2_QTB500_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
	ICBCFG_HWIO_DW(A2_NOC_TILEAGGRE_NOC_ANOC_2_TILE_QTB500_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
	ICBCFG_HWIO_DW(HPASS_ADAS_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
	ICBCFG_HWIO_DW(HPASS_AUDIO_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
	ICBCFG_HWIO_DW(MMSS_NOC_SFTGTQTB500_QTB500_QTBOVRUARCHREG_LOW, 0x8f000000U),
	ICBCFG_HWIO_DW(MMSS_NOC_SFTGTQTB500_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
	ICBCFG_HWIO_DW(MMSS_NOC_MDP0QTB500_QTB500_QTBOVRUARCHREG_LOW, 0x81000000U),
	ICBCFG_HWIO_DW(MMSS_NOC_MDP0QTB500_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007cU),
	ICBCFG_HWIO_DW(MMSS_NOC_CAMHFQTB500_QTB500_QTBOVRUARCHREG_LOW, 0x81000000U),
	ICBCFG_HWIO_DW(MMSS_NOC_CAMHFQTB500_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007cU),
	ICBCFG_HWIO_DW(MMSS_NOC_DPRX0HFQTB500_QTB500_QTBOVRUARCHREG_LOW, 0x81000000U),
	ICBCFG_HWIO_DW(MMSS_NOC_DPRX0HFQTB500_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007cU),
	ICBCFG_HWIO_DW(MMSS_NOC_DPRX1HFQTB500_QTB500_QTBOVRUARCHREG_LOW, 0x81000000U),
	ICBCFG_HWIO_DW(MMSS_NOC_DPRX1HFQTB500_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007cU),
	ICBCFG_HWIO_DW(MMSS_NOC_MDP1QTB500_QTB500_QTBOVRUARCHREG_LOW, 0x81000000U),
	ICBCFG_HWIO_DW(MMSS_NOC_MDP1QTB500_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007cU),
	ICBCFG_HWIO_DW(PCIE_NOC_DATA_INBOUND_PCIE_DATA_INBOUND_PCIE_CTRL_0_QTB600_QTB600_CONFIGURATION2_QTBUARCHREG_LOW,
		       0x00000008U),
	ICBCFG_HWIO_DW(PCIE_NOC_DATA_INBOUND_PCIE_DATA_INBOUND_PCIE_CTRL_1_QTB600_QTB600_CONFIGURATION2_QTBUARCHREG_LOW,
		       0x00000008U),
	ICBCFG_HWIO_DW(PCIE_NOC_DATA_INBOUND_PCIE_DATA_INBOUND_PCIE_CTRL_2_QTB600_QTB600_CONFIGURATION2_QTBUARCHREG_LOW,
		       0x00000008U),
	ICBCFG_HWIO_DW(PCIE_NOC_DATA_INBOUND_PCIE_DATA_INBOUND_PCIE_CTRL_3_QTB600_QTB600_CONFIGURATION2_QTBUARCHREG_LOW,
		       0x00000008U),
	ICBCFG_HWIO_DW(SAFEDMA_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
	ICBCFG_HWIO_DW(SAIL_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
};

static struct icbcfg_prop icbcfg_prop_common_nord_v1 = {
	/* Length of the config data array */
	.len = ARRAY_SIZE(icbcfg_data_nord_v1),
	/* Pointer to config data array */
	.data = icbcfg_data_nord_v1,
};

/* Separated for NSP Qultivate Parts */
/* NSP 0 */
static struct icbcfg_data icbcfg_data_nsp0_nord_v1[] = {
	ICBCFG_HWIO_DW(NSP_00_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
};

static struct icbcfg_prop icbcfg_prop_nsp0_nord_v1 = {
	/* Length of the config data array */
	.len = ARRAY_SIZE(icbcfg_data_nsp0_nord_v1),
	/* Pointer to config data array */
	.data = icbcfg_data_nsp0_nord_v1,
	/* Number of qultivate_part_type */
	.num_qtv_parts = 1U,
	/* Qultivate Type array */
	.qtv_parts = (struct icbcfg_qtv []){
		{ .part = CHIPINFO_PART_NSP, .part_idx = 0U },
	},
};

/* NSP 1 */
static struct icbcfg_data icbcfg_data_nsp1_nord_v1[] = {
	ICBCFG_HWIO_DW(NSP_01_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
};

static struct icbcfg_prop icbcfg_prop_nsp1_nord_v1 = {
	/* Length of the config data array */
	.len = ARRAY_SIZE(icbcfg_data_nsp1_nord_v1),
	/* Pointer to config data array */
	.data = icbcfg_data_nsp1_nord_v1,
	/* Number of qultivate_part_type */
	.num_qtv_parts = 1U,
	/* Qultivate Type array */
	.qtv_parts = (struct icbcfg_qtv []){
		{ .part = CHIPINFO_PART_NSP, .part_idx = 1U },
	},
};

/* NSP 2 */
static struct icbcfg_data icbcfg_data_nsp2_nord_v1[] = {
	ICBCFG_HWIO_DW(NSP_02_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
};

static struct icbcfg_prop icbcfg_prop_nsp2_nord_v1 = {
	/* Length of the config data array */
	.len = ARRAY_SIZE(icbcfg_data_nsp2_nord_v1),
	/* Pointer to config data array */
	.data = icbcfg_data_nsp2_nord_v1,
	/* Number of qultivate_part_type */
	.num_qtv_parts = 1U,
	/* Qultivate Type array */
	.qtv_parts = (struct icbcfg_qtv []){
		{ .part = CHIPINFO_PART_NSP, .part_idx = 2U },
	},
};

/* NSP 3 */
static struct icbcfg_data icbcfg_data_nsp3_nord_v1[] = {
	ICBCFG_HWIO_DW(NSP_03_QTB500_QTBOVRADDRHAZD_LOW, 0x0000007bU),
};

static struct icbcfg_prop icbcfg_prop_nsp3_nord_v1 = {
	/* Length of the config data array */
	.len = ARRAY_SIZE(icbcfg_data_nsp3_nord_v1),
	/* Pointer to config data array */
	.data = icbcfg_data_nsp3_nord_v1,
	/* Number of qultivate_part_type */
	.num_qtv_parts = 1U,
	/* Qultivate Type array */
	.qtv_parts = (struct icbcfg_qtv []){
		{ .part = CHIPINFO_PART_NSP, .part_idx = 3U },
	},
};

/* Nord configuration segments */
static const struct icbcfg_prop *icbcfg_prop_segs_nord_v1[] = {
	[0] = &icbcfg_prop_common_nord_v1,
	[1] = &icbcfg_prop_nsp0_nord_v1,
	[2] = &icbcfg_prop_nsp1_nord_v1,
	[3] = &icbcfg_prop_nsp2_nord_v1,
	[4] = &icbcfg_prop_nsp3_nord_v1,
};

static struct icbcfg_prop_list icbcfg_prop_list_nord_v1 = {
	/* Length of the config data array */
	.len = ARRAY_SIZE(icbcfg_prop_segs_nord_v1),
	/* Pointer to config data array */
	.segs = icbcfg_prop_segs_nord_v1,
};

static struct icbcfg_device_config nord = {
	/* Chip version information for this device data. */
	.family  = (uint32_t)CHIPINFO_FAMILY_NORDAU, /**< Chip family */
	.match   = false,                            /**< Exact match for version? */
	.version = CHIPINFO_VERSION(1, 0),           /**< Chip version */

	/* Prop lists. */
	.prop_data           = NULL, /**< Init time prop list. */
	.post_prop_data      = NULL, /** Unused **/
	.prop_data_list      = &icbcfg_prop_list_nord_v1,
	.post_prop_data_list = NULL,
};

/* Definitions list */
static struct icbcfg_device_config *configs_nord[] = {
	[0] = &nord,
};

/* Exported target definitions */
struct icbcfg_info icbcfg_info = {
	.num_configs = ARRAY_SIZE(configs_nord),
	.configs     = configs_nord,
};