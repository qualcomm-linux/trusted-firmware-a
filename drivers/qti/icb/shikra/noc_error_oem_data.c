/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * NoC error OEM configuration data for the Shikra platform.
 * Provides nocerr_config_info_oem, consumed by drivers/qti/icb/noc_error.c.
 */

#include <drivers/qti/chipinfo/chipinfo.h>

#include "noc_error.h"

/* Shikra OEM NOC configuration data - 5 NOC instances */
static struct nocerr_info_type_oem nocerr_cfgdata_oem[] = {
	/* [0] CONFIG_NOC */
	{
		.name        = "CONFIG_NOC",
		.intr_enable = true,
		.error_fatal = true,
		.sbms        = (struct nocerr_sbm_info_oem []){
			[0] = { .faultin_en0_low = 0xffffffffU,
				.faultin_en0_high = 0xffffU },
			[1] = { .faultin_en0_low = 0x1ffU },
			[2] = { .faultin_en0_low = 0xfU },
			[3] = { .faultin_en0_low = 0x3U },
			[4] = { .faultin_en0_low = 0xfU },
		},
		.obs_mask    = (struct nocerr_sbm_info_oem []){
			[0] = { .faultin_en0_low = 0x1U,
				.faultin_en0_high = 0x0U },
			[1] = { .faultin_en0_low = 0x0U },
			[2] = { .faultin_en0_low = 0x0U },
			[3] = { .faultin_en0_low = 0x0U },
			[4] = { .faultin_en0_low = 0xfU },
		},
		.to_reg_vals = (uint32_t []){
			0x100U, /* REFCTL0_LOW direct timeout ex clock */
			0x100U, /* REFCTL1_LOW */
			0x100U, /* REFCTL2_LOW */
			0x100U, /* REFCTL3_LOW */
			0x100U, /* REFCTL4_LOW */
			0x100U, /* REFCTL5_LOW */
			0x100U, /* REFCTL6_LOW */
			0x100U, /* REFCTL7_LOW */
			0x100U, /* REFCTL8_LOW */
			0x100U, /* REFCTL9_LOW */
			0x100U, /* REFCTL10_LOW */
			0x100U, /* REFCTL11_LOW */
			0x100U, /* REFCTL12_LOW */
			0x100U, /* REFCTL13_LOW */
			0x100U, /* REFCTL14_LOW */
			0x100U, /* REFCTL15_LOW */
			0x100U, /* REFCTL16_LOW */
			0x100U, /* REFCTL17_LOW */
			0x100U, /* REFCTL18_LOW */
			0x100U, /* REFCTL19_LOW */
			0x100U, /* REFCTL20_LOW */
			0x100U, /* REFCTL21_LOW */
			0x100U, /* REFCTL22_LOW */
			0x100U, /* REFCTL23_LOW */
			0x100U, /* REFCTL24_LOW */
			0x100U, /* REFCTL25_LOW */
			0x100U, /* REFCTL26_LOW */
			0x100U, /* REFCTL27_LOW */
			0x105U, /* REFCTL28_LOW West slave */
			/*
			 * REFCTL29_LOW West slave increased to 218ms due to
			 * SDC timeout temp fix.
			 */
			0x105U,
			0x1U,   /* MAINCTL_LOW */
		},
	},
	/* [1] SYSTEM_NOC */
	{
		.name        = "SYSTEM_NOC",
		.intr_enable = true,
		.error_fatal = true,
		.sbms        = (struct nocerr_sbm_info_oem []){
			[0] = { .faultin_en0_low = 0x3f7U },
		},
		.obs_mask    = (struct nocerr_sbm_info_oem []){
			[0] = { .faultin_en0_low = 0x1U },
		},
		.to_reg_vals = (uint32_t []){
			/*
			 * 512 / 2.4 / (1 << 5) => 6.8ms => 19.2/4/2 => 2.4MHz
			 * => snoc qos gen clock as ref 19.2/div2/div4.
			 */
			0x105U, /* REFCTL0_LOW */
			0x105U, /* REFCTL1_LOW */
			0x0U,   /* REFCTL2_LOW wcss */
			0x105U, /* REFCTL3_LOW */
			0x105U, /* REFCTL4_LOW imem */
			0x105U, /* REFCTL5_LOW qxs_pimem */
			0x109U, /* REFCTL6_LOW pcie2_0 109 ms */
			0x105U, /* REFCTL7_LOW */
			0x105U, /* REFCTL8_LOW */
			0x1U,   /* MAINCTL_LOW */
		},
	},
	/* [2] MEM_NOC_CORE */
	{
		.name        = "MEM_NOC_CORE",
		.intr_enable = true,
		.error_fatal = true,
		.sbms        = (struct nocerr_sbm_info_oem []){
			[0] = { .faultin_en0_low = 0x3fefffU },
		},
		.obs_mask    = (struct nocerr_sbm_info_oem []){
			[0] = { .faultin_en0_low = 0x1U },
		},
		.to_reg_vals = (uint32_t []){
			0x0U,   /* REFCTL0_LOW pd_memnoc */
			0x108U, /* REFCTL1_LOW 512 / (19.2 / (1 << 8)) => 6.8ms */
			0x108U, /* REFCTL2_LOW qns_llcc0_256CB */
			0x108U, /* REFCTL3_LOW qns_llcc1_256CB */
			0x108U, /* REFCTL4_LOW qns_pcie TBD */
			0x108U, /* REFCTL5_LOW qns_snoc */
			0x1U,   /* MAINCTL_LOW */
		},
	},
	/* [3] DEFRAG_NOC */
	{
		.name        = "DEFRAG_NOC",
		.intr_enable = true,
		.error_fatal = true,
		.sbms        = (struct nocerr_sbm_info_oem []){
			[0] = { .faultin_en0_low = 0x7U },
		},
		.obs_mask    = (struct nocerr_sbm_info_oem []){
			[0] = { .faultin_en0_low = 0x1U },
		},
		.to_reg_vals = (uint32_t []){
			0x0U,   /* REFCTL0_LOW pd_memnoc */
			0x108U, /* REFCTL1_LOW qns_llcc0 */
			0x108U, /* REFCTL2_LOW qns_llcc1 */
			0x1U,   /* MAINCTL_LOW */
		},
	},
	/* [4] DC_NOC */
	{
		.name        = "DC_NOC",
		.intr_enable = true,
		.error_fatal = true,
		.sbms        = (struct nocerr_sbm_info_oem []){
			[0] = { .faultin_en0_low = 0xfbffU },
		},
		.obs_mask    = (struct nocerr_sbm_info_oem []){
			[0] = { .faultin_en0_low = 0x1U },
		},
		.to_reg_vals = (uint32_t []){
			0x108U, /* REFCTL0_LOW */
			0x108U, /* REFCTL1_LOW */
			0x108U, /* REFCTL2_LOW */
			0x108U, /* REFCTL3_LOW */
			0x108U, /* REFCTL4_LOW */
			0x108U, /* REFCTL5_LOW */
			0x108U, /* REFCTL6_LOW */
			0x1U,   /* MAINCTL_LOW */
		},
	},
};

