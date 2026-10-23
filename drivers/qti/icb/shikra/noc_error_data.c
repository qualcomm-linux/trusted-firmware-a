/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * NoC error configuration data for the Shikra platform.
 * Provides nocerr_config_info, consumed by drivers/qti/icb/noc_error.c.
 */

#include <drivers/qti/chipinfo/chipinfo.h>

#include "noc_error.h"
#include "noc_error_hwio.h"
#include "noc_hw_type.h"

/* Shikra NOC configuration data - 5 NOC instances */
static struct nocerr_info_type nocerr_cfgdata[] = {
	/* [0] CONFIG_NOC */
	{
		.name          = "CONFIG_NOC",
		.hw            = &qnoc_4_0,
		.base_addr     = (void *)CNOC_OBS_SWID_LOW,
		.intr_vector   = 282U,
		.num_sbms      = 5U,
		.sb_hw         = (struct noc_sideband_hw *[]){
			&qnoc_sb_4_0_2l, &qnoc_sb_4_0_l, &qnoc_sb_4_0_l,
			&qnoc_sb_4_0_l, &qnoc_sb_4_0_l,
		},
		.sb_base_addrs = (void *[]){
			(void *)CNOC_SBM_SWID_LOW,
			(void *)CNOC_NORTH_FM_SWID_LOW,
			(void *)CNOC_PCIE_FM_SWID_LOW,
			(void *)CNOC_WEST_FM_SWID_LOW,
			(void *)CNOC_SBM_TIC_SWID_LOW,
		},
		.num_tos       = 31U,
		.to_addrs      = (void *[]){
			(void *)CONFIG_NOC_REFGEN_REFCTLn(0),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(1),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(2),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(3),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(4),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(5),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(6),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(7),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(8),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(9),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(10),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(11),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(12),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(13),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(14),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(15),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(16),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(17),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(18),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(19),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(20),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(21),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(22),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(23),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(24),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(25),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(26),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(27),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(28),
			(void *)CONFIG_NOC_REFGEN_REFCTLn(29),
			(void *)CONFIG_NOC_REFGEN_MAINCTL_LOW,
		},
		.syndrome      = {
			.sbms = (struct nocerr_sbm_syndrome []){
				{0}, {0}, {0}, {0}, {0},
			},
		},
	},
	/* [1] SYSTEM_NOC */
	{
		.name          = "SYSTEM_NOC",
		.hw            = &qnoc_4_0,
		.base_addr     = (void *)SNOC_OBS_ERRORLOGGER_SWID_LOW,
		.intr_vector   = 281U,
		.num_sbms      = 1U,
		.sb_hw         = (struct noc_sideband_hw *[]){ &qnoc_sb_4_0_l },
		.sb_base_addrs = (void *[]){
			(void *)SYSTEM_NOC_SBM_SWID_LOW,
		},
		.num_tos       = 10U,
		.to_addrs      = (void *[]){
			(void *)SYSTEM_NOC_REFGEN_REFCTLn(0),
			(void *)SYSTEM_NOC_REFGEN_REFCTLn(1),
			(void *)SYSTEM_NOC_REFGEN_REFCTLn(2),
			(void *)SYSTEM_NOC_REFGEN_REFCTLn(3),
			(void *)SYSTEM_NOC_REFGEN_REFCTLn(4),
			(void *)SYSTEM_NOC_REFGEN_REFCTLn(5),
			(void *)SYSTEM_NOC_REFGEN_REFCTLn(6),
			(void *)SYSTEM_NOC_REFGEN_REFCTLn(7),
			(void *)SYSTEM_NOC_REFGEN_REFCTLn(8),
			(void *)SYSTEM_NOC_REFGEN_MAINCTL_LOW,
		},
		.syndrome      = {
			.sbms = (struct nocerr_sbm_syndrome []){ {0} },
		},
	},
	/* [2] MEM_NOC_CORE */
	{
		.name          = "MEM_NOC_CORE",
		.hw            = &qnoc_4_0,
		.base_addr     = (void *)MEMNOC_HM_MEM_NOC_CORE_ERL_SWID_LOW,
		.intr_vector   = 317U,
		.num_sbms      = 1U,
		.sb_hw         = (struct noc_sideband_hw *[]){ &qnoc_sb_4_0_l },
		.sb_base_addrs = (void *[]){
			(void *)MEMNOC_HM_MEM_NOC_CORE_FAULT_SBM_SWID_LOW,
		},
		.num_tos       = 7U,
		.to_addrs      = (void *[]){
			(void *)MEMNOC_HM_MEM_NOC_CORE_REFGEN_REFCTLn(0),
			(void *)MEMNOC_HM_MEM_NOC_CORE_REFGEN_REFCTLn(1),
			(void *)MEMNOC_HM_MEM_NOC_CORE_REFGEN_REFCTLn(2),
			(void *)MEMNOC_HM_MEM_NOC_CORE_REFGEN_REFCTLn(3),
			(void *)MEMNOC_HM_MEM_NOC_CORE_REFGEN_REFCTLn(4),
			(void *)MEMNOC_HM_MEM_NOC_CORE_REFGEN_REFCTLn(5),
			(void *)MEMNOC_HM_MEM_NOC_CORE_REFGEN_MAINCTL_LOW,
		},
		.syndrome      = {
			.sbms = (struct nocerr_sbm_syndrome []){ {0} },
		},
	},
	/* [3] DEFRAG_NOC */
	{
		.name          = "DEFRAG_NOC",
		.hw            = &qnoc_4_0,
		.base_addr     = (void *)MEMNOC_HM_DEFRAG_NOC_ERL_SWID_LOW,
		.intr_vector   = 317U,
		.num_sbms      = 1U,
		.sb_hw         = (struct noc_sideband_hw *[]){ &qnoc_sb_4_0_l },
		.sb_base_addrs = (void *[]){
			(void *)MEMNOC_HM_DEFRAG_NOC_SBM_SWID_LOW,
		},
		.num_tos       = 4U,
		.to_addrs      = (void *[]){
			(void *)MEMNOC_HM_DEFRAG_NOC_REFGEN_REFCTLn(0),
			(void *)MEMNOC_HM_DEFRAG_NOC_REFGEN_REFCTLn(1),
			(void *)MEMNOC_HM_DEFRAG_NOC_REFGEN_REFCTLn(2),
			(void *)MEMNOC_HM_DEFRAG_NOC_REFGEN_MAINCTL_LOW,
		},
		.syndrome      = {
			.sbms = (struct nocerr_sbm_syndrome []){ {0} },
		},
	},
	/* [4] DC_NOC */
	{
		.name          = "DC_NOC",
		.hw            = &qnoc_4_0,
		.base_addr     = (void *)DC_NOC_DC_NOC_ERL_SWID_LOW,
		.intr_vector   = 284U,
		.num_sbms      = 1U,
		.sb_hw         = (struct noc_sideband_hw *[]){ &qnoc_sb_4_0_l },
		.sb_base_addrs = (void *[]){
			(void *)DC_NOC_DC_NOC_ERROR_SBM_SWID_LOW,
		},
		.num_tos       = 8U,
		.to_addrs      = (void *[]){
			(void *)DC_NOC_DC_NOC_REFGEN_REFCTLn(0),
			(void *)DC_NOC_DC_NOC_REFGEN_REFCTLn(1),
			(void *)DC_NOC_DC_NOC_REFGEN_REFCTLn(2),
			(void *)DC_NOC_DC_NOC_REFGEN_REFCTLn(3),
			(void *)DC_NOC_DC_NOC_REFGEN_REFCTLn(4),
			(void *)DC_NOC_DC_NOC_REFGEN_REFCTLn(5),
			(void *)DC_NOC_DC_NOC_REFGEN_REFCTLn(6),
			(void *)DC_NOC_DC_NOC_REFGEN_MAINCTL_LOW,
		},
		.syndrome      = {
			.sbms = (struct nocerr_sbm_syndrome []){ {0} },
		},
	},
};

