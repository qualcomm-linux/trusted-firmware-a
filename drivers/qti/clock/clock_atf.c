/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TZ-specific clock driver back-end for lemans (qcs9075): image/CPU init hooks
 * and the GPU GDSC, DCD and boot-IMEM teardown sequences.
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/clock/clock.h>
#include <drivers/qti/clock/clock_bsp.h>
#include <drivers/qti/clock/clock_driver.h>
#include <drivers/qti/clock/clock_hwio.h>
#include <drivers/qti/clock/qti_sec_chipinfo.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

/* Branch-control CLK_HW_CTL field (generic across CBCRs). */
#define CLOCK_BRANCH_CTRL_CLK_HW_CTL_BMSK	0x2U

/* Whether to bring up the GPU GDSCs during init. */
static const bool clock_enable_gpu = true;

/* Whether to enable the multimedia dynamic clock dividers. */
static const bool clock_dcd_enable = true;

/* Registers programmed during multimedia DCD setup. */
static const uintptr_t clock_dcd_cfg[] = {
	CLOCK_VCODEC_VIDEO_CC_MVS0C_DIV_DCDR,
	CLOCK_VCODEC_VIDEO_CC_MVS1C_DIV_DCDR,
};

/*
 * Enable a branch clock and spin (bounded) until it reports on. No error is
 * returned if it does not turn on in time; this is used in teardown paths
 * where a reset must be asserted regardless. Mirrors the vendor
 * ENABLE_CBCR_AND_SPIN_NO_ERROR helper.
 */
static void clock_enable_cbcr_and_spin(uintptr_t cbcr, uint32_t clk_off_bmsk)
{
	uint32_t max_delay = 500U;

	mmio_setbits_32(cbcr, CLOCK_GCC_CFG_NOC_LPASS_CBCR_CLK_ENABLE_BMSK);

	if ((mmio_read_32(cbcr) & CLOCK_BRANCH_CTRL_CLK_HW_CTL_BMSK) != 0U) {
		return;
	}

	while ((max_delay != 0U) &&
	       ((mmio_read_32(cbcr) & clk_off_bmsk) != 0U)) {
		udelay(1U);
		max_delay--;
	}
}

static void clock_enable_gpu_gx_gdsc(void)
{
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_GFX3D_AON_DIV_CTRL,
			   CLOCK_GPUCC_GPU_CC_GFX3D_AON_DIV_CTRL_DIV_BY_BMSK,
			   2U << CLOCK_GPUCC_GPU_CC_GFX3D_AON_DIV_CTRL_DIV_BY_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL2,
			   CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL2_BITS_9_1_BMSK,
			   250U << CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL2_BITS_9_1_SHFT);
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_GX_CFG2_GDSCR,
			   CLOCK_GPUCC_GPU_CC_GX_CFG2_GDSCR_DLY_ASSERT_CLAMP_MEM_BMSK,
			   7U << CLOCK_GPUCC_GPU_CC_GX_CFG2_GDSCR_DLY_ASSERT_CLAMP_MEM_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_GX_BCR,
			   CLOCK_GPUCC_GPU_CC_GX_BCR_BLK_ARES_BMSK,
			   1U << CLOCK_GPUCC_GPU_CC_GX_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_ACD_BCR,
			   CLOCK_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_BMSK,
			   1U << CLOCK_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_ACD_MISC,
			   CLOCK_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << CLOCK_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(150U);
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_GX_BCR,
			   CLOCK_GPUCC_GPU_CC_GX_BCR_BLK_ARES_BMSK,
			   0U << CLOCK_GPUCC_GPU_CC_GX_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_ACD_BCR,
			   CLOCK_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_BMSK,
			   0U << CLOCK_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_ACD_MISC,
			   CLOCK_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   0U << CLOCK_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   1U << CLOCK_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();
	udelay(1U);

	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   0U << CLOCK_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   0U << CLOCK_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_GX_GDSCR,
			   CLOCK_GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << CLOCK_GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(CLOCK_GPUCC_GPU_CC_GX_CFG_GDSCR) &
		 CLOCK_GPUCC_GPU_CC_GX_CFG_GDSCR_GDSC_POWER_UP_COMPLETE_BMSK) >>
		CLOCK_GPUCC_GPU_CC_GX_CFG_GDSCR_GDSC_POWER_UP_COMPLETE_SHFT) == 0U) {
	}
}

