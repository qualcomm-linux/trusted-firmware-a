/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Clock BSP data for lemans (qcs9075).
 *
 * Only the data exercised by the boot flow is defined: the clock groups
 * enabled/disabled during init, and the PLL sources voted on by the back-end.
 * The per-clock by-name table and QUP frequency plans from the original vendor
 * BSP are omitted, as the matching control APIs are not built for this target.
 */

#include <stdint.h>

#include <drivers/qti/clock/clock.h>
#include <drivers/qti/clock/clock_bsp.h>
#include <drivers/qti/clock/clock_driver.h>
#include <drivers/qti/clock/clock_hwio.h>
#include <drivers/qti/clock/icbuarb.h>
#include <drivers/qti/pwr_utils/voltage_level.h>

/* Forward declaration of the source table referenced by the clock groups. */
static struct clock_source sources[CLOCK_SOURCE_TOTAL];

/*
 * Clock groups.
 */
static struct clock_group clock_groups[CLOCK_GROUP_TOTAL] = {
	[CLOCK_GROUP_INIT] = {
		.clocks = (struct clock_clk_desc[]) {
			{ CLOCK_GCC_CAMERA_HF_AXI_CBCR },
			{ CLOCK_GCC_CAMERA_SF_AXI_CBCR },
			{ CLOCK_GCC_CE0_AHB_CBCR, CLOCK_VOTE(CE0_AHB_CLK_ENA) },
			{ CLOCK_GCC_CE1_AHB_CBCR, CLOCK_VOTE(CE1_AHB_CLK_ENA) },
			{ CLOCK_GCC_DCC_AHB_CBCR },
			{ CLOCK_GCC_DISP_HF_AXI_CBCR },
			{ CLOCK_GCC_IPA_2X_CBCR },
			{ CLOCK_GCC_IPA_AHB_CBCR },
			{ CLOCK_GCC_IPA_CBCR },
			{ CLOCK_GCC_MMSS_QM_AHB_CBCR },
			{ CLOCK_GCC_MMSS_QM_CORE_CBCR },
			{ CLOCK_GCC_QUPV3_WRAP0_CORE_CBCR,
			  CLOCK_VOTE_1(QUPV3_WRAP0_CORE_CLK_ENA) },
			{ CLOCK_GCC_QUPV3_WRAP_0_M_AHB_CBCR,
			  CLOCK_VOTE_1(QUPV3_WRAP_0_M_AHB_CLK_ENA) },
			{ CLOCK_GCC_QUPV3_WRAP_0_S_AHB_CBCR,
			  CLOCK_VOTE_1(QUPV3_WRAP_0_S_AHB_CLK_ENA) },
			{ CLOCK_GCC_QUPV3_WRAP1_CORE_CBCR,
			  CLOCK_VOTE_1(QUPV3_WRAP1_CORE_CLK_ENA) },
			{ CLOCK_GCC_QUPV3_WRAP_1_M_AHB_CBCR,
			  CLOCK_VOTE_1(QUPV3_WRAP_1_M_AHB_CLK_ENA) },
			{ CLOCK_GCC_QUPV3_WRAP_1_S_AHB_CBCR,
			  CLOCK_VOTE_1(QUPV3_WRAP_1_S_AHB_CLK_ENA) },
			{ CLOCK_GCC_QUPV3_WRAP2_CORE_CBCR,
			  CLOCK_VOTE_2(QUPV3_WRAP2_CORE_CLK_ENA) },
			{ CLOCK_GCC_QUPV3_WRAP_2_M_AHB_CBCR,
			  CLOCK_VOTE_2(QUPV3_WRAP_2_M_AHB_CLK_ENA) },
			{ CLOCK_GCC_QUPV3_WRAP_2_S_AHB_CBCR,
			  CLOCK_VOTE_2(QUPV3_WRAP_2_S_AHB_CLK_ENA) },
			{ CLOCK_GCC_QUPV3_WRAP3_CORE_CBCR,
			  CLOCK_VOTE(QUPV3_WRAP3_CORE_CLK_ENA) },
			{ CLOCK_GCC_QUPV3_WRAP_3_M_AHB_CBCR,
			  CLOCK_VOTE(QUPV3_WRAP_3_M_AHB_CLK_ENA) },
			{ CLOCK_GCC_QUPV3_WRAP_3_S_AHB_CBCR,
			  CLOCK_VOTE(QUPV3_WRAP_3_S_AHB_CLK_ENA) },
			{ CLOCK_GCC_SDCC1_AHB_CBCR },
			{ CLOCK_GCC_SDCC1_ICE_CORE_CBCR },
			{ CLOCK_GCC_UFS_CARD_AHB_CBCR },
			{ CLOCK_GCC_UFS_CARD_AXI_CBCR },
			{ CLOCK_GCC_UFS_CARD_ICE_CORE_CBCR },
			{ CLOCK_GCC_UFS_PHY_AHB_CBCR },
			{ CLOCK_GCC_UFS_PHY_ICE_CORE_CBCR },
			{ CLOCK_CAM_CC_CPAS_AHB_CBCR },
			{ CLOCK_CAM_CC_CPAS_FAST_AHB_CBCR },
			{ CLOCK_CAM_CC_CORE_AHB_CBCR },
			{ CLOCK_MDSS_0_DISP_CC_MDSS_AHB_CBCR },
			{ CLOCK_MDSS_0_DISP_CC_MDSS_AHB1_CBCR },
			{ CLOCK_GCC_MMNOC_TBU_CAMERA_HF4_CBCR },
			{ CLOCK_GCC_TURING_1_Q6_TBU2_CBCR },
			{ CLOCK_GCC_TURING_1_Q6_TBU3_CBCR },
			{ CLOCK_GCC_TURING_0_Q6_TBU0_CBCR },
			{ CLOCK_GCC_TURING_0_Q6_TBU1_CBCR },
			{ CLOCK_GCC_MMNOC_TBU_HF0_CBCR },
			{ CLOCK_GCC_MMNOC_TBU_HF1_CBCR },
			{ CLOCK_LPASS_AON_CC_DEBUG_XPU_CLIENT_CBCR },
			{ CLOCK_LPASS_AON_CC_PDC_H_CBCR },
			{ CLOCK_LPASS_AON_CC_Q6_XPU2_CLIENT_CBCR },
			{ CLOCK_LPASS_AON_CC_Q6_XPU2_CONFIG_CBCR },
			{ CLOCK_LPASS_AON_CC_VA_XPU2_CLIENT_CBCR },
			{ CLOCK_LPASS_AUDIO_CC_XPU2_CLIENT_CBCR },
			{ CLOCK_LPASS_AUDIO_CC_BUS_CBCR },
			{ 0 }
		},
		.power_domains = (struct clock_power_domain_desc[]) {
			{ CLOCK_CAM_CC_TITAN_TOP_GDSCR },
			{ CLOCK_MDSS_0_DISP_CC_MDSS_CORE_GDSCR },
			{ CLOCK_GCC_IPA_GDSCR },
			{ CLOCK_GCC_MMNOC_GDSCR },
			{ CLOCK_LPASS_AON_CC_LPASS_AUDIO_HM_GDSCR },
			{ CLOCK_LPASS_AON_CC_LPASS_PDC_HM_GDSCR },
			{ CLOCK_LPASS_AON_CC_LPASS_AUDIO_ML_GDSCR },
			{ CLOCK_GCC_USB30_PRIM_GDSCR },
			{ CLOCK_GCC_USB30_SEC_GDSCR },
			{ CLOCK_GCC_PCIE_0_GDSCR },
			{ CLOCK_GCC_PCIE_1_GDSCR },
			{ CLOCK_GCC_UFS_CARD_GDSCR },
			{ CLOCK_GCC_UFS_PHY_GDSCR },
			{ CLOCK_GCC_EMAC1_GDSCR },
			{ CLOCK_GCC_TURING_0_GDSCR },
			{ 0 }
		},
		.access_clocks = (struct clock_clk_desc[]) {
			{ CLOCK_GCC_CAMERA_AHB_CBCR },
			{ CLOCK_GCC_CAMERA_XO_CBCR },
			{ CLOCK_GCC_CFG_NOC_MMNOC_AHB_CBCR },
			{ CLOCK_GCC_DISP_AHB_CBCR },
			{ CLOCK_GCC_DISP1_AHB_CBCR },
			{ CLOCK_GCC_DISP_XO_CBCR },
			{ CLOCK_GCC_DISP1_XO_CBCR },
			{ CLOCK_GCC_GPU_CFG_AHB_CBCR },
			{ CLOCK_GCC_LPASS_CFG_NOC_SWAY_CBCR },
			{ CLOCK_GCC_VIDEO_AHB_CBCR },
			{ 0 }
		},
		.icb_requests = (struct clock_icb_request[]) {
			{ ICBID_MASTER_MDP0,        ICBID_SLAVE_EBI1,     1 },
			{ ICBID_MASTER_IPA_CORE,    ICBID_SLAVE_IPA_CORE, 1 },
			{ ICBID_MASTER_CDSP_PROC,   ICBID_SLAVE_EBI1,     1 },
			{ ICBID_MASTER_CDSP_PROC_B, ICBID_SLAVE_EBI1,     1 },
			{ ICBID_MASTER_PCIE_0,      ICBID_SLAVE_EBI1,     1 },
			{ ICBID_MASTER_PKA_CORE,    ICBID_SLAVE_PKA_CORE, 1 },
			{ 0 }
		},
		.voltage_requests = (struct clock_voltage_request[]) {
			{ "mmcx.lvl", RAIL_VOLTAGE_LEVEL_NOM },
			{ "gfx.lvl",  RAIL_VOLTAGE_LEVEL_NOM },
			{ 0 }
		},
	},

