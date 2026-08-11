/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * NoC error OEM configuration data for the Nord (SA8797P / NordAU) platform.
 *
 * This target has NO safety module in silicon: the .sfty_ctl OEM field
 * has been omitted from every instance.
 *
 * Provides nocerr_config_info_oem, consumed by drivers/qti/icb/noc_error.c.
 */

#include "noc_error.h"

/* Nord QLI OEM NOC configuration data - 64 NOC instances (matches noc_error_data.c) */
static struct nocerr_info_type_oem nocerr_cfgdata_oem[] = {
	/* [0] CNOC_MAIN */
	{
		.name	= "CNOC_MAIN",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x1000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFFFFFFFU,
		.faultin_en0_high = 0x7U },
      { .faultin_en0_low  = 0x19U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1U,
		.faultin_en0_high = 0x0U },
	{ .faultin_en0_low  = 0x19U },
      },
		.to_reg_vals	= (uint32_t []){ 0x106U, /**< REFCTL0_LOW */ /* 3.4 ms = 1024U/(19.2e6/(1U<<6U)) */
				  0x106U, /**< REFCTL1_LOW */
				  0x106U, /**< REFCTL2_LOW */
				  0x106U, /**< REFCTL3_LOW */
				  0x106U, /**< REFCTL4_LOW */
				  0x106U, /**< REFCTL5_LOW */
				  0x106U, /**< REFCTL6_LOW */
				  0x106U, /**< REFCTL7_LOW */
				  0x106U, /**< REFCTL8_LOW */
				  0x106U, /**< REFCTL9_LOW */
				  0x106U, /**< REFCTL10_LOW */
				  0x106U, /**< REFCTL11_LOW */
				  0x106U, /**< REFCTL12_LOW */
				  0x106U, /**< REFCTL13_LOW */
				  0x106U, /**< REFCTL14_LOW */
				  0x106U, /**< REFCTL15_LOW */
				  0x106U, /**< REFCTL16_LOW */
				  0x106U, /**< REFCTL17_LOW */
				  0x107U, /**< REFCTL18_LOW */ /* qss_ddrss_cfg 6.8 ms */
				  0x106U, /**< REFCTL19_LOW */
				  0x106U, /**< REFCTL20_LOW */
				  0x106U, /**< REFCTL21_LOW */
				  0x106U, /**< REFCTL22_LOW */
				  0x106U, /**< REFCTL23_LOW */
				  0x106U, /**< REFCTL24_LOW */
				  0x106U, /**< REFCTL25_LOW */
				  0x106U, /**< REFCTL26_LOW */
				  0x106U, /**< REFCTL27_LOW */
				  0x1U,  /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL0_LOW 13ms */
				  0x10FU, /**< TRCHECK_0_REFCTL1_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL2_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL3_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL4_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL5_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL6_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL7_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL8_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL9_LOW */
				  0x1U,  /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL1_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL2_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL3_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL4_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL5_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL6_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL7_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL8_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL9_LOW */
				  0x1U,  /**< MAINCTL_LOW */
				  0x106U, /**< REFCTL28_LOW */ },
	},
	/* [1] CNOC_CFG */
	{
		.name	= "CNOC_CFG",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x800000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFFFFFFFU,
		.faultin_en0_high = 0xFFFFFFFFU,
		.faultin_en1_low  = 0xFFFFFFFFU,
		.faultin_en1_high = 0xFFFFFFFFU,
		.faultin_en2_low  = 0xFFFFFFFFU,
		.faultin_en2_high = 0x1FFFFU },
      { .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1U,
		.faultin_en0_high = 0x0U,
		.faultin_en1_low  = 0x0U,
		.faultin_en1_high = 0x0U,
		.faultin_en2_low  = 0x0U,
		.faultin_en2_high = 0x0U },
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x100U, /**< REFCTL0_LOW  3.4ms */
				  0x100U, /**< REFCTL1_LOW */
				  0x100U, /**< REFCTL2_LOW */
				  0x100U, /**< REFCTL3_LOW */
				  0x100U, /**< REFCTL4_LOW */
				  0x100U, /**< REFCTL5_LOW */
				  0x100U, /**< REFCTL6_LOW */
				  0x100U, /**< REFCTL7_LOW */
				  0x100U, /**< REFCTL8_LOW */
				  0x100U, /**< REFCTL9_LOW */
				  0x100U, /**< REFCTL10_LOW */
				  0x100U, /**< REFCTL11_LOW */
				  0x100U, /**< REFCTL12_LOW */
				  0x100U, /**< REFCTL13_LOW */
				  0x100U, /**< REFCTL14_LOW */
				  0x100U, /**< REFCTL15_LOW */
				  0x100U, /**< REFCTL16_LOW */
				  0x100U, /**< REFCTL17_LOW */
				  0x100U, /**< REFCTL18_LOW */
				  0x100U, /**< REFCTL19_LOW */
				  0x100U, /**< REFCTL20_LOW */
				  0x100U, /**< REFCTL21_LOW */
				  0x100U, /**< REFCTL22_LOW */
				  0x100U, /**< REFCTL23_LOW */
				  0x100U, /**< REFCTL24_LOW */
				  0x100U, /**< REFCTL25_LOW */
				  0x100U, /**< REFCTL26_LOW */
				  0x100U, /**< REFCTL27_LOW */
				  0x100U, /**< REFCTL28_LOW */
				  0x100U, /**< REFCTL29_LOW */
				  0x100U, /**< REFCTL30_LOW */
				  0x100U, /**< REFCTL31_LOW */
				  0x100U, /**< REFCTL32_LOW */
				  0x100U, /**< REFCTL33_LOW */
				  0x100U, /**< REFCTL34_LOW */
				  0x100U, /**< REFCTL35_LOW */
				  0x100U, /**< REFCTL36_LOW */
				  0x100U, /**< REFCTL37_LOW */
				  0x100U, /**< REFCTL38_LOW */
				  0x100U, /**< REFCTL39_LOW */
				  0x100U, /**< REFCTL40_LOW */
				  0x100U, /**< REFCTL41_LOW */
				  0x100U, /**< REFCTL42_LOW */
				  0x100U, /**< REFCTL43_LOW */
				  0x100U, /**< REFCTL44_LOW */
				  0x100U, /**< REFCTL45_LOW */
				  0x100U, /**< REFCTL46_LOW */
				  0x100U, /**< REFCTL47_LOW */
				  0x100U, /**< REFCTL48_LOW */
				  0x100U, /**< REFCTL49_LOW */
				  0x100U, /**< REFCTL50_LOW */
				  0x100U, /**< REFCTL51_LOW */
				  0x100U, /**< REFCTL52_LOW */
				  0x100U, /**< REFCTL53_LOW */
				  0x100U, /**< REFCTL54_LOW */
				  0x100U, /**< REFCTL55_LOW */
				  0x100U, /**< REFCTL56_LOW */
				  0x100U, /**< REFCTL57_LOW */
				  0x1U,   /**< MAINCTL_LOW */

				  0x100U, /**< REFGEN1_REFCTL0_LOW Qns_d2d_noc_tile ISSUE */
				  0x100U, /**< REFGEN1_REFCTL1_LOW Qns_d2d_noc_time_1 ISSUE */
				  0x100U, /**< REFGEN1_REFCTL2_LOW */
				  0x100U, /**< REFGEN1_REFCTL3_LOW */
				  0x100U, /**< REFGEN1_REFCTL4_LOW */
				  0x100U, /**< REFGEN1_REFCTL5_LOW */
				  0x100U, /**< REFGEN1_REFCTL6_LOW */
				  0x100U, /**< REFGEN1_REFCTL7_LOW */
				  0x100U, /**< REFGEN1_REFCTL8_LOW */
				  0x100U, /**< REFGEN1_REFCTL9_LOW */
				  0x100U, /**< REFGEN1_REFCTL10_LOW */
				  0x100U, /**< REFGEN1_REFCTL11_LOW */
				  0x100U, /**< REFGEN1_REFCTL12_LOW */
				  0x100U, /**< REFGEN1_REFCTL13_LOW */
				  0x100U, /**< REFGEN1_REFCTL14_LOW */
				  0x100U, /**< REFGEN1_REFCTL15_LOW */
				  0x100U, /**< REFGEN1_REFCTL16_LOW */
				  0x100U, /**< REFGEN1_REFCTL17_LOW */
				  0x100U, /**< REFGEN1_REFCTL18_LOW */
				  0x100U, /**< REFGEN1_REFCTL19_LOW */
				  0x100U, /**< REFGEN1_REFCTL20_LOW */
				  0x100U, /**< REFGEN1_REFCTL21_LOW */
				  0x100U, /**< REFGEN1_REFCTL22_LOW */
				  0x100U, /**< REFGEN1_REFCTL23_LOW */
				  0x100U, /**< REFGEN1_REFCTL24_LOW */
				  0x100U, /**< REFGEN1_REFCTL25_LOW */
				  0x100U, /**< REFGEN1_REFCTL26_LOW */
				  0x100U, /**< REFGEN1_REFCTL27_LOW */
				  0x100U, /**< REFGEN1_REFCTL28_LOW */
				  0x100U, /**< REFGEN1_REFCTL29_LOW */
				  0x100U, /**< REFGEN1_REFCTL30_LOW */
				  0x100U, /**< REFGEN1_REFCTL31_LOW */
				  0x100U, /**< REFGEN1_REFCTL32_LOW */
				  0x100U, /**< REFGEN1_REFCTL33_LOW */
				  0x100U, /**< REFGEN1_REFCTL34_LOW */
				  0x100U, /**< REFGEN1_REFCTL35_LOW */
				  0x1U,   /**< REFGEN1_MAINCTL_LOW */
				  0x10FU,  /**<TRCHECK_0_REFGEN_REFCTL0_LOW*/ /* Tr check clk reference remains same 19.2Mhz for all NOCs including cnoc CFG */
				  0x1U,   /**< TRCHECK_0_MAINCTL_LOW */
				  0x10FU,  /**<TRCHECK_1_REFGEN_REFCTL0_LOW*/
				  0x1U,   /**< TRCHECK_1_MAINCTL_LOW */
				  0x10FU, /**< REFCTL58_LOW */
				  0x10FU, /**< REFCTL59_LOW */
				  0x10FU, /**< REFCTL60_LOW */
				  0x10FU, /**< REFCTL61_LOW */},
	},
	/* [2] SLICE_0_DC_NOC_LOCAL */
	{
		.name	= "SLICE_0_DC_NOC_LOCAL",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x100U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFE5F1C7U,
		.faultin_en0_high = 0x1U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U,
		.faultin_en0_high = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFCTL0_LOW */
				  0x105U, /**< REFCTL1_LOW */
				  0x105U, /**< REFCTL2_LOW */
				  0x105U, /**< REFCTL3_LOW */
				  0x105U, /**< REFCTL4_LOW */
				  0x105U, /**< REFCTL5_LOW */
				  0x105U, /**< REFCTL6_LOW */
				  0x105U, /**< REFCTL7_LOW */
				  0x105U, /**< REFCTL8_LOW */
				  0x105U, /**< REFCTL9_LOW */
				  0x105U, /**< REFCTL10_LOW */
				  0x105U, /**< REFCTL11_LOW */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL0_LOW */
				  0x1U, /**< TRCHECK_0_MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0_LOW*/
				  0x1U, /**< TRCHECK_1_MAINCTL_LOW */ },
	},
	/* [3] SLICE_0_DC_NOC_MAIN */
	{
		.name	= "SLICE_0_DC_NOC_MAIN",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x20000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x37U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x31U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFCTL0_LOW */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL0_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL1_LOW */
				  0x1U, /**< TRCHECK_0_MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0_LOW*/
				  0x10FU, /**< TRCHECK_1_REFCTL1_LOW*/
				  0x1U, /**< TRCHECK_1_MAINCTL_LOW */},
	},
	/* [4] SLICE_1_DC_NOC_LOCAL */
	{
		.name	= "SLICE_1_DC_NOC_LOCAL",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x200U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFE5F1C7U,
		.faultin_en0_high = 0x1U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U,
		.faultin_en0_high = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFCTL0_LOW */
				  0x105U, /**< REFCTL1_LOW */
				  0x105U, /**< REFCTL2_LOW */
				  0x105U, /**< REFCTL3_LOW */
				  0x105U, /**< REFCTL4_LOW */
				  0x105U, /**< REFCTL5_LOW */
				  0x105U, /**< REFCTL6_LOW */
				  0x105U, /**< REFCTL7_LOW */
				  0x105U, /**< REFCTL8_LOW */
				  0x105U, /**< REFCTL9_LOW */
				  0x105U, /**< REFCTL10_LOW */
				  0x105U, /**< REFCTL11_LOW */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL0_LOW */
				  0x1U, /**< TRCHECK_0_MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0_LOW*/
				  0x1U, /**< TRCHECK_1_MAINCTL_LOW */ },
	},
	/* [5] SLICE_1_DC_NOC_MAIN */
	{
		.name	= "SLICE_1_DC_NOC_MAIN",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x80000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x37U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x31U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFCTL0_LOW */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL0_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL1_LOW */
				  0x1U, /**< TRCHECK_0_MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0_LOW*/
				  0x10FU, /**< TRCHECK_1_REFCTL1_LOW*/
				  0x1U, /**< TRCHECK_1_MAINCTL_LOW */},
	},
	/* [6] SLICE_2_DC_NOC_LOCAL */
	{
		.name	= "SLICE_2_DC_NOC_LOCAL",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x400U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFE5F1C7U,
		.faultin_en0_high = 0x1U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U,
		.faultin_en0_high = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFCTL0_LOW */
				  0x105U, /**< REFCTL1_LOW */
				  0x105U, /**< REFCTL2_LOW */
				  0x105U, /**< REFCTL3_LOW */
				  0x105U, /**< REFCTL4_LOW */
				  0x105U, /**< REFCTL5_LOW */
				  0x105U, /**< REFCTL6_LOW */
				  0x105U, /**< REFCTL7_LOW */
				  0x105U, /**< REFCTL8_LOW */
				  0x105U, /**< REFCTL9_LOW */
				  0x105U, /**< REFCTL10_LOW */
				  0x105U, /**< REFCTL11_LOW */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL0_LOW */
				  0x1U, /**< TRCHECK_0_MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0_LOW*/
				  0x1U, /**< TRCHECK_1_MAINCTL_LOW */ },
	},
	/* [7] SLICE_2_DC_NOC_MAIN */
	{
		.name	= "SLICE_2_DC_NOC_MAIN",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x1U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x37U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x31U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFCTL0_LOW */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL0_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL1_LOW */
				  0x1U, /**< TRCHECK_0_MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0_LOW*/
				  0x10FU, /**< TRCHECK_1_REFCTL1_LOW*/
				  0x1U, /**< TRCHECK_1_MAINCTL_LOW */},
	},
	/* [8] SLICE_3_DC_NOC_LOCAL */
	{
		.name	= "SLICE_3_DC_NOC_LOCAL",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x800U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low = 0xFFE5F1C7U,
		.faultin_en0_high = 0x1U},
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low = 0x7U,
		.faultin_en0_high = 0x0U},
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFCTL0_LOW */
				  0x105U, /**< REFCTL1_LOW */
				  0x105U, /**< REFCTL2_LOW */
				  0x105U, /**< REFCTL3_LOW */
				  0x105U, /**< REFCTL4_LOW */
				  0x105U, /**< REFCTL5_LOW */
				  0x105U, /**< REFCTL6_LOW */
				  0x105U, /**< REFCTL7_LOW */
				  0x105U, /**< REFCTL8_LOW */
				  0x105U, /**< REFCTL9_LOW */
				  0x105U, /**< REFCTL10_LOW */
				  0x105U, /**< REFCTL11_LOW */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL0_LOW */
				  0x1U, /**< TRCHECK_0_MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0_LOW*/
				  0x1U, /**< TRCHECK_1_MAINCTL_LOW */ },
	},
	/* [9] SLICE_3_DC_NOC_MAIN */
	{
		.name	= "SLICE_3_DC_NOC_MAIN",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x37U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x31U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFCTL0_LOW */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL0_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL1_LOW */
				  0x1U, /**< TRCHECK_0_MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0_LOW*/
				  0x10FU, /**< TRCHECK_1_REFCTL1_LOW*/
				  0x1U, /**< TRCHECK_1_MAINCTL_LOW */},
	},
	/* [10] HSCNOC_CFG_DDR0HN0 */
	{
		.name	= "HSCNOC_CFG_DDR0HN0",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ },
		.poc	= (struct nocerr_pos_info_oem []){
	{ .enable = true },  /**< HN0_POC */
      },
	},
	/* [11] HSCNOC_CFG_DDR0HN1 */
	{
		.name	= "HSCNOC_CFG_DDR0HN1",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ },
		.poc	= (struct nocerr_pos_info_oem []){
	{ .enable = true },  /**< HN1_POC */
      },
	},
	/* [12] HSCNOC_CFG_DDR1HN0 */
	{
		.name	= "HSCNOC_CFG_DDR1HN0",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ },
		.poc	= (struct nocerr_pos_info_oem []){
	{ .enable = true },  /**< HN0_POC */
      },
	},
	/* [13] HSCNOC_CFG_DDR1HN1 */
	{
		.name	= "HSCNOC_CFG_DDR1HN1",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ },
		.poc	= (struct nocerr_pos_info_oem []){
	{ .enable = true },  /**< HN1_POC */
      },
	},
	/* [14] HSCNOC_CFG_DDR2HN0 */
	{
		.name	= "HSCNOC_CFG_DDR2HN0",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ },
		.poc	= (struct nocerr_pos_info_oem []){
	{ .enable = true },  /**< HN0_POC */
      },
	},
	/* [15] HSCNOC_CFG_DDR2HN1 */
	{
		.name	= "HSCNOC_CFG_DDR2HN1",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ },
		.poc	= (struct nocerr_pos_info_oem []){
	{ .enable = true },  /**< HN1_POC */
      },
	},
	/* [16] HSCNOC_CFG_DDR3HN0 */
	{
		.name	= "HSCNOC_CFG_DDR3HN0",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ },
		.poc	= (struct nocerr_pos_info_oem []){
	{ .enable = true },  /**< HN0_POC */
      },
	},
	/* [17] HSCNOC_CFG_DDR3HN1 */
	{
		.name	= "HSCNOC_CFG_DDR3HN1",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ },
		.poc	= (struct nocerr_pos_info_oem []){
	{ .enable = true },  /**< HN1_POC */
      },
	},
	/* [18] AGGRE1_NOC */
	{
		.name	= "AGGRE1_NOC",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x200000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1U },
      },
		.to_reg_vals	= (uint32_t []){ },
	},
	/* [19] AGGRE1_NOC_TILE */
	{
		.name	= "AGGRE1_NOC_TILE",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x100000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x9U },
      },
		.to_reg_vals	= (uint32_t []){ },
	},
	/* [20] AGGRE2_NOC */
	{
		.name	= "AGGRE2_NOC",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x400000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7FU },
	{ .faultin_en0_low  = 0xFFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x67U },
	{ .faultin_en0_low  = 0x5FU },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFCTL0_LOW 1.7ms 1024U / (19.2 / 1U << 5U)*/
				  0x105U, /**< REFCTL1_LOW */
				  0x105U, /**< REFCTL2_LOW */
				  0x105U, /**< REFCTL3_LOW */
				  0x1U,   /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL0_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL1_LOW */
				  0x111U, /**< TRCHCK_0_REFCTL2_LOW */ /* QSTABILITY-20351046U, 23741730U - doubled qhm_qup2_I timeout 7U*(1U<<0x11U)/19.2e6 ~ 47ms */
				  0x10FU, /**< TRCHCK_0_REFCTL3_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL4_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL5_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL6_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL7_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL8_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL9_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL10_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL11_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL12_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL13_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL14_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL15_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL16_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL17_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL18_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL19_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL20_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL21_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL22_LOW */
				  0x10FU, /**< TRCHCK_0_REFCTL23_LOW */
				  0x1U,   /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL0_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL1_LOW */
				  0x111U, /**< TRCHCK_1_REFCTL2_LOW */ /* QSTABILITY-20351046U, 23741730U - doubled qhm_qup2_I timeout 7U*(1U<<0x11U)/19.2e6 ~ 47ms */
				  0x10FU, /**< TRCHCK_1_REFCTL3_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL4_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL5_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL6_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL7_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL8_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL9_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL10_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL11_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL12_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL13_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL14_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL15_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL16_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL17_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL18_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL19_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL20_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL21_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL22_LOW */
				  0x10FU, /**< TRCHCK_1_REFCTL23_LOW */
				  0x1U,   /**< MAINCTL_LOW */},
	},
	/* [21] AGGRE2_NOC_TILE */
	{
		.name	= "AGGRE2_NOC_TILE",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x10000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x9U },
      },
		.to_reg_vals	= (uint32_t []){ },
	},
	/* [22] SYSTEM_NOC */
	{
		.name	= "SYSTEM_NOC",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x40000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xC7U },
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xC5U },
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10FU, /**< TRCHECK_0_REFCTL0 */
				  0x10FU, /**< TRCHECK_0_REFCTL1 */
				  0x10FU, /**< TRCHECK_0_REFCTL2 */
				  0x10FU, /**< TRCHECK_0_REFCTL3 */
				  0x10FU, /**< TRCHECK_0_REFCTL4 */
				  0x10FU, /**< TRCHECK_0_REFCTL5 */
				  0x10FU, /**< TRCHECK_0_REFCTL6 */
				  0x1U,   /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0 */
				  0x10FU, /**< TRCHECK_1_REFCTL1 */
				  0x10FU, /**< TRCHECK_1_REFCTL2 */
				  0x10FU, /**< TRCHECK_1_REFCTL3 */
				  0x10FU, /**< TRCHECK_1_REFCTL4 */
				  0x10FU, /**< TRCHECK_1_REFCTL5 */
				  0x10FU, /**< TRCHECK_1_REFCTL6 */
				  0x1U,   /**< MAINCTL_LOW */ },
	},
	/* [23] MMSS_NOC */
	{
		.name	= "MMSS_NOC",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x80000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x3FFFFU },
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7FFU },
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGEN_REFCTL0 */
				  0x105U, /**< REFGEN_REFCTL1 */
				  0x105U, /**< REFGEN_REFCTL2 */
				  0x105U, /**< REFGEN_REFCTL3 */
				  0x105U, /**< REFGEN_REFCTL4 */
				  0x105U, /**< REFGEN_REFCTL5 */
				  0x105U, /**< REFGEN_REFCTL6 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL0 */
				  0x10FU, /**< TRCHECK_0_REFCTL1 */
				  0x10FU, /**< TRCHECK_0_REFCTL2 */
				  0x10FU, /**< TRCHECK_0_REFCTL3 */
				  0x10FU, /**< TRCHECK_0_REFCTL4 */
				  0x10FU, /**< TRCHECK_0_REFCTL5 */
				  0x10FU, /**< TRCHECK_0_REFCTL6 */
				  0x10FU, /**< TRCHECK_0_REFCTL7 */
				  0x10FU, /**< TRCHECK_0_REFCTL8 */
				  0x10FU, /**< TRCHECK_0_REFCTL9 */
				  0x10FU, /**< TRCHECK_0_REFCTL10 */
				  0x10FU, /**< TRCHECK_0_REFCTL11 */
				  0x10FU, /**< TRCHECK_0_REFCTL12 */
				  0x10FU, /**< TRCHECK_0_REFCTL13 */
				  0x10FU, /**< TRCHECK_0_REFCTL14 */
				  0x10FU, /**< TRCHECK_0_REFCTL15 */
				  0x10FU, /**< TRCHECK_0_REFCTL16 */
				  0x10FU, /**< TRCHECK_0_REFCTL17 */
				  0x10FU, /**< TRCHECK_0_REFCTL18 */
				  0x10FU, /**< TRCHECK_0_REFCTL19 */
				  0x10FU, /**< TRCHECK_0_REFCTL20 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0 */
				  0x10FU, /**< TRCHECK_1_REFCTL1 */
				  0x10FU, /**< TRCHECK_1_REFCTL2 */
				  0x10FU, /**< TRCHECK_1_REFCTL3 */
				  0x10FU, /**< TRCHECK_1_REFCTL4 */
				  0x10FU, /**< TRCHECK_1_REFCTL5 */
				  0x10FU, /**< TRCHECK_1_REFCTL6 */
				  0x10FU, /**< TRCHECK_1_REFCTL7 */
				  0x10FU, /**< TRCHECK_1_REFCTL8 */
				  0x10FU, /**< TRCHECK_1_REFCTL9 */
				  0x10FU, /**< TRCHECK_1_REFCTL10 */
				  0x10FU, /**< TRCHECK_1_REFCTL11 */
				  0x10FU, /**< TRCHECK_1_REFCTL12 */
				  0x10FU, /**< TRCHECK_1_REFCTL13 */
				  0x10FU, /**< TRCHECK_1_REFCTL14 */
				  0x10FU, /**< TRCHECK_1_REFCTL15 */
				  0x10FU, /**< TRCHECK_1_REFCTL16 */
				  0x10FU, /**< TRCHECK_1_REFCTL17 */
				  0x10FU, /**< TRCHECK_1_REFCTL18 */
				  0x10FU, /**< TRCHECK_1_REFCTL19 */
				  0x10FU, /**< TRCHECK_1_REFCTL20 */
				  0x1U, /**< MAINCTL_LOW */},
	},
	/* [24] HPASS_AG_NOC */
	{
		.name	= "HPASS_AG_NOC",
		.intr_enable	= true,
		.error_fatal	= false,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFFFFFFFU,
		.faultin_en0_high = 0x1U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x40000003U,
		.faultin_en0_high = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){
				  0x105U, /**< REFGEN_REFCTL0 */
				  0x105U, /**< REFGEN_REFCTL1 */
				  0x105U, /**< REFGEN_REFCTL2 */
				  0x105U, /**< REFGEN_REFCTL3 */
				  0x105U, /**< REFGEN_REFCTL4 */
				  0x105U, /**< REFGEN_REFCTL5 */
				  0x105U, /**< REFGEN_REFCTL6 */
				  0x105U, /**< REFGEN_REFCTL7 */
				  0x105U, /**< REFGEN_REFCTL8 */
				  0x105U, /**< REFGEN_REFCTL9 */
				  0x105U, /**< REFGEN_REFCTL10 */
				  0x105U, /**< REFGEN_REFCTL11 */
				  0x105U, /**< REFGEN_REFCTL12 */
				  0x105U, /**< REFGEN_REFCTL13 */
				  0x105U, /**< REFGEN_REFCTL14 */
				  0x105U, /**< REFGEN_REFCTL15 */
				  0x105U, /**< REFGEN_REFCTL16 */
				  0x105U, /**< REFGEN_REFCTL17 */
				  0x105U, /**< REFGEN_REFCTL18 */
				  0x105U, /**< REFGEN_REFCTL19 */
				  0x105U, /**< REFGEN_REFCTL20 */
				  0x105U, /**< REFGEN_REFCTL21 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL0 */
				  0x10FU, /**< TRCHECK_0_REFCTL1 */
				  0x10FU, /**< TRCHECK_0_REFCTL2 */
				  0x10FU, /**< TRCHECK_0_REFCTL3 */
				  0x10FU, /**< TRCHECK_0_REFCTL4 */
				  0x10FU, /**< TRCHECK_0_REFCTL5 */
				  0x10FU, /**< TRCHECK_0_REFCTL6 */
				  0x10FU, /**< TRCHECK_0_REFCTL7 */
				  0x10FU, /**< TRCHECK_0_REFCTL8 */
				  0x10FU, /**< TRCHECK_0_REFCTL9 */
				  0x10FU, /**< TRCHECK_0_REFCTL10 */
				  0x10FU, /**< TRCHECK_0_REFCTL11 */
				  0x10FU, /**< TRCHECK_0_REFCTL12 */
				  0x10FU, /**< TRCHECK_0_REFCTL13 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0 */
				  0x10FU, /**< TRCHECK_1_REFCTL1 */
				  0x10FU, /**< TRCHECK_1_REFCTL2 */
				  0x10FU, /**< TRCHECK_1_REFCTL3 */
				  0x10FU, /**< TRCHECK_1_REFCTL4 */
				  0x10FU, /**< TRCHECK_1_REFCTL5 */
				  0x10FU, /**< TRCHECK_1_REFCTL6 */
				  0x10FU, /**< TRCHECK_1_REFCTL7 */
				  0x10FU, /**< TRCHECK_1_REFCTL8 */
				  0x10FU, /**< TRCHECK_1_REFCTL9 */
				  0x10FU, /**< TRCHECK_1_REFCTL10 */
				  0x10FU, /**< TRCHECK_1_REFCTL11 */
				  0x10FU, /**< TRCHECK_1_REFCTL12 */
				  0x10FU, /**< TRCHECK_1_REFCTL13 */
				  0x1U, /**< MAINCTL_LOW */},
	},
	/* [25] PCIE_CFG */
	{
		.name	= "PCIE_CFG",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x4000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1FFFU },
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1U },
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x105U, /**< REFGENERATOR_REFCTL1 */
				  0x105U, /**< REFGENERATOR_REFCTL2 */
				  0x105U, /**< REFGENERATOR_REFCTL3 */
				  0x105U, /**< REFGENERATOR_REFCTL4 */
				  0x105U, /**< REFGENERATOR_REFCTL5 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_0_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_1_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */ },
	},
	/* [26] NSP_CFG_NOC_0 */
	{
		.name	= "NSP_CFG_NOC_0",
		.intr_enable	= false,
		.error_fatal	= false,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1FFC07U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x0U, /**< REFGEN_REFCTL0 */
				  0x0U, /**< REFGEN_REFCTL1 */
				  0x0U, /**< REFGEN_REFCTL2 */
				  0x0U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGEN_REFCTL0 */
				  0x10FU, /**< REFGEN_REFCTL1 */
				  0x0U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGEN_REFCTL0 */
				  0x10FU, /**< REFGEN_REFCTL1 */
				  0x0U, /**< MAINCTL_LOW */ },
	},
	/* [27] NSP_CFG_NOC_1 */
	{
		.name	= "NSP_CFG_NOC_1",
		.intr_enable	= false,
		.error_fatal	= false,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1FFC07U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x0U, /**< REFGEN_REFCTL0 */
				  0x0U, /**< REFGEN_REFCTL1 */
				  0x0U, /**< REFGEN_REFCTL2 */
				  0x0U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGEN_REFCTL0 */
				  0x10FU, /**< REFGEN_REFCTL1 */
				  0x0U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGEN_REFCTL0 */
				  0x10FU, /**< REFGEN_REFCTL1 */
				  0x0U, /**< MAINCTL_LOW */
    },
	},
	/* [28] NSP_CFG_NOC_2 */
	{
		.name	= "NSP_CFG_NOC_2",
		.intr_enable	= false,
		.error_fatal	= false,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1FFC07U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x0U, /**< REFGEN_REFCTL0 */
				  0x0U, /**< REFGEN_REFCTL1 */
				  0x0U, /**< REFGEN_REFCTL2 */
				  0x0U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGEN_REFCTL0 */
				  0x10FU, /**< REFGEN_REFCTL1 */
				  0x0U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGEN_REFCTL0 */
				  0x10FU, /**< REFGEN_REFCTL1 */
				  0x0U, /**< MAINCTL_LOW */
    },
	},
	/* [29] NSP_CFG_NOC_3 */
	{
		.name	= "NSP_CFG_NOC_3",
		.intr_enable	= false,
		.error_fatal	= false,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1FFC07U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x0U, /**< REFGEN_REFCTL0 */
				  0x0U, /**< REFGEN_REFCTL1 */
				  0x0U, /**< REFGEN_REFCTL2 */
				  0x0U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGEN_REFCTL0 */
				  0x10FU, /**< REFGEN_REFCTL1 */
				  0x0U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGEN_REFCTL0 */
				  0x10FU, /**< REFGEN_REFCTL1 */
				  0x0U, /**< MAINCTL_LOW */
    },
	},
	/* [30] NSP_DATA_NOC_0 */
	{
		.name	= "NSP_DATA_NOC_0",
		.intr_enable	= false,
		.error_fatal	= false,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFF000FFU,
		.faultin_en0_high = 0x1U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xC1U,
		.faultin_en0_high = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x105U, /**< REFGENERATOR_REFCTL1 */
				  0x105U, /**< REFGENERATOR_REFCTL2 */
				  0x105U, /**< REFGENERATOR_REFCTL3 */
				  0x105U, /**< REFGENERATOR_REFCTL4 */
				  0x105U, /**< REFGENERATOR_REFCTL5 */
				  0x105U, /**< REFGENERATOR_REFCTL6 */
				  0x105U, /**< REFGENERATOR_REFCTL7 */
				  0x105U, /**< REFGENERATOR_REFCTL8 */
				  0x105U, /**< REFGENERATOR_REFCTL9 */
				  0x105U, /**< REFGENERATOR_REFCTL10 */
				  0x105U, /**< REFGENERATOR_REFCTL11 */
				  0x105U, /**< REFGENERATOR_REFCTL12 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGENERATOR_0_REFCTL0 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL1 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL2 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL3 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL4 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL5 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL6 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL7 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL8 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL9 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL10 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGENERATOR_1_REFCTL0 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL1 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL2 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL3 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL4 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL5 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL6 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL7 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL8 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL9 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL10 */
				  0x1U, /**< MAINCTL_LOW */ },
	},
	/* [31] NSP_DATA_NOC_1 */
	{
		.name	= "NSP_DATA_NOC_1",
		.intr_enable	= false,
		.error_fatal	= false,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFF000FFU,
		.faultin_en0_high = 0x1U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xC1U,
		.faultin_en0_high = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x105U, /**< REFGENERATOR_REFCTL1 */
				  0x105U, /**< REFGENERATOR_REFCTL2 */
				  0x105U, /**< REFGENERATOR_REFCTL3 */
				  0x105U, /**< REFGENERATOR_REFCTL4 */
				  0x105U, /**< REFGENERATOR_REFCTL5 */
				  0x105U, /**< REFGENERATOR_REFCTL6 */
				  0x105U, /**< REFGENERATOR_REFCTL7 */
				  0x105U, /**< REFGENERATOR_REFCTL8 */
				  0x105U, /**< REFGENERATOR_REFCTL9 */
				  0x105U, /**< REFGENERATOR_REFCTL10 */
				  0x105U, /**< REFGENERATOR_REFCTL11 */
				  0x105U, /**< REFGENERATOR_REFCTL12 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGENERATOR_0_REFCTL0 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL1 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL2 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL3 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL4 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL5 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL6 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL7 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL8 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL9 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL10 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGENERATOR_1_REFCTL0 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL1 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL2 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL3 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL4 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL5 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL6 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL7 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL8 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL9 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL10 */
				  0x1U, /**< MAINCTL_LOW */ },
	},
	/* [32] NSP_DATA_NOC_2 */
	{
		.name	= "NSP_DATA_NOC_2",
		.intr_enable	= false,
		.error_fatal	= false,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFF000FFU,
		.faultin_en0_high = 0x1U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xC1U,
		.faultin_en0_high = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x105U, /**< REFGENERATOR_REFCTL1 */
				  0x105U, /**< REFGENERATOR_REFCTL2 */
				  0x105U, /**< REFGENERATOR_REFCTL3 */
				  0x105U, /**< REFGENERATOR_REFCTL4 */
				  0x105U, /**< REFGENERATOR_REFCTL5 */
				  0x105U, /**< REFGENERATOR_REFCTL6 */
				  0x105U, /**< REFGENERATOR_REFCTL7 */
				  0x105U, /**< REFGENERATOR_REFCTL8 */
				  0x105U, /**< REFGENERATOR_REFCTL9 */
				  0x105U, /**< REFGENERATOR_REFCTL10 */
				  0x105U, /**< REFGENERATOR_REFCTL11 */
				  0x105U, /**< REFGENERATOR_REFCTL12 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGENERATOR_0_REFCTL0 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL1 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL2 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL3 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL4 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL5 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL6 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL7 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL8 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL9 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL10 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGENERATOR_1_REFCTL0 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL1 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL2 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL3 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL4 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL5 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL6 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL7 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL8 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL9 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL10 */
				  0x1U, /**< MAINCTL_LOW */ },
	},
	/* [33] NSP_DATA_NOC_3 */
	{
		.name	= "NSP_DATA_NOC_3",
		.intr_enable	= false,
		.error_fatal	= false,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFF000FFU,
		.faultin_en0_high = 0x1U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xC1U,
		.faultin_en0_high = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x105U, /**< REFGENERATOR_REFCTL1 */
				  0x105U, /**< REFGENERATOR_REFCTL2 */
				  0x105U, /**< REFGENERATOR_REFCTL3 */
				  0x105U, /**< REFGENERATOR_REFCTL4 */
				  0x105U, /**< REFGENERATOR_REFCTL5 */
				  0x105U, /**< REFGENERATOR_REFCTL6 */
				  0x105U, /**< REFGENERATOR_REFCTL7 */
				  0x105U, /**< REFGENERATOR_REFCTL8 */
				  0x105U, /**< REFGENERATOR_REFCTL9 */
				  0x105U, /**< REFGENERATOR_REFCTL10 */
				  0x105U, /**< REFGENERATOR_REFCTL11 */
				  0x105U, /**< REFGENERATOR_REFCTL12 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGENERATOR_0_REFCTL0 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL1 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL2 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL3 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL4 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL5 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL6 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL7 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL8 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL9 */
				  0x10FU, /**< REFGENERATOR_0_REFCTL10 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGENERATOR_1_REFCTL0 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL1 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL2 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL3 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL4 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL5 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL6 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL7 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL8 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL9 */
				  0x10FU, /**< REFGENERATOR_1_REFCTL10 */
				  0x1U, /**< MAINCTL_LOW */ },
	},
	/* [34] APSS_NOC */
	{
		.name	= "APSS_NOC",
		.intr_enable	= true,
		.error_fatal	= true,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x15U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x15U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10FU, /**< FUSA_REFGEN_REFCTL0 */
				  0x10FU, /**< FUSA_REFGEN_REFCTL1 */
				  0x10FU, /**< FUSA_REFGEN_REFCTL2 */
				  0x10FU, /**< FUSA_REFGEN_REFCTL3 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL5 */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL6 */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL7 */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL8 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [35] NSINOC */
	{
		.name	= "NSINOC",
		.intr_enable	= true,
		.error_fatal	= true,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1FF5U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1005U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x105U, /**< REFGENERATOR_REFCTL1 */
				  0x105U, /**< REFGENERATOR_REFCTL2 */
				  0x105U, /**< REFGENERATOR_REFCTL3 */
				  0x105U, /**< REFGENERATOR_REFCTL4 */
				  0x105U, /**< REFGENERATOR_REFCTL5 */
				  0x105U, /**< REFGENERATOR_REFCTL6 */
				  0x105U, /**< REFGENERATOR_REFCTL7 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< FUSA_REFGEN_REFCTL0 */
				  0x10FU, /**< FUSA_REFGEN_REFCTL1 */
				  0x10FU, /**< FUSA_REFGEN_REFCTL2 */
				  0x10FU, /**< FUSA_REFGEN_REFCTL3 */
				  0x10FU, /**< FUSA_REFGEN_REFCTL4 */
				  0x10FU, /**< FUSA_REFGEN_REFCTL5 */
				  0x10FU, /**< FUSA_REFGEN_REFCTL6 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL0 */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL1 */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL2 */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL3 */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL4 */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL5 */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL6 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [36] CPUSS_NOC */
	{
		.name	= "CPUSS_NOC",
		.intr_enable	= true,
		.error_fatal	= true,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x15U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x15U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10FU, /**< FUSA_REFGEN_REFCTL0 */
				  0x10FU, /**< FUSA_REFGEN_REFCTL1 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL0 */
				  0x10FU, /**< FUSA1_REFGEN_REFCTL1 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [37] COMPUTE_NOC_0 */
	{
		.name	= "COMPUTE_NOC_0",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x8U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL0 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL1 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL2 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL3 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL4 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL5 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL0 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL1 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL2 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL3 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL4 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL5 */
				  0x1U, /**< MAINCTL_LOW */

    },
	},
	/* [38] COMPUTE_NOC_1 */
	{
		.name	= "COMPUTE_NOC_1",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x10U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL0 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL1 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL2 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL3 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL4 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL5 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL0 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL1 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL2 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL3 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL4 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL5 */
				  0x1U, /**< MAINCTL_LOW */

    },
	},
	/* [39] COMPUTE_NOC_2 */
	{
		.name	= "COMPUTE_NOC_2",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x20U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL0 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL1 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL2 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL3 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL4 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL5 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL0 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL1 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL2 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL3 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL4 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL5 */
				  0x1U, /**< MAINCTL_LOW */

    },
	},
	/* [40] COMPUTE_NOC_3 */
	{
		.name	= "COMPUTE_NOC_3",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL0 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL1 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL2 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL3 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL4 */
				  0x10FU, /**< NOC_0_REFGENERATOR_0_REFCTL5 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL0 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL1 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL2 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL3 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL4 */
				  0x10FU, /**< NOC_0_REFGENERATOR_1_REFCTL5 */
				  0x1U, /**< MAINCTL_LOW */

    },
	},
	/* [41] COMPUTE_NOC_CFG */
	{
		.name	= "COMPUTE_NOC_CFG",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x8000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10FU, /**< CFG_NOC_REFGENERATOR_0_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< CFG_NOC_REFGENERATOR_1_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [42] HPASS_ADAS */
	{
		.name	= "HPASS_ADAS",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x200U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x19U },
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x19U },
	{ .faultin_en0_low  = 0x3U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL1 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL2 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL1 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL2 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [43] HPASS_AUDIO */
	{
		.name	= "HPASS_AUDIO",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x400U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x19U },
	{ .faultin_en0_low  = 0x3U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x19U },
	{ .faultin_en0_low  = 0x1U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL1 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL1 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [44] NSP_00 */
	{
		.name	= "NSP_00",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x800U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x19U },
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x19U },
	{ .faultin_en0_low  = 0x3U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL1 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL2 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL1 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL2 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [45] NSP_01 */
	{
		.name	= "NSP_01",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x1000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x19U },
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x19U },
	{ .faultin_en0_low  = 0x3U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL1 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL2 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL1 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL2 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [46] NSP_02 */
	{
		.name	= "NSP_02",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x2000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x19U },
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x19U },
	{ .faultin_en0_low  = 0x3U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL1 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL2 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL1 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL2 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [47] NSP_03 */
	{
		.name	= "NSP_03",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x4000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x19U },
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x19U },
	{ .faultin_en0_low  = 0x3U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL1 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL2 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL1 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL2 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [48] SAFEDMA */
	{
		.name	= "SAFEDMA",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x10000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1AU },
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1AU },
	{ .faultin_en0_low  = 0x3U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL1 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL1 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [49] SAIL */
	{
		.name	= "SAIL",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x20000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1AU },
	{ .faultin_en0_low  = 0xFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1AU },
	{ .faultin_en0_low  = 0x3U },
      },
		.to_reg_vals	= (uint32_t []){ 0x105U, /**< REFGENERATOR_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL1 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL1 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [50] CNOC_ADDR_TRANS */
	{
		.name	= "CNOC_ADDR_TRANS",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x2000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL1 */
				  0x10FU, /**< TR_CHECK_0_REFGENERATOR_REFCTL2 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL0 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL1 */
				  0x10FU, /**< TR_CHECK_1_REFGENERATOR_REFCTL2 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [51] PCIE_DATA_OUTBOUND */
	{
		.name	= "PCIE_DATA_OUTBOUND",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x1U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFU },
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1U },
	{ .faultin_en0_low  = 0x7U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10BU, /**< REFGENERATOR_REFCTL0 xs_pcie_0 - 109U ms to support endpoint timeouts */
				  0x10BU, /**< REFGENERATOR_REFCTL1 xs_pcie_1 */
				  0x10BU, /**< REFGENERATOR_REFCTL2 xs_pcie_2 */
				  0x10BU, /**< REFGENERATOR_REFCTL3 xs_pcie_3 */
				  0x105U, /**< REFGENERATOR_REFCTL4 qxs_pcie_dma_0 */
				  0x105U, /**< REFGENERATOR_REFCTL5 qxs_pcie_dma_1 */
				  0x105U, /**< REFGENERATOR_REFCTL6 qxs_pcie_dma_2 */
				  0x1U, /**< MAINCTL_LOW */
				  0x113U, /**< SFTY_EXTREF_0_REFGEN_REFCTL0 qnm_hscnoc_pcie_I */
				  0x10FU, /**< SFTY_EXTREF_0_REFGEN_REFCTL1 qsm_data_outbound_cfg_I */
				  0x113U, /**< SFTY_EXTREF_0_REFGEN_REFCTL2 qnm_pcie_ibnoc_dma_I */
				  0x10FU, /**< SFTY_EXTREF_0_REFGEN_REFCTL3 qnm_cnoc_pcie_dma_I */
				  0x1U, /**< MAINCTL_LOW */
				  0x113U, /**< SFTY_EXTREF_1_REFGEN_REFCTL0 qnm_hscnoc_pcie_I */
				  0x10FU, /**< SFTY_EXTREF_1_REFGEN_REFCTL1 qsm_data_outbound_cfg_I */
				  0x113U, /**< SFTY_EXTREF_1_REFGEN_REFCTL2 qnm_pcie_ibnoc_dma_I */
				  0x10FU, /**< SFTY_EXTREF_1_REFGEN_REFCTL3 qnm_cnoc_pcie_dma_I */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [52] PCIE_DATA_INBOUND */
	{
		.name	= "PCIE_DATA_INBOUND",
		.intr_enable	= true,
		.error_fatal	= false,
		.summary_intr_enable_bit_set	= 0x4U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7BFFU },
	{ .faultin_en0_low  = 0x7U },
	{ .faultin_en0_low  = 0x1FU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1EFU },
	{ .faultin_en0_low  = 0x7U },
	{ .faultin_en0_low  = 0x10U },
      },
		.to_reg_vals	= (uint32_t []){ 0x0U, /**< REFGENERATOR_REFCTL0 col_data_inbound - disabled due to col_data_inbound timeout */
				  0x1U, /**< MAINCTL_LOW */
				  0x105U, /**< QTB_REFGEN_REFCTL0 */
				  0x105U, /**< QTB_REFGEN_REFCTL1 */
				  0x105U, /**< QTB_REFGEN_REFCTL2 */
				  0x105U, /**< QTB_REFGEN_REFCTL3 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< SFTY_EXTREF_0_REFGEN_REFCTL0 qsm_data_inbound_cfg_I */
				  0x10FU, /**< SFTY_EXTREF_0_REFGEN_REFCTL1 xm_pcie_0_I */
				  0x10FU, /**< SFTY_EXTREF_0_REFGEN_REFCTL2 xm_pcie_1_I */
				  0x10FU, /**< SFTY_EXTREF_0_REFGEN_REFCTL3 xm_pcie_2_I */
				  0x10FU, /**< SFTY_EXTREF_0_REFGEN_REFCTL4 xm_pcie_3_I */
				  0x113U, /**< SFTY_EXTREF_0_REFGEN_REFCTL5 qxm_pcie_dma_1_I */
				  0x113U, /**< SFTY_EXTREF_0_REFGEN_REFCTL6 qxm_pcie_dma_0_I */
				  0x113U, /**< SFTY_EXTREF_0_REFGEN_REFCTL7 qxm_pcie_dma_2_I */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< SFTY_EXTREF_1_REFGEN_REFCTL0 qsm_data_inbound_cfg_I */
				  0x10FU, /**< SFTY_EXTREF_1_REFGEN_REFCTL1 xm_pcie_0_I */
				  0x10FU, /**< SFTY_EXTREF_1_REFGEN_REFCTL2 xm_pcie_1_I */
				  0x10FU, /**< SFTY_EXTREF_1_REFGEN_REFCTL3 xm_pcie_2_I */
				  0x10FU, /**< SFTY_EXTREF_1_REFGEN_REFCTL4 xm_pcie_3_I */
				  0x113U, /**< SFTY_EXTREF_1_REFGEN_REFCTL5 qxm_pcie_dma_1_I */
				  0x113U, /**< SFTY_EXTREF_1_REFGEN_REFCTL6 qxm_pcie_dma_0_I */
				  0x113U, /**< SFTY_EXTREF_1_REFGEN_REFCTL7 qxm_pcie_dma_2_I */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [53] HSCNOC_ATNOC_CFG */
	{
		.name	= "HSCNOC_ATNOC_CFG",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x17U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x16U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10FU, /**< REFGENERATOR_REFCTL0 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_REFRENCE_GENERATOR_REFCTL0 */
				  0x10FU, /**< TRCHECK_REFRENCE_GENERATOR_REFCTL1 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< TRCHECK_REFRENCE_GENERATOR1_REFCTL0 */
				  0x10FU, /**< TRCHECK_REFRENCE_GENERATOR1_REFCTL1 */
				  0x1U, /**< MAINCTL_LOW */
    },
	},
	/* [54] HSCNOC_CFG_CENTER */
	{
		.name	= "HSCNOC_CFG_CENTER",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x3FU },
	{ .faultin_en0_low  = 0x7FFU },
	{ .faultin_en0_low  = 0xFU },
	{ .faultin_en0_low  = 0x1FFFFFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
	{ .faultin_en0_low  = 0x3C0U },
	{ .faultin_en0_low  = 0xEU },
	{ .faultin_en0_low  = 0x1FFFF4U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10FU, /**< REFGENERATOR_REFCTL0 13ms */
				  0x10FU, /**< REFGENERATOR_REFCTL1 */
				  0x10FU, /**< REFGENERATOR_REFCTL2 */
				  0x10FU, /**< REFGENERATOR_REFCTL3 */
				  0x10FU, /**< REFGENERATOR_REFCTL4 */
				  0x10FU, /**< REFGENERATOR_REFCTL5 */
				  0x10FU, /**< REFGENERATOR_REFCTL6 */
				  0x110U, /**< REFGENERATOR_REFCTL7 gpu_tcu 26ms */
				  0x110U, /**< REFGENERATOR_REFCTL8 gpu_tcu 26ms */
				  0x110U, /**< REFGENERATOR_REFCTL9 alm_qtc_I 26ms */
				  0x110U, /**< REFGENERATOR_REFCTL10 alm_sys_tcu0_I 26ms */
				  0x110U, /**< REFGENERATOR_REFCTL11 alm_sys_tcu1_I  26ms */
				  0x110U, /**< REFGENERATOR_REFCTL12 alm_sys_tcu2_I  26ms */
				  0x110U, /**< REFGENERATOR_REFCTL13 chm_appsA_chm_appsA0  26ms*/
				  0x110U, /**< REFGENERATOR_REFCTL14 chm_appsA_chm_appsA1  */
				  0x110U, /**< REFGENERATOR_REFCTL15 chm_appsB_chm_appsB0  */
				  0x110U, /**< REFGENERATOR_REFCTL16 chm_appsB_chm_appsB1  */
				  0x110U, /**< REFGENERATOR_REFCTL17 chm_appsC_chm_appsC0  */
				  0x110U, /**< REFGENERATOR_REFCTL18 chm_appsC_chm_appsC1  */
				  0x10FU, /**< REFGENERATOR_REFCTL19 13ms qns_cnoc */
				  0x113U, /**< REFGENERATOR_REFCTL20 218ms qns_pcie  */
				  0x10FU, /**< REFGENERATOR_REFCTL21 13ms qns_ddr0_HN0 */
				  0x10FU, /**< REFGENERATOR_REFCTL22 qns_ddr0_HN1  */
				  0x1U, /**< MAINCTL_LOW */
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL0 26ms */
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL1 */
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL2 */
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL3 */
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL4 */
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL5 */
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL6 */
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL7 */
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL8 */
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL9 */
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL10*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL11*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL12*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL13*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL14*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL15*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL16*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL17*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL18*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL19*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL20*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL21*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL22*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL23*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL24*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL25*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL26*/
				  0x1U, /**< MAINCTL_LOW */
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL27*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL28*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL29*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL30*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL31*/
				  0x110U, /**< TRCHECK_REFGENERATOR_REFCTL32*/
				  0x10FU, /**< REFGENERATOR_REFCTL23 qns_ddr1_HN0  */
				  0x10FU, /**< REFGENERATOR_REFCTL24 qns_ddr1_HN1  */
				  0x10FU, /**< REFGENERATOR_REFCTL25 qns_ddr2_HN0 */
				  0x10FU, /**< REFGENERATOR_REFCTL26 qns_ddr2_HN1 */
				  0x10FU, /**< REFGENERATOR_REFCTL27 D2D*/
				  0x10FU, /**< REFGENERATOR_REFCTL28 */
				  0x10FU, /**< REFGENERATOR_REFCTL29 qns_ddr3_HN0  */
				  0x10FU, /**< REFGENERATOR_REFCTL30 qns_ddr3_HN1 */
    },
	},
	/* [55] HSCNOC_CFG_CENTER1 */
	{
		.name	= "HSCNOC_CFG_CENTER1",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x25FFFFU },
	{ .faultin_en0_low  = 0xFFFFFFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x20FFFEU },
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ 0x10FU, /**< REFGENERATOR1_REFCTL0 */
				  0x10FU, /**< REFGENERATOR1_REFCTL1 */
				  0x10FU, /**< REFGENERATOR1_REFCTL2 */
				  0x10FU, /**< REFGENERATOR1_REFCTL3 */
				  0x10FU, /**< REFGENERATOR1_REFCTL4 */
				  0x10FU, /**< REFGENERATOR1_REFCTL5 */
				  0x10FU, /**< REFGENERATOR1_REFCTL6 */
				  0x10FU, /**< REFGENERATOR1_REFCTL7 */
				  0x10FU, /**< REFGENERATOR1_REFCTL8 */
				  0x10FU, /**< REFGENERATOR1_REFCTL9 */
				  0x10FU, /**< REFGENERATOR1_REFCTL10 */
				  0x10FU, /**< REFGENERATOR1_REFCTL11 */
				  0x10FU, /**< REFGENERATOR1_REFCTL12 */
				  0x10FU, /**< REFGENERATOR1_REFCTL13 */
				  0x10FU, /**< REFGENERATOR1_REFCTL14 */
				  0x10FU, /**< REFGENERATOR1_REFCTL15 */
				  0x10FU, /**< REFGENERATOR1_REFCTL16 */
				  0x10FU, /**< REFGENERATOR1_REFCTL17 */
				  0x10FU, /**< REFGENERATOR1_REFCTL18 */
				  0x10FU, /**< REFGENERATOR1_REFCTL19 */
				  0x10FU, /**< REFGENERATOR1_REFCTL20 */
				  0x10FU, /**< REFGENERATOR1_REFCTL21 */
				  0x10FU, /**< REFGENERATOR1_REFCTL22 */
				  0x10FU, /**< REFGENERATOR1_REFCTL23 */
				  0x10FU, /**< REFGENERATOR1_REFCTL24 */
				  0x10FU, /**< REFGENERATOR1_REFCTL25 */
				  0x10FU, /**< REFGENERATOR1_REFCTL26 */
				  0x1U, /**< MAINCTL_LOW */
				  0x10FU, /**< REFGENERATOR1_REFCTL27 */
				  0x10FU, /**< REFGENERATOR1_REFCTL28 */
				  0x10FU, /**< REFGENERATOR1_REFCTL29 */
				  0x10FU, /**< REFGENERATOR1_REFCTL30 */
				  0x10FU, /**< REFGENERATOR1_REFCTL31 */
				  0x10FU, /**< REFGENERATOR1_REFCTL32 */
    },
	},
	/* [56] HSCNOC_CFG_CENTER2 */
	{
		.name	= "HSCNOC_CFG_CENTER2",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x3FFU },
	{ .faultin_en0_low  = 0xFU },
	{ .faultin_en0_low  = 0xFU },
	{ .faultin_en0_low  = 0x3FFFFFFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x20FU },
	{ .faultin_en0_low  = 0x0U },
	{ .faultin_en0_low  = 0x0U },
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ },
	},
	/* [57] HSCNOC_CFG_EAST1 */
	{
		.name	= "HSCNOC_CFG_EAST1",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x1FU },
	{ .faultin_en0_low  = 0x7FU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
	{ .faultin_en0_low  = 0x40U },
      },
		.to_reg_vals	= (uint32_t []){ },
		.poc	= (struct nocerr_pos_info_oem []){
	{ .enable = true },  /**< PCIE_POC */
      },
	},
	/* [58] HSCNOC_CFG_EAST */
	{
		.name	= "HSCNOC_CFG_EAST",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7FU },
	{ .faultin_en0_low  = 0x7U },
	{ .faultin_en0_low  = 0x3FU },
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x40U },
	{ .faultin_en0_low  = 0x2U },
	{ .faultin_en0_low  = 0x30U },
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ },
		.poc	= (struct nocerr_pos_info_oem []){
	{ .enable = true },  /**< CNOC_POC */

      },
	},
	/* [59] HSCNOC_CFG_WEST */
	{
		.name	= "HSCNOC_CFG_WEST",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFU },
	{ .faultin_en0_low  = 0x1FU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ },
	},
	/* [60] SOCCP_TRACE */
	{
		.name	= "SOCCP_TRACE",
		.intr_enable	= true,
		.error_fatal	= true,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7U },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){
	0x105U, /* refgen_RefCtl0 */
	0x105U, /* refgen_RefCtl1 */
	0x105U, /* refgen_RefCtl2 */
      0x1U,
      },
	},
	/* [61] AG_TRACE_NOC */
	{
		.name	= "AG_TRACE_NOC",
		.intr_enable	= true,
		.error_fatal	= true,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0xFFFFFFFFU,
		.faultin_en0_high = 0xFFFFFFFFU,
		.faultin_en1_low  = 0xFFFFFFFFU,
		.faultin_en1_high = 0x3FFFFU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U,
		.faultin_en0_high = 0x0U,
		.faultin_en1_low  = 0x0U,
		.faultin_en1_high = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){
	0x105U, /* refgen_RefCtl0 */
	0x105U, /* refgen_RefCtl1 */
	0x105U, /* refgen_RefCtl2 */
	0x105U, /* refgen_RefCtl3 */
	0x105U, /* refgen_RefCtl4 */
	0x105U, /* refgen_RefCtl5 */
	0x105U, /* refgen_RefCtl6 */
	0x105U, /* refgen_RefCtl7 */
	0x105U, /* refgen_RefCtl8 */
	0x105U, /* refgen_RefCtl9 */
	0x105U, /* refgen_RefCtl10 */
	0x105U, /* refgen_RefCtl11 */
	0x105U, /* refgen_RefCtl12 */
	0x105U, /* refgen_RefCtl13 */
	0x105U, /* refgen_RefCtl14 */
	0x105U, /* refgen_RefCtl15 */
	0x105U, /* refgen_RefCtl16 */
	0x105U, /* refgen_RefCtl17 */
	0x105U, /* refgen_RefCtl18 */
	0x105U, /* refgen_RefCtl19 */
	0x105U, /* refgen_RefCtl20 */
	0x105U, /* refgen_RefCtl21 */
	0x105U, /* refgen_RefCtl22 */
	0x105U, /* refgen_RefCtl23 */
	0x105U, /* refgen_RefCtl24 */
	0x105U, /* refgen_RefCtl25 */
	0x105U, /* refgen_RefCtl26 */
	0x105U, /* refgen_RefCtl27 */
	0x105U, /* refgen_RefCtl28 */
	0x105U, /* refgen_RefCtl29 */
	0x105U, /* refgen_RefCtl30 */
	0x105U, /* refgen_RefCtl31 */
	0x105U, /* refgen_RefCtl32 */
	0x105U, /* refgen_RefCtl33 */
	0x105U, /* refgen_RefCtl34 */
	0x105U, /* refgen_RefCtl35 */
	0x105U, /* refgen_RefCtl36 */
	0x105U, /* refgen_RefCtl37 */
	0x105U, /* refgen_RefCtl38 */
	0x105U, /* refgen_RefCtl39 */
	0x105U, /* refgen_RefCtl40 */
	0x105U, /* refgen_RefCtl41 */
	0x105U, /* refgen_RefCtl42 */
	0x105U, /* refgen_RefCtl43 */
	0x105U, /* refgen_RefCtl44 */
	0x105U, /* refgen_RefCtl45 */
	0x105U, /* refgen_RefCtl46 */
	0x105U, /* refgen_RefCtl47 */
	0x105U, /* refgen_RefCtl48 */
	0x105U, /* refgen_RefCtl49 */
	0x105U, /* refgen_RefCtl50 */
	0x105U, /* refgen_RefCtl51 */
	0x105U, /* refgen_RefCtl52 */
	0x105U, /* refgen_RefCtl53 */
	0x105U, /* refgen_RefCtl54 */
	0x105U, /* refgen_RefCtl55 */
	0x105U, /* refgen_RefCtl56 */
	0x105U, /* refgen_RefCtl57 */
	0x105U, /* refgen_RefCtl58 */
	0x105U, /* refgen_RefCtl59 */
	0x105U, /* refgen_RefCtl60 */
	0x105U, /* refgen_RefCtl61 */
	0x105U, /* refgen_RefCtl62 */
	0x105U, /* refgen_RefCtl63 */
	0x1U, /* Refgen_MainCtl */
	0x105U, /* refgen_RefCtl0 */
	0x105U, /* refgen_RefCtl1 */
	0x105U, /* refgen_RefCtl2 */
	0x105U, /* refgen_RefCtl3 */
	0x105U, /* refgen_RefCtl4 */
	0x105U, /* refgen_RefCtl5 */
	0x105U, /* refgen_RefCtl6 */
	0x105U, /* refgen_RefCtl7 */
	0x105U, /* refgen_RefCtl8 */
	0x105U, /* refgen_RefCtl9 */
	0x105U, /* refgen_RefCtl10 */
	0x105U, /* refgen_RefCtl11 */
	0x105U, /* refgen_RefCtl12 */
	0x105U, /* refgen_RefCtl13 */
	0x105U, /* refgen_RefCtl14 */
	0x105U, /* refgen_RefCtl15 */
	0x105U, /* refgen_RefCtl16 */
	0x105U, /* refgen_RefCtl17 */
	0x105U, /* refgen_RefCtl18 */
	0x105U, /* refgen_RefCtl19 */
	0x105U, /* refgen_RefCtl20 */
	0x105U, /* refgen_RefCtl21 */
	0x105U, /* refgen_RefCtl22 */
	0x105U, /* refgen_RefCtl23 */
	0x105U, /* refgen_RefCtl24 */
	0x105U, /* refgen_RefCtl25 */
	0x105U, /* refgen_RefCtl26 */
	0x105U, /* refgen_RefCtl27 */
	0x105U, /* refgen_RefCtl28 */
	0x105U, /* refgen_RefCtl29 */
	0x105U, /* refgen_RefCtl30 */
	0x105U, /* refgen_RefCtl31 */
	0x105U, /* refgen_RefCtl32 */
	0x105U, /* refgen_RefCtl33 */
	0x105U, /* refgen_RefCtl34 */
	0x105U, /* refgen_RefCtl35 */
	0x105U, /* refgen_RefCtl36 */
	0x105U, /* refgen_RefCtl37 */
	0x105U, /* refgen_RefCtl38 */
	0x105U, /* refgen_RefCtl39 */
	0x105U, /* refgen_RefCtl40 */
	0x105U, /* refgen_RefCtl41 */
	0x105U, /* refgen_RefCtl42 */
	0x105U, /* refgen_RefCtl43 */
	0x105U, /* refgen_RefCtl44 */
	0x1U, /* Refgen_MainCtl */
	0x105U, /* refgen_RefCtl45 */
	0x105U, /* refgen_RefCtl46 */
	0x105U, /* refgen_RefCtl47 */
	0x105U, /* refgen_RefCtl48 */
      0x105U, /* refgen_RefCtl49 */
      },
	},
	/* [62] HSCNOC_CFG_CENTER3 */
	{
		.name	= "HSCNOC_CFG_CENTER3",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
	},
	/* [63] HSCNOC_CFG_EAST2 */
	{
		.name	= "HSCNOC_CFG_EAST2",
		.intr_enable	= true,
		.error_fatal	= true,
		.summary_intr_enable_bit_set	= 0x40000000U,
		.sbms	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x7FU },
      },
		.obs_mask	= (struct nocerr_sbm_info_oem []){
	{ .faultin_en0_low  = 0x0U },
      },
		.to_reg_vals	= (uint32_t []){ },
		.poc	= (struct nocerr_pos_info_oem []){
	{ .enable = true },  /**< PCIE_POC */
      },
	},
};