/* External timeout clock registers */
static void *clock_reg_addrs[] = {
	(void *)GCC_NOC_BUS_TIMEOUT_EXTREF_CBCR,
	(void *)GCC_NOC_BUS_TIMEOUT_EXTREF_CDIVR,
	(void *)GCC_NOC_BUS_TIMEOUT_EXTREF_DIV512_CDIVR,
};

/* Global configuration info exported to the common driver */
struct nocerr_config_info nocerr_config_info = {
	.num_configs = 1U,
	.configs     = (struct nocerr_propdata_type []){
		/* Target info: Shikra v1.0 and later */
		[0] = {
			.family          = CHIPINFO_FAMILY_SHIKRA,
			.match           = false,
			.version         = CHIPINFO_VERSION(1, 0),
			.len             = sizeof(nocerr_cfgdata) /
					   sizeof(nocerr_cfgdata[0]),
			.noc_info        = nocerr_cfgdata,
			.num_clock_regs  = sizeof(clock_reg_addrs) /
					   sizeof(clock_reg_addrs[0]),
			.clock_reg_addrs = clock_reg_addrs,
			.num_filters    = 0U,
			.filters        = NULL,
			.reg_addr       = NULL,
			.reg_mask       = 0U,
			.reg_val        = 0U,
		},
	},
};