	[CLOCK_GROUP_INIT_SSC] = {
		.clocks = (struct clock_clk_desc[]) {
			{ 0 }
		},
	},

	[CLOCK_GROUP_QDSS] = {
		.clocks = (struct clock_clk_desc[]) {
			/*
			 * This register enables the CDBGPWRUPREQ signal that
			 * enables all QDSS-related clocks.
			 */
			{ CLOCK_GCC_DEBUG_EN },
			{ 0 }
		},
	},

	[CLOCK_GROUP_VSENSE_PRIMARY] = {
		.clocks = (struct clock_clk_desc[]) {
			{ CLOCK_GCC_APC0_VS_PRIM_CBCR },
			{ CLOCK_GCC_APC1_VS_PRIM_CBCR },
			{ CLOCK_GCC_GPU_VS_PRIM_CBCR },
			{ CLOCK_GCC_VDDADDR_VS_PRIM_CBCR },
			{ CLOCK_GCC_VDDCX_VS_PRIM_CBCR },
			{ CLOCK_GCC_VDDMMCX_VS_PRIM_CBCR },
			{ CLOCK_GCC_VDDMXA_VS_PRIM_CBCR },
			{ CLOCK_GCC_VDDMXC_VS_PRIM_CBCR },
			{ 0 }
		},
	},