static uint32_t clock_reg_vals[] = {

  0x1U,  /* Enable timeout reference clock */
  0x3U,  /* Set first divider to DIV4, so that we hit 4.8 MHz.
	       * Used by QoS reference and initial step of timeout reference. */
  0xFU, /* Second divider to DIV16, so our timeout reference is 300U KHz.
		 With a timeout counter of 1024U (default with refctl), this
		 means we have a timeout of 3.41 ms base. (1024U)/(19.2MHz/4U/16U)=3.41 ms */
  0x1U,
  0x3U,
  0xFU,

  0x1U,
  0x3U,
  0xFU,

  0x1U,
  0x3U,
  0xFU,
};

static struct nocerr_filter_oem filters_oem[] = {
	{ .enable = true, .delay_fatal = true }, /* [0] */
	{ .enable = true, .delay_fatal = false }, /* [1] */
	{ .enable = true, .delay_fatal = false }, /* [2] */
	{ .enable = true, .delay_fatal = false }, /* [3] */
	{ .enable = true, .delay_fatal = false }, /* [4] */
	{ .enable = true, .delay_fatal = false }, /* [5] */
};

/* Global OEM configuration info exported to the common driver */
struct nocerr_config_info_oem nocerr_config_info_oem = {
	.num_configs = 1U,
	.configs = (struct nocerr_propdata_type_oem []){
		{
			.family         = (uint32_t)CHIPINFO_FAMILY_NORDAU,
			.match          = false,
			.version        = CHIPINFO_VERSION(1, 0),
			.len            = sizeof(nocerr_cfgdata_oem) /
					  sizeof(nocerr_cfgdata_oem[0]),
			.noc_info_oem   = nocerr_cfgdata_oem,
			.clock_reg_vals = clock_reg_vals,
			.filters        = filters_oem,
			.reg_addr       = NULL,
			.reg_mask       = 0U,
			.reg_val        = 0U,
		},
	},
};