static void clock_disable_gpu_gx_gdsc(void)
{
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_GX_GDSCR,
			   CLOCK_GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << CLOCK_GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_SHFT);

	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_ACD_MISC,
			   CLOCK_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << CLOCK_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(150U);

	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   1U << CLOCK_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);

	while (((mmio_read_32(CLOCK_GPUCC_GPU_CC_GX_CFG_GDSCR) &
		 CLOCK_GPUCC_GPU_CC_GX_CFG_GDSCR_GDSC_POWER_DOWN_COMPLETE_BMSK) >>
		CLOCK_GPUCC_GPU_CC_GX_CFG_GDSCR_GDSC_POWER_DOWN_COMPLETE_SHFT) == 0U) {
	}
}

static void clock_enable_gpu_cx_gdsc(void)
{
	mmio_clrsetbits_32(CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0,
			   CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0_MUX_SEL_PWR_DWN_BMSK,
			   1U << CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0_MUX_SEL_PWR_DWN_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0,
			   CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0_MUX_SEL_STS_BMSK,
			   1U << CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0_MUX_SEL_STS_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0,
			   CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_STS_NRET_BMSK,
			   1U << CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_STS_NRET_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0,
			   CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_STS_RET_BMSK,
			   1U << CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_STS_RET_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0,
			   CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_PWR_DWN_BMSK,
			   0U << CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_PWR_DWN_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0,
			   CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_PWR_DWN_ACK_BMSK,
			   0U << CLOCK_GPU_CX_MISC_GFX_SPARE_CNTL0_CSR_CNTL_PWR_DWN_ACK_SHFT);
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_CX_CFG2_GDSCR,
			   CLOCK_GPUCC_GPU_CC_CX_CFG2_GDSCR_MEM_PWR_DWN_TIMEOUT_BMSK,
			   1U << CLOCK_GPUCC_GPU_CC_CX_CFG2_GDSCR_MEM_PWR_DWN_TIMEOUT_SHFT);

	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_CX_GDSCR,
			   CLOCK_GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << CLOCK_GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(CLOCK_GPUCC_GPU_CC_CX_CFG_GDSCR) &
		 CLOCK_GPUCC_GPU_CC_CX_CFG_GDSCR_GDSC_POWER_UP_COMPLETE_BMSK) >>
		CLOCK_GPUCC_GPU_CC_CX_CFG_GDSCR_GDSC_POWER_UP_COMPLETE_SHFT) == 0U) {
	}
}

static void clock_disable_gpu_cx_gdsc(void)
{
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_CX_GDSCR,
			   CLOCK_GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << CLOCK_GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
}

static void clock_enable_dcd(void)
{
	for (uint32_t i = 0U; i < ARRAY_SIZE(clock_dcd_cfg); i++) {
		mmio_write_32(clock_dcd_cfg[i], 1U);
	}
}