/* External timeout clock register values */
uint32_t clock_reg_vals[] = {
	[0] = 0x1U, /* Enable timeout reference clock */
	[1] = 0x3U, /* Set timeout divider to maximum, timeout = ~6.8 ms */
	/*
	 * Second divider to DIV512, so our timeout reference is 19.2 MHz.
	 * With a timeout counter of 512 (default with refctl), this means we
	 * have a timeout of 6.8 ms base. (512)/(19.2MHz/4/64)=6.8 ms as per
	 * GCC HPG.
	 */
	[2] = 0x3FU,
};

/* Global OEM configuration info exported to the common driver */
struct nocerr_config_info_oem nocerr_config_info_oem = {
	.num_configs = 1U,
	.configs     = (struct nocerr_propdata_type_oem []){
		/* Target info: Shikra v1.0 */
		[0] = {
			.family         = CHIPINFO_FAMILY_SHIKRA,
			.match          = false,
			.version        = CHIPINFO_VERSION(1, 0),
			.len            = sizeof(nocerr_cfgdata_oem) /
					  sizeof(nocerr_cfgdata_oem[0]),
			.noc_info_oem   = nocerr_cfgdata_oem,
			.clock_reg_vals = clock_reg_vals,
			.filters        = NULL,
			.reg_addr       = NULL,
			.reg_mask       = 0U,
			.reg_val        = 0U,
		},
	},
};
