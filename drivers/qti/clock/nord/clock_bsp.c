/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Clock BSP data for nord (full multi-instance: dual GPU/display,
 * multi-NSP).
 *
 * Only the data exercised by the boot flow is defined: the clock groups
 * enabled/disabled during init, and the PLL sources voted on by the
 * back-end. Nord has four independent GCC domains (main GCC, NE_GCC,
 * NW_GCC, SE_GCC), each with its own GPLL0 and vote register.
 *
 * The multimedia clocks/power-domains/access-clocks/voltage-requests are
 * always present on this device, so they are included directly in
 * CLOCK_GROUP_INIT below rather than modeled as a separate conditional
 * group.
 *
 * The NSP0-3 and MDP0 -> EBI1 bandwidth votes that the old TZ-era BSP data
 * declared via clock_group.icb_requests have no replacement in the current
 * framework (that field was removed along with the rest of the clock
 * driver's built-in ICB integration); they are intentionally dropped here.
 */

#include <stdint.h>

#include <drivers/qti/clock/clock.h>
#include <drivers/qti/clock/clock_bsp.h>
#include <drivers/qti/clock/clock_driver.h>
#include "clock_hwio.h"
#include <drivers/qti/pwr_utils/voltage_level.h>

/* Forward declaration of the source table referenced by the clock groups. */
static struct clock_source sources[CLOCK_SOURCE_TOTAL];

/*
 * Clock groups.
 */
static struct clock_group clock_groups[CLOCK_GROUP_TOTAL] = {
	[CLOCK_GROUP_INIT] = {
		.clocks = (struct clock_clk_desc[]) {
			/*
			 * QSPI AHB/CORE: the vote lands and reads back voted=1
			 * in the vote register, but the CBCR itself never
			 * reports CLK_OFF=0 and the enable call times out.
			 * Nord Ride boots via UFS, not QSPI-attached flash, so
			 * the QSPI block is likely not populated on this board.
			 * Commented out pending hardware confirmation.
			 */
			/* { GCC_QSPI_AHB_CBCR, VOTE_3(QSPI_AHB_CLK_ENA) }, */
			/* { GCC_QSPI_CORE_CBCR, VOTE_3(QSPI_CORE_CLK_ENA) }, */
			{ GCC_QUPV3_WRAP3_CORE_CBCR,
			  VOTE(QUPV3_WRAP3_CORE_CLK_ENA) },
			{ GCC_QUPV3_WRAP3_M_CBCR,
			  VOTE(QUPV3_WRAP3_M_CLK_ENA) },
			{ GCC_QUPV3_WRAP3_S_AHB_CBCR,
			  VOTE_2(QUPV3_WRAP3_S_AHB_CLK_ENA) },
			{ NE_GCC_CE1_AHB_CBCR, NE_VOTE(CE1_AHB_CLK_ENA) },
			{ NE_GCC_CE1_CBCR, NE_VOTE(CE1_CLK_ENA) },
			{ NE_GCC_QUPV3_WRAP2_CORE_CBCR,
			  NE_VOTE_1(QUPV3_WRAP2_CORE_CLK_ENA) },
			{ NE_GCC_QUPV3_WRAP2_M_AHB_CBCR,
			  NE_VOTE(QUPV3_WRAP2_M_AHB_CLK_ENA) },
			{ NE_GCC_QUPV3_WRAP2_S_AHB_CBCR,
			  NE_VOTE(QUPV3_WRAP2_S_AHB_CLK_ENA) },
			{ NE_GCC_SDCC4_AXI_CBCR },
			{ NE_GCC_UFS_PHY_ICE_CORE_CBCR },
			{ SE_GCC_QUPV3_WRAP0_CORE_CBCR,
			  SE_VOTE(QUPV3_WRAP0_CORE_CLK_ENA) },
			{ SE_GCC_QUPV3_WRAP0_M_AHB_CBCR,
			  SE_VOTE(QUPV3_WRAP0_M_AHB_CLK_ENA) },
			{ SE_GCC_QUPV3_WRAP0_S_AHB_CBCR,
			  SE_VOTE(QUPV3_WRAP0_S_AHB_CLK_ENA) },
			{ SE_GCC_QUPV3_WRAP1_CORE_CBCR,
			  SE_VOTE(QUPV3_WRAP1_CORE_CLK_ENA) },
			{ SE_GCC_QUPV3_WRAP1_M_AHB_CBCR,
			  SE_VOTE(QUPV3_WRAP1_M_AHB_CLK_ENA) },
			{ SE_GCC_QUPV3_WRAP1_S_AHB_CBCR,
			  SE_VOTE(QUPV3_WRAP1_S_AHB_CLK_ENA) },
			/*
			 * HPASS_CC_NOC_DEBUG_CBCR: HW_CTL-gated debug/trace NoC
			 * tap that only ungates on active bus/debug traffic,
			 * which doesn't exist this early in boot, so the
			 * blocking CLK_OFF poll always times out. Commented out
			 * pending a non-blocking enable path for this class of
			 * clock (raw=0x88000001 enable=1 hw_ctl=0 off=1).
			 */
			/* { HPASS_CC_NOC_DEBUG_CBCR }, */
			{ NE_GCC_AGGRE_NOC_USB3_PRIM_AXI_CBCR },
			{ NE_GCC_AGGRE_NOC_USB3_SEC_AXI_CBCR },
			{ NE_GCC_AGGRE_NOC_USB2_AXI_CBCR },

			{ NSPSS_0_NSP_SS_CC_CENG_NOC_CBCR,          .part = CHIPINFO_PART_NSP, .part_idx = 0 },
			{ NSPSS_0_NSP_SS_CC_CENG_NSP_CBCR,          .part = CHIPINFO_PART_NSP, .part_idx = 0 },
			{ NSPSS_0_NSP_SS_CC_GEMNOC_CBCR,            .part = CHIPINFO_PART_NSP, .part_idx = 0 },
			{ NSPSS_0_NSP_SS_CC_NOC_HCP_CORE_CBCR,      .part = CHIPINFO_PART_NSP, .part_idx = 0 },
			{ NSPSS_0_NSP_SS_CC_NOC_HCP_CORE_MSF_CBCR,  .part = CHIPINFO_PART_NSP, .part_idx = 0 },
			{ NSPSS_0_NSP_SS_CC_NOC_UBWCD_CBCR,         .part = CHIPINFO_PART_NSP, .part_idx = 0 },
			{ NSPSS_0_NSP_SS_CC_NOC_UBWCD_MSF_CBCR,     .part = CHIPINFO_PART_NSP, .part_idx = 0 },
			{ NSPSS_0_NSP_SS_CC_NSPAUX_XO_CBCR,         .part = CHIPINFO_PART_NSP, .part_idx = 0 },
			{ NSPSS_0_NSP_SS_CC_NSPNOC_CBCR,            .part = CHIPINFO_PART_NSP, .part_idx = 0 },
			{ NSPSS_0_NSP_SS_CC_NSPNOC_CFG_AHBS_CBCR,   .part = CHIPINFO_PART_NSP, .part_idx = 0 },
			{ NSPSS_0_NSP_SS_CC_VTCM_BOOT_CBCR,         .part = CHIPINFO_PART_NSP, .part_idx = 0 },

			{ NSPSS_1_NSP_SS_CC_CENG_NOC_CBCR,          .part = CHIPINFO_PART_NSP, .part_idx = 1 },
			{ NSPSS_1_NSP_SS_CC_CENG_NSP_CBCR,          .part = CHIPINFO_PART_NSP, .part_idx = 1 },
			{ NSPSS_1_NSP_SS_CC_GEMNOC_CBCR,            .part = CHIPINFO_PART_NSP, .part_idx = 1 },
			{ NSPSS_1_NSP_SS_CC_NOC_HCP_CORE_CBCR,      .part = CHIPINFO_PART_NSP, .part_idx = 1 },
			{ NSPSS_1_NSP_SS_CC_NOC_HCP_CORE_MSF_CBCR,  .part = CHIPINFO_PART_NSP, .part_idx = 1 },
			{ NSPSS_1_NSP_SS_CC_NOC_UBWCD_CBCR,         .part = CHIPINFO_PART_NSP, .part_idx = 1 },
			{ NSPSS_1_NSP_SS_CC_NOC_UBWCD_MSF_CBCR,     .part = CHIPINFO_PART_NSP, .part_idx = 1 },
			{ NSPSS_1_NSP_SS_CC_NSPAUX_XO_CBCR,         .part = CHIPINFO_PART_NSP, .part_idx = 1 },
			{ NSPSS_1_NSP_SS_CC_NSPNOC_CBCR,            .part = CHIPINFO_PART_NSP, .part_idx = 1 },
			{ NSPSS_1_NSP_SS_CC_NSPNOC_CFG_AHBS_CBCR,   .part = CHIPINFO_PART_NSP, .part_idx = 1 },
			{ NSPSS_1_NSP_SS_CC_VTCM_BOOT_CBCR,         .part = CHIPINFO_PART_NSP, .part_idx = 1 },

			{ NSPSS_2_NSP_SS_CC_CENG_NOC_CBCR,          .part = CHIPINFO_PART_NSP, .part_idx = 2 },
			{ NSPSS_2_NSP_SS_CC_CENG_NSP_CBCR,          .part = CHIPINFO_PART_NSP, .part_idx = 2 },
			{ NSPSS_2_NSP_SS_CC_GEMNOC_CBCR,            .part = CHIPINFO_PART_NSP, .part_idx = 2 },
			{ NSPSS_2_NSP_SS_CC_NOC_HCP_CORE_CBCR,      .part = CHIPINFO_PART_NSP, .part_idx = 2 },
			{ NSPSS_2_NSP_SS_CC_NOC_HCP_CORE_MSF_CBCR,  .part = CHIPINFO_PART_NSP, .part_idx = 2 },
			{ NSPSS_2_NSP_SS_CC_NOC_UBWCD_CBCR,         .part = CHIPINFO_PART_NSP, .part_idx = 2 },
			{ NSPSS_2_NSP_SS_CC_NOC_UBWCD_MSF_CBCR,     .part = CHIPINFO_PART_NSP, .part_idx = 2 },
			{ NSPSS_2_NSP_SS_CC_NSPAUX_XO_CBCR,         .part = CHIPINFO_PART_NSP, .part_idx = 2 },
			{ NSPSS_2_NSP_SS_CC_NSPNOC_CBCR,            .part = CHIPINFO_PART_NSP, .part_idx = 2 },
			{ NSPSS_2_NSP_SS_CC_NSPNOC_CFG_AHBS_CBCR,   .part = CHIPINFO_PART_NSP, .part_idx = 2 },
			{ NSPSS_2_NSP_SS_CC_VTCM_BOOT_CBCR,         .part = CHIPINFO_PART_NSP, .part_idx = 2 },

			{ NSPSS_3_NSP_SS_CC_CENG_NOC_CBCR,          .part = CHIPINFO_PART_NSP, .part_idx = 3 },
			{ NSPSS_3_NSP_SS_CC_CENG_NSP_CBCR,          .part = CHIPINFO_PART_NSP, .part_idx = 3 },
			{ NSPSS_3_NSP_SS_CC_GEMNOC_CBCR,            .part = CHIPINFO_PART_NSP, .part_idx = 3 },
			{ NSPSS_3_NSP_SS_CC_NOC_HCP_CORE_CBCR,      .part = CHIPINFO_PART_NSP, .part_idx = 3 },
			{ NSPSS_3_NSP_SS_CC_NOC_HCP_CORE_MSF_CBCR,  .part = CHIPINFO_PART_NSP, .part_idx = 3 },
			{ NSPSS_3_NSP_SS_CC_NOC_UBWCD_CBCR,         .part = CHIPINFO_PART_NSP, .part_idx = 3 },
			{ NSPSS_3_NSP_SS_CC_NOC_UBWCD_MSF_CBCR,     .part = CHIPINFO_PART_NSP, .part_idx = 3 },
			{ NSPSS_3_NSP_SS_CC_NSPAUX_XO_CBCR,         .part = CHIPINFO_PART_NSP, .part_idx = 3 },
			{ NSPSS_3_NSP_SS_CC_NSPNOC_CBCR,            .part = CHIPINFO_PART_NSP, .part_idx = 3 },
			{ NSPSS_3_NSP_SS_CC_NSPNOC_CFG_AHBS_CBCR,   .part = CHIPINFO_PART_NSP, .part_idx = 3 },
			{ NSPSS_3_NSP_SS_CC_VTCM_BOOT_CBCR,         .part = CHIPINFO_PART_NSP, .part_idx = 3 },

			/* Multimedia clocks. */
			{ CAM_CC_CSID_CBCR,               .part = CHIPINFO_PART_CAMERA, .part_idx = 0 },
			{ CAM_CC_CSID_CSIPHY_RX_CBCR,     .part = CHIPINFO_PART_CAMERA, .part_idx = 0 },
			{ CAM_CC_IFE_LITE_AHB_CBCR,       .part = CHIPINFO_PART_CAMERA, .part_idx = 0 },
			{ CAM_CC_IFE_LITE_CBCR,           .part = CHIPINFO_PART_CAMERA, .part_idx = 0 },
			{ CAM_CC_IFE_LITE_CPHY_RX_CBCR,   .part = CHIPINFO_PART_CAMERA, .part_idx = 0 },
			{ CAM_CC_IFE_LITE_CSID_CBCR,      .part = CHIPINFO_PART_CAMERA, .part_idx = 0 },
			{ CAM_CC_TOP_AHB_CBCR,             .part = CHIPINFO_PART_CAMERA, .part_idx = 0 },
			{ CAM_CC_TOP_FAST_AHB_CBCR,        .part = CHIPINFO_PART_CAMERA, .part_idx = 0 },
			{ CAM_CC_TOP_IFE_LITE_CBCR,        .part = CHIPINFO_PART_CAMERA, .part_idx = 0 },
			/*
			 * CAMERA_HF/SF_AXI_CBCR: same HW_CTL-gated NoC/AXI
			 * fabric bus clock behavior as HPASS_CC_NOC_DEBUG above
			 * - only ungates on real bus traffic, which no camera
			 * session generates this early in boot. The sibling
			 * AHB/XO branches on the same camera block enable fine.
			 * Commented out pending a non-blocking enable path
			 * (raw=0x88000001 enable=1 hw_ctl=0 off=1).
			 */
			/* { NW_GCC_CAMERA_HF_AXI_CBCR,       .part = CHIPINFO_PART_CAMERA, .part_idx = 0 }, */
			/* { NW_GCC_CAMERA_SF_AXI_CBCR,       .part = CHIPINFO_PART_CAMERA, .part_idx = 0 }, */
			{ MDSS_0_DISP_CC_MDSS_AHB_CBCR,    .part = CHIPINFO_PART_DISPLAY, .part_idx = 0 },
			/*
			 * DISP_0/1_HF_AXI_CBCR: same HW_CTL-gated NoC/AXI fabric
			 * bus clock behavior as the camera/HPASS entries above -
			 * only ungates on active display traffic, none of which
			 * exists this early in boot. Commented out pending a
			 * non-blocking enable path (raw=0x88000001 enable=1
			 * hw_ctl=0 off=1).
			 */
			/* { NW_GCC_DISP_0_HF_AXI_CBCR,       .part = CHIPINFO_PART_DISPLAY, .part_idx = 0 }, */
			{ MDSS_1_DISP_CC_MDSS_AHB_CBCR,    .part = CHIPINFO_PART_DISPLAY, .part_idx = 1 },
			{ MDSS_1_DISP_CC_MDSS_NON_GDSC_AHB_CBCR, .part = CHIPINFO_PART_DISPLAY, .part_idx = 1 },
			/* DISP_1_HF_AXI_CBCR: HW_CTL-gated, see comment above. */
			/* { NW_GCC_DISP_1_HF_AXI_CBCR,       .part = CHIPINFO_PART_DISPLAY, .part_idx = 1 }, */
			{ 0 }
		},
		.power_domains = (struct clock_power_domain_desc[]) {
			{ NE_GCC_UFS_PHY_GDSCR },
			{ NE_GCC_USB31_PRIM_GDSCR },
			/* Multimedia power domains. */
			{ CAM_CC_TITAN_TOP_GDSCR,          .part = CHIPINFO_PART_CAMERA, .part_idx = 0 },
			{ MDSS_0_DISP_CC_MDSS_CORE_GDSCR,  .part = CHIPINFO_PART_DISPLAY, .part_idx = 0 },
			{ MDSS_1_DISP_CC_MDSS_CORE_GDSCR,  .part = CHIPINFO_PART_DISPLAY, .part_idx = 1 },
			{ 0 }
		},
		.access_clocks = (struct clock_clk_desc[]) {
			{ NW_GCC_CFG_NOC_MMNOC_AHB_CBCR },
			/* Multimedia access clocks. */
			{ NE_GCC_GPU_2_CFG_CBCR,           .part = CHIPINFO_PART_GPU, .part_idx = 1 },
			{ NW_GCC_CAMERA_AHB_CBCR,          .part = CHIPINFO_PART_CAMERA, .part_idx = 0 },
			{ NW_GCC_CAMERA_XO_CBCR,           .part = CHIPINFO_PART_CAMERA, .part_idx = 0 },
			{ NW_GCC_DISP_0_AHB_CBCR,          .part = CHIPINFO_PART_DISPLAY, .part_idx = 0 },
			{ NW_GCC_DISP_1_AHB_CBCR,          .part = CHIPINFO_PART_DISPLAY, .part_idx = 1 },
			{ NW_GCC_DPRX0_CFG_AHB_CBCR,       .part = CHIPINFO_PART_DISPLAY, .part_idx = 0 },
			{ NW_GCC_DPRX1_CFG_AHB_CBCR,       .part = CHIPINFO_PART_DISPLAY, .part_idx = 1 },
			{ NW_GCC_GPU_CFG_AHB_CBCR,         .part = CHIPINFO_PART_GPU, .part_idx = 0 },
			{ NW_GCC_VIDEO_AHB_CBCR,           .part = CHIPINFO_PART_VIDEO, .part_idx = 0 },
			{ 0 }
		},
		.voltage_requests = (struct clock_voltage_request[]) {
			/* Multimedia voltage requests. */
			{ "mmcx.lvl", RAIL_VOLTAGE_LEVEL_NOM },
			{ "mxc.lvl", RAIL_VOLTAGE_LEVEL_NOM },
			{ "gfx.lvl", RAIL_VOLTAGE_LEVEL_NOM, .part = CHIPINFO_PART_GPU, .part_idx = 0 },
			{ "gfx1.lvl", RAIL_VOLTAGE_LEVEL_NOM, .part = CHIPINFO_PART_GPU, .part_idx = 1 },
			{ 0 }
		},
	},
};

/*
 * Source configuration. The back-end votes all four domains' GPLL0 on
 * during init; each domain has its own PLL_MODE and vote register.
 */
static struct clock_source sources[CLOCK_SOURCE_TOTAL] = {
	[CLOCK_SOURCE_GPLL0] = {
		.hw_source = { GCC_GPLL0_PLL_MODE, PLL_VOTE(GCC_GPLL0) },
		.source = &sources[CLOCK_SOURCE_XO]
	},
	[CLOCK_SOURCE_NE_GCC_GPLL0] = {
		.hw_source = { NE_GCC_GPLL0_PLL_MODE,
			       NE_PLL_VOTE(NE_GCC_GPLL0) },
		.source = &sources[CLOCK_SOURCE_XO]
	},
	[CLOCK_SOURCE_NW_GCC_GPLL0] = {
		.hw_source = { NW_GCC_GPLL0_PLL_MODE,
			       NW_PLL_VOTE(NW_GCC_GPLL0) },
		.source = &sources[CLOCK_SOURCE_XO]
	},
	[CLOCK_SOURCE_SE_GCC_GPLL0] = {
		.hw_source = { SE_GCC_GPLL0_PLL_MODE,
			       SE_PLL_VOTE(SE_GCC_GPLL0) },
		.source = &sources[CLOCK_SOURCE_XO]
	},
};

/*
 * Main BSP data, referenced by the framework via extern.
 */
struct clock_tfa_bsp clock_tfa_bsp_config = {
	.clock_groups = clock_groups,
	.sources      = sources,
};