static void clock_disable_boot_imem(void)
{
	clock_enable_cbcr_and_spin(CLOCK_GCC_CFG_NOC_LPASS_CBCR,
				   CLOCK_GCC_CFG_NOC_LPASS_CBCR_CLK_OFF_BMSK);
	clock_enable_cbcr_and_spin(CLOCK_LPASS_LPASS_TOP_CC_LPI_Q6_AXIM_HS_CBCR,
				   CLOCK_LPASS_LPASS_TOP_CC_LPI_Q6_AXIM_HS_CBCR_CLK_OFF_BMSK);

	/*
	 * Clean and invalidate the dcache before disabling BOOT_IMEM, to
	 * ensure there are no stale entries.
	 */
	dcsw_op_all(DCCISW);
	udelay(100U);

	/*
	 * BOOT_IMEM is no longer required by TZ. This is a sticky bit: once 1
	 * is written, only the tcsr_fp_alt_ares reset can return it to 0.
	 */
	mmio_clrsetbits_32(CLOCK_TCSR_BOOT_IMEM_DISABLE,
			   CLOCK_TCSR_BOOT_IMEM_DISABLE_BOOT_IMEM_DISABLE_BMSK,
			   1U << CLOCK_TCSR_BOOT_IMEM_DISABLE_BOOT_IMEM_DISABLE_SHFT);

	/*
	 * Restore the Q6 RCG XO input to XO and disable the AXIS2 clock, which
	 * is no longer required as IMEM.
	 */
	mmio_clrsetbits_32(CLOCK_GCC_LPASS_Q6SS_BOOT_GPLL0_MUXR,
			   CLOCK_GCC_LPASS_Q6SS_BOOT_GPLL0_MUXR_MUX_SEL_BMSK,
			   0U << CLOCK_GCC_LPASS_Q6SS_BOOT_GPLL0_MUXR_MUX_SEL_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_LPASS_AXIS2_CBCR,
			   CLOCK_GCC_LPASS_AXIS2_CBCR_CLK_ENABLE_BMSK,
			   0U << CLOCK_GCC_LPASS_AXIS2_CBCR_CLK_ENABLE_SHFT);

	mmio_write_32(CLOCK_LPASS_LPASS_ALT_RESET_Q6SS, 1U);
	udelay(50U);

	/*
	 * Trigger the SS func_ares signal. Skipping this can result in rogue
	 * transactions from the subsystem under certain conditions (e.g. when
	 * LPI_CX collapses before LPASS PIL has executed).
	 */
	mmio_clrsetbits_32(CLOCK_AOSS_CC_LPASS_RESTART,
			   CLOCK_AOSS_CC_LPASS_RESTART_SS_RESTART_BMSK,
			   1U << CLOCK_AOSS_CC_LPASS_RESTART_SS_RESTART_SHFT);
	udelay(500U);
	mmio_clrsetbits_32(CLOCK_AOSS_CC_LPASS_RESTART,
			   CLOCK_AOSS_CC_LPASS_RESTART_SS_RESTART_BMSK,
			   0U << CLOCK_AOSS_CC_LPASS_RESTART_SS_RESTART_SHFT);
	udelay(500U);

	mmio_write_32(CLOCK_LPASS_LPASS_ALT_RESET_Q6SS, 0U);
}