	[CLOCK_GROUP_VSENSE_SECONDARY] = {
		.clocks = (struct clock_clk_desc[]) {
			{ 0 }
		},
	},
};

/*
 * Source configuration. The back-end votes GPLL0 on during init; the other
 * PLLs are described for completeness of the source tree.
 */
static struct clock_source sources[CLOCK_SOURCE_TOTAL] = {
	[CLOCK_SOURCE_GPLL0] = {
		.hw_source = { CLOCK_GCC_GPLL0_PLL_MODE,
			       CLOCK_PLL_VOTE_MASK(GCC_GPLL0),
			       CLOCK_GCC_GPLL0_PLL_MODE_PLL_LOCK_DET_BMSK },
		.source = &sources[CLOCK_SOURCE_XO]
	},
	[CLOCK_SOURCE_GPLL0_DIV2] = {
		.source = &sources[CLOCK_SOURCE_GPLL0]
	},
	[CLOCK_SOURCE_GPLL6] = {
		.hw_source = { CLOCK_GCC_GPLL6_PLL_MODE,
			       CLOCK_PLL_VOTE_MASK(GCC_GPLL6),
			       CLOCK_GCC_GPLL6_PLL_MODE_PLL_LOCK_DET_BMSK },
		.source = &sources[CLOCK_SOURCE_XO]
	},
	[CLOCK_SOURCE_GPLL4] = {
		.hw_source = { CLOCK_GCC_GPLL4_PLL_MODE,
			       CLOCK_PLL_VOTE_MASK(GCC_GPLL4),
			       CLOCK_GCC_GPLL4_PLL_MODE_PLL_LOCK_DET_BMSK },
		.source = &sources[CLOCK_SOURCE_XO]
	}
};

/*
 * Main BSP data, referenced by the framework via extern.
 */
struct clock_tz_bsp clock_tz_bsp_config = {
	.clock_groups = clock_groups,
	.sources      = sources,
};

/*
 * The shared PLL enable-vote register address (chipset specific). Consumed by
 * the folded HAL source accessors in clock_hal.c.
 */
uintptr_t clock_hal_get_pll_vote_addr(void)
{
	return CLOCK_GCC_APCS_TZ_PLL_BRANCH_ENA_VOTE;
}
