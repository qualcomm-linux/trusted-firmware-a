/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Clock configuration data for shikra. Only the data the boot flow uses is
 * defined: the init clock groups and the PLL sources.
 */

#include <stdint.h>

#include "clock_hwio.h"
#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/clock/clock.h>
#include <drivers/qti/clock/clock_cfg.h>
#include <drivers/qti/clock/clock_driver.h>

/* GPLL0 is the parent of the branches enabled during init. */
static struct clock_source sources[CLOCK_SOURCE_TOTAL] = {
	[CLOCK_SOURCE_GPLL0] = {
		.hw_source = { GCC_GPLL0_PLL_MODE,
			       PLL_VOTE(GPLL0) },
		.source = &sources[CLOCK_SOURCE_XO]
	}
};

static struct clock_group clock_groups[CLOCK_GROUP_TOTAL] = {
	[CLOCK_GROUP_INIT] = {
		.clks = (struct clock_desc[]) {
			{ GCC_NOC_BUS_TIMEOUT_EXTREF_CBCR },
			{ GCC_SYS_NOC_AXI_CBCR },
			{ GCC_CFG_NOC_AHB_CBCR },
			{ GCC_RPM_BUS_AHB_CBCR },
			{ GCC_CE1_CBCR,       VOTE_1(CE1_CLK_ENA) },
			{ GCC_CE1_AXI_CBCR,   VOTE_1(CE1_AXI_CLK_ENA) },
			{ GCC_CE1_AHB_CBCR,   VOTE_1(CE1_AHB_CLK_ENA) },
			{ GCC_IPA_2X_CBCR },
			{ GCC_IPA_CBCR },
			{ GCC_IPA_AHB_CBCR },
			{ GCC_IPA_SLEEP_CBCR },
			{ GCC_MMSS_QM_AHB_CBCR },
			{ GCC_PIMEM_AXI_CBCR },
			{ GCC_PIMEM_AHB_CBCR },
			{ GCC_MSG_RAM_AHB_CBCR,
			  VOTE_1(MSG_RAM_AHB_CLK_ENA) },
			{ GCC_QUPV3_WRAP0_CORE_CBCR,
			  VOTE(QUPV3_WRAP0_CORE_CLK_ENA) },
			{ GCC_QUPV3_WRAP_0_M_AHB_CBCR,
			  VOTE(QUPV3_WRAP_0_M_AHB_CLK_ENA) },
			{ GCC_QUPV3_WRAP_0_S_AHB_CBCR,
			  VOTE(QUPV3_WRAP_0_S_AHB_CLK_ENA) },
			{ GCC_LPASS_CORE_AXIM_CBCR },
			{ GCC_SYS_NOC_COMPUTE_SF_AXI_CBCR },
			{ GCC_SDCC1_APPS_CBCR },
			{ GCC_SDCC1_AHB_CBCR },
			{ GCC_SDCC1_ICE_CORE_CBCR },
			{ GCC_SDCC2_AHB_CBCR },
			{ GCC_SDCC2_APPS_CBCR },
			{ GCC_IMEM_CFG_AHB_CBCR },
			{ GCC_SPDM_MSTR_AHB_CBCR },
			{ GCC_SPDM_FF_CBCR },
			{ GCC_DDRSS_GPU_AXI_CBCR },
			{ GCC_DCC_AHB_CBCR },
			{ GCC_MCU_AHB_CBCR },
			{ GCC_QPIC_AHB_CBCR },
			{ GCC_QPIC_CBCR },
			{ GCC_CAMERA_AHB_CBCR,    .part = CHIPINFO_PART_CAMERA },
			{ GCC_CAMSS_TOP_AHB_CBCR, .part = CHIPINFO_PART_CAMERA },
			{ GCC_CAMSS_AXI_CBCR,     .part = CHIPINFO_PART_CAMERA },
			{ GCC_CAMSS_NRT_AXI_CBCR, .part = CHIPINFO_PART_CAMERA },
			{ GCC_CAMSS_RT_AXI_CBCR,  .part = CHIPINFO_PART_CAMERA },
			{ DISP_CC_MDSS_AHB_CBCR,          .part = CHIPINFO_PART_DISPLAY },
			{ DISP_CC_MDSS_MDP_CBCR,          .part = CHIPINFO_PART_DISPLAY },
			{ DISP_CC_MDSS_NON_GDSC_AHB_CBCR, .part = CHIPINFO_PART_DISPLAY },
			{ GCC_QMIP_DISP_AHB_CBCR,
			  VOTE(QMIP_DISP_AHB_CLK_ENA),  .part = CHIPINFO_PART_DISPLAY },
			{ GCC_MSS_CFG_AHB_CBCR,           .part = CHIPINFO_PART_MODEM },
			{ GCC_MSS_Q6_DDRSS_AXI_CBCR,      .part = CHIPINFO_PART_MODEM },
			{ GCC_MSS_Q6_DDRSS_AXI_PIPE_CBCR, .part = CHIPINFO_PART_MODEM },
			{ GCC_MSS_Q6_MSMPU_CFG_AHB_CBCR,  .part = CHIPINFO_PART_MODEM },
			{ 0 }
		},
		.pwr_domains = (struct clock_power_domain_desc[]) {
			{ GCC_USB30_PRIM_GDSCR },
			{ GCC_CAMSS_TOP_GDSCR,    .part = CHIPINFO_PART_CAMERA },
			{ DISP_CC_MDSS_CORE_GDSCR, .part = CHIPINFO_PART_DISPLAY },
			{ 0 }
		},
		.access_clks = (struct clock_desc[]) {
			{ GCC_GPU_CFG_AHB_CBCR, .part = CHIPINFO_PART_GPU },
			{ GCC_CAMERA_XO_CBCR,   .part = CHIPINFO_PART_CAMERA },
			{ GCC_DISP_XO_CBCR,     .part = CHIPINFO_PART_DISPLAY },
			{ GCC_DISP_AHB_CBCR,    .part = CHIPINFO_PART_DISPLAY },
			{ 0 }
		},
	},

	[CLOCK_GROUP_QDSS] = {
		.clks = (struct clock_desc[]) {
			/*
			 * This register enables the CDBGPWRUPREQ signal that
			 * enables all QDSS-related clocks.
			 */
			{ GCC_GLOBAL_EN, { GCC_GLOBAL_EN,
					   GCC_GLOBAL_EN_CDBGPWRUPREQ_BMSK } },
			{ 0 }
		},
	},
};

struct clock_config clock_cfg = {
	.clock_groups = clock_groups,
	.sources      = sources,
};