int clock_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	struct clock_source *gpll0 = &drv_ctxt->bsp->sources[CLOCK_SOURCE_GPLL0];

	/*
	 * Disabling IMEM must occur before enabling the init groups, since it
	 * may involve resets that undo some of the clocks enabled there.
	 */
	clock_disable_boot_imem();

	/* Enable clocks required for init. */
	clock_enable_clock_group(CLOCK_GROUP_INIT);
	clock_enable_clock_group(CLOCK_GROUP_QDSS);

	/*
	 * These two clocks are needed before turning on the CX/GX GDSCs: the
	 * memory sleep staggering logic requires them, and the GDSCs are
	 * required for GPU SMMU access.
	 */
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_CX_GMU_CBCR,
			   CLOCK_GPUCC_GPU_CC_CX_GMU_CBCR_CLK_ENABLE_BMSK,
			   1U << CLOCK_GPUCC_GPU_CC_CX_GMU_CBCR_CLK_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_TZ_VOTE_HUB_CX_INT_CLK,
			   CLOCK_GPUCC_GPU_CC_TZ_VOTE_HUB_CX_INT_CLK_CLK_ENABLE_BMSK,
			   1U << CLOCK_GPUCC_GPU_CC_TZ_VOTE_HUB_CX_INT_CLK_CLK_ENABLE_SHFT);

	if (clock_enable_gpu) {
		clock_enable_gpu_cx_gdsc();
		clock_enable_gpu_gx_gdsc();
	}

	/*
	 * To ensure TZ can always vote for the GDSCs, disallow SW-override
	 * mode, since overriding also disables the voting logic.
	 */
	mmio_clrsetbits_32(CLOCK_MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
			   CLOCK_MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_BMSK,
			   0U << CLOCK_MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_CAM_CC_GDSCR,
			   CLOCK_GCC_CAM_CC_GDSCR_SW_OVERRIDE_BMSK,
			   0U << CLOCK_GCC_CAM_CC_GDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_LPASS_TBU_SGDSCR,
			   CLOCK_GCC_APCS_TZ_LPASS_TBU_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_LPASS_TBU_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_USB30_PRIM_SGDSCR,
			   CLOCK_GCC_APCS_TZ_USB30_PRIM_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_USB30_PRIM_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_MMNOC_SGDSCR,
			   CLOCK_GCC_APCS_TZ_MMNOC_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_MMNOC_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_UFS_PHY_SGDSCR,
			   CLOCK_GCC_APCS_TZ_UFS_PHY_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_UFS_PHY_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_IPA_SGDSCR,
			   CLOCK_GCC_APCS_TZ_IPA_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_IPA_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_MMU3_SGDSCR,
			   CLOCK_GCC_APCS_TZ_MMU3_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_MMU3_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_TURING_0_SGDSCR,
			   CLOCK_GCC_APCS_TZ_TURING_0_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_TURING_0_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_TURING_1_SGDSCR,
			   CLOCK_GCC_APCS_TZ_TURING_1_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_TURING_1_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_PCIE_0_SGDSCR,
			   CLOCK_GCC_APCS_TZ_PCIE_0_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_PCIE_0_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_PCIE_1_SGDSCR,
			   CLOCK_GCC_APCS_TZ_PCIE_1_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_PCIE_1_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_EMAC1_SGDSCR,
			   CLOCK_GCC_APCS_TZ_EMAC1_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_EMAC1_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_EMAC0_SGDSCR,
			   CLOCK_GCC_APCS_TZ_EMAC0_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_EMAC0_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_UFS_CARD_SGDSCR,
			   CLOCK_GCC_APCS_TZ_UFS_CARD_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_UFS_CARD_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_USB30_SEC_SGDSCR,
			   CLOCK_GCC_APCS_TZ_USB30_SEC_SGDSCR_SW_OVERRIDE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_USB30_SEC_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_LPASS_AON_CC_LPASS_AUDIO_HM_RETENTION_VOTE,
			   CLOCK_LPASS_AON_CC_LPASS_AUDIO_HM_RETENTION_VOTE_SECURE_GDSC_HW_CONTROL_OVERRIDE_BMSK,
			   0U << CLOCK_LPASS_AON_CC_LPASS_AUDIO_HM_RETENTION_VOTE_SECURE_GDSC_HW_CONTROL_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_LPASS_AON_CC_LPASS_PDC_HM_RETENTION_VOTE,
			   CLOCK_LPASS_AON_CC_LPASS_PDC_HM_RETENTION_VOTE_SECURE_GDSC_HW_CONTROL_OVERRIDE_BMSK,
			   0U << CLOCK_LPASS_AON_CC_LPASS_PDC_HM_RETENTION_VOTE_SECURE_GDSC_HW_CONTROL_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_LPASS_AON_CC_LPASS_AUDIO_ML_RETENTION_VOTE,
			   CLOCK_LPASS_AON_CC_LPASS_AUDIO_ML_RETENTION_VOTE_SECURE_GDSC_HW_CONTROL_OVERRIDE_BMSK,
			   0U << CLOCK_LPASS_AON_CC_LPASS_AUDIO_ML_RETENTION_VOTE_SECURE_GDSC_HW_CONTROL_OVERRIDE_SHFT);

	if (clock_enable_gpu) {
		mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX,
				   CLOCK_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_BMSK,
				   0U << CLOCK_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_SHFT);
		mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX,
				   CLOCK_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_BMSK,
				   0U << CLOCK_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_SHFT);
	}

	/*
	 * Enable GDSC retention mode, required for SMMU to retain state across
	 * power collapse.
	 */
	mmio_clrsetbits_32(CLOCK_MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
			   CLOCK_MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_CAM_CC_GDSCR,
			   CLOCK_GCC_CAM_CC_GDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_CAM_CC_GDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_LPASS_TBU_SGDSCR,
			   CLOCK_GCC_APCS_TZ_LPASS_TBU_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_LPASS_TBU_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_USB30_PRIM_SGDSCR,
			   CLOCK_GCC_APCS_TZ_USB30_PRIM_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_USB30_PRIM_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_MMNOC_SGDSCR,
			   CLOCK_GCC_APCS_TZ_MMNOC_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_MMNOC_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_UFS_PHY_SGDSCR,
			   CLOCK_GCC_APCS_TZ_UFS_PHY_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_UFS_PHY_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_IPA_SGDSCR,
			   CLOCK_GCC_APCS_TZ_IPA_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_IPA_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_MMU3_SGDSCR,
			   CLOCK_GCC_APCS_TZ_MMU3_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_MMU3_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_TURING_0_SGDSCR,
			   CLOCK_GCC_APCS_TZ_TURING_0_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_TURING_0_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_EMAC1_SGDSCR,
			   CLOCK_GCC_APCS_TZ_EMAC1_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_EMAC1_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_EMAC0_SGDSCR,
			   CLOCK_GCC_APCS_TZ_EMAC0_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_EMAC0_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_UFS_CARD_SGDSCR,
			   CLOCK_GCC_APCS_TZ_UFS_CARD_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_UFS_CARD_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_USB30_SEC_SGDSCR,
			   CLOCK_GCC_APCS_TZ_USB30_SEC_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_USB30_SEC_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_PCIE_0_SGDSCR,
			   CLOCK_GCC_APCS_TZ_PCIE_0_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_PCIE_0_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_PCIE_1_SGDSCR,
			   CLOCK_GCC_APCS_TZ_PCIE_1_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_PCIE_1_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_LPASS_AON_CC_LPASS_AUDIO_HM_RETENTION_VOTE,
			   CLOCK_LPASS_AON_CC_LPASS_AUDIO_HM_RETENTION_VOTE_SECURE_RETENTION_VOTE_BMSK,
			   1U << CLOCK_LPASS_AON_CC_LPASS_AUDIO_HM_RETENTION_VOTE_SECURE_RETENTION_VOTE_SHFT);
	mmio_clrsetbits_32(CLOCK_LPASS_AON_CC_LPASS_PDC_HM_RETENTION_VOTE,
			   CLOCK_LPASS_AON_CC_LPASS_PDC_HM_RETENTION_VOTE_SECURE_RETENTION_VOTE_BMSK,
			   1U << CLOCK_LPASS_AON_CC_LPASS_PDC_HM_RETENTION_VOTE_SECURE_RETENTION_VOTE_SHFT);
	mmio_clrsetbits_32(CLOCK_LPASS_AON_CC_LPASS_AUDIO_ML_RETENTION_VOTE,
			   CLOCK_LPASS_AON_CC_LPASS_AUDIO_ML_RETENTION_VOTE_SECURE_RETENTION_VOTE_BMSK,
			   1U << CLOCK_LPASS_AON_CC_LPASS_AUDIO_ML_RETENTION_VOTE_SECURE_RETENTION_VOTE_SHFT);

	if (clock_enable_gpu) {
		mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX,
				   CLOCK_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_BMSK,
				   1U << CLOCK_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_SHFT);
		mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX,
				   CLOCK_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_BMSK,
				   1U << CLOCK_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_SHFT);
	}

	mmio_clrsetbits_32(CLOCK_GCC_APCS_TZ_TURING_1_SGDSCR,
			   CLOCK_GCC_APCS_TZ_TURING_1_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GCC_APCS_TZ_TURING_1_SGDSCR_RETAIN_FF_ENABLE_SHFT);

	/*
	 * Crashes were observed during CX power collapse without this. Since
	 * DPM (always sourcing from GPLL0) handshakes with the CX ARC during
	 * CXPC, GPLL0 must be kept on.
	 */
	if (clock_enable_source(gpll0) != 0) {
		return -1;
	}

	/* Enable multimedia DCDs. */
	if (clock_dcd_enable) {
		clock_enable_dcd();
	}

	/*
	 * SWA for GPU BIMC error / stage-2 faults: MMU TLB corruption due to
	 * the SMMU SRAM clock turning on before power-on. HW CR QCTDD07258963.
	 */
	mmio_clrsetbits_32(CLOCK_GPUCC_GPU_CC_CX_GMU_CBCR,
			   CLOCK_GPUCC_GPU_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_BMSK,
			   1U << CLOCK_GPUCC_GPU_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_SHFT);

	if (clock_init_cpu(drv_ctxt) != 0) {
		return -1;
	}

	/*
	 * HW Errata QCTDD07226112 (v2+): iris_cc_gcc_clks_off does not go high,
	 * blocking MMCX collapse, because VIDEO_CC_MVS1_CMD_RCGR__ROOT_OFF is
	 * not asserted and the SREG PSCBC controller is stuck.
	 */
	if (qti_sec_get_chip_version() >= CHIPINFO_VERSION(2, 0)) {
		mmio_clrsetbits_32(CLOCK_VCODEC_VIDEO_CC_MVS1_CBCR,
				   CLOCK_VCODEC_VIDEO_CC_MVS1_CBCR_SW_ONLY_EN_BMSK,
				   1U << CLOCK_VCODEC_VIDEO_CC_MVS1_CBCR_SW_ONLY_EN_SHFT);
		mmio_clrsetbits_32(CLOCK_VCODEC_VIDEO_CC_MVS1_CBCR,
				   CLOCK_VCODEC_VIDEO_CC_MVS1_CBCR_CLK_ENABLE_BMSK,
				   0U << CLOCK_VCODEC_VIDEO_CC_MVS1_CBCR_CLK_ENABLE_SHFT);
		mmio_clrsetbits_32(CLOCK_VCODEC_VIDEO_CC_MVS1_CBCR,
				   CLOCK_VCODEC_VIDEO_CC_MVS1_CBCR_FORCE_MEM_PERIPH_ON_BMSK,
				   1U << CLOCK_VCODEC_VIDEO_CC_MVS1_CBCR_FORCE_MEM_PERIPH_ON_SHFT);
		mmio_clrsetbits_32(CLOCK_VCODEC_VIDEO_CC_MVS1_CBCR,
				   CLOCK_VCODEC_VIDEO_CC_MVS1_CBCR_FORCE_MEM_CORE_ON_BMSK,
				   1U << CLOCK_VCODEC_VIDEO_CC_MVS1_CBCR_FORCE_MEM_CORE_ON_SHFT);
	}

	return 0;
}

