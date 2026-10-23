/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * ICB configuration query data for the Shikra platform.
 * Provides icbcfg_info, consumed by the ICB configuration driver.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/chipinfo/chipinfo.h>
#include <lib/utils_def.h>

#include "icbcfg_hwio.h"
#include "icbcfg_query.h"

/*
 * One-time register configuration data.
 */
static struct icbcfg_data icbcfg_data_shikra_v1[] = {
	ICBCFG_HWIO_DW(SNOC_DCD_PIMEM_QXCLK_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SNOC_DCD_CENTER_QXCLK_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SNOC_DCD_MM_SF_QXCLK_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SNOC_DCD_MM_HF_QXCLK_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SNOC_DCD_LPASS_QXCLK_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SNOC_DCD_WCSS_QXCLK_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SNOC_DCD_MODEM_QXCLK_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_APSS_QHCLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_CDSP_QXCLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_CENTER_PEIPH_CLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_CNOC_SF_QXCLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_CNOC_SLAVE_SF_QXCLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_EMAC0_QXCLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_EMAC1_QXCLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_MCUSS_QXCLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_MEMNOC_SF_QXCLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_PCIE2_QXCLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_QDSS_STM_CLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_QUP0_PERIPH_CLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_USB2_QXCLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_USB3_QXCLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_WEST_PEIPH_CLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(SYSTEM_NOC_SNOC_DCD_WEST_QXCLK_DCD_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(CNOC_CENTER_DCD_QHCLK_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(CNOC_WEST_DCD_QHCLK_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(CNOC_NORTH_DCD_QHCLK_MAXDIV_LOW, 0x4U),
	ICBCFG_HWIO_DW(CONFIG_NOC_CNOC_PCIE_DCD_QHCLK_DCD_MAXDIV_LOW, 0x4U),
};

static struct icbcfg_prop icbcfg_prop_shikra_v1 = {
	.len  = ARRAY_SIZE(icbcfg_data_shikra_v1),
	.data = icbcfg_data_shikra_v1,
};

static struct icbcfg_device_config shikra_v1 = {
	.family         = CHIPINFO_FAMILY_SHIKRA,
	.match          = false,
	.version        = CHIPINFO_VERSION(1, 0),
	.prop_data      = &icbcfg_prop_shikra_v1,
	.post_prop_data = NULL,
};

/* Definitions list */
static struct icbcfg_device_config *configs_shikra[] = {
	[0] = &shikra_v1,
};

/**
 * icbcfg_info - exported ICB configuration descriptor for Shikra.
 *
 * Consumed by the common icbcfg driver to select the correct SKU
 * variant and apply one-time NOC register writes at boot.
 */
struct icbcfg_info icbcfg_info = {
	.num_configs = ARRAY_SIZE(configs_shikra),
	.configs     = configs_shikra,
};