int clock_post_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

	/* Restore clocks enabled during init to their original state. */
	if (clock_enable_gpu) {
		clock_disable_gpu_gx_gdsc();
		clock_disable_gpu_cx_gdsc();
	}
	clock_disable_clock_group(CLOCK_GROUP_QDSS);
	clock_disable_clock_group(CLOCK_GROUP_INIT);

	/* Re-enable the QDSS STM clock; needed because of a DS-exit issue. */
	mmio_clrsetbits_32(CLOCK_GCC_QDSS_STM_CBCR,
			   CLOCK_GCC_QDSS_STM_CBCR_CLK_ENABLE_BMSK,
			   1U << CLOCK_GCC_QDSS_STM_CBCR_CLK_ENABLE_SHFT);

	/* Program the *PLL_SECURE registers back to secure mode. */
	mmio_write_32(CLOCK_CL0_GOLD_PLL_SECURE, 0x0U);
	mmio_write_32(CLOCK_CL1_GOLD_PLL_SECURE, 0x0U);
	mmio_write_32(CLOCK_CL0_L3_PLL_SECURE, 0x0U);
	mmio_write_32(CLOCK_CL1_L3_PLL_SECURE, 0x0U);

	return 0;
}

int clock_init_cpu(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

	/*
	 * The qcs9075 clock layer performs no CPU clock bring-up here; CPU/OSM
	 * clocking is handled elsewhere. Present so the framework's init path
	 * has a definition to call.
	 *
	 * TODO: add CPU/OSM bring-up if it becomes required for this target.
	 */
	return 0;
}
