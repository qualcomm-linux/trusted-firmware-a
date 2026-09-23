/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-A clock driver back-end for shikra: image init hooks and the
 * GPU GDSC sequences.
 */

#include <errno.h>
#include <stdint.h>

#include <arch_helpers.h>
#include "clock_hwio.h"
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/clock/clock.h>
#include <drivers/qti/clock/clock_cfg.h>
#include <drivers/qti/clock/clock_driver.h>
#include <lib/mmio.h>

/* Bounded retry count for GDSC power-up/down completion polling. */
#define CLOCK_GDSC_POLL_RETRIES	500U

static int clock_enable_gpu_gx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_BCR,
			   GPUCC_GPU_CC_GX_BCR_BLK_ARES_BMSK,
			   1U << GPUCC_GPU_CC_GX_BCR_BLK_ARES_SHFT);
	dsb();
	udelay(1U);
	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_BCR,
			   GPUCC_GPU_CC_GX_BCR_BLK_ARES_BMSK,
			   0U << GPUCC_GPU_CC_GX_BCR_BLK_ARES_SHFT);
	dsb();

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_DOMAIN_MISC,
			   GPUCC_GPU_CC_GX_DOMAIN_MISC_GPU_GX_GMEM_RESET_BMSK,
			   1U << GPUCC_GPU_CC_GX_DOMAIN_MISC_GPU_GX_GMEM_RESET_SHFT);
	dsb();
	udelay(1U);
	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_DOMAIN_MISC,
			   GPUCC_GPU_CC_GX_DOMAIN_MISC_GPU_GX_GMEM_RESET_BMSK,
			   0U << GPUCC_GPU_CC_GX_DOMAIN_MISC_GPU_GX_GMEM_RESET_SHFT);
	dsb();

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_DOMAIN_MISC,
			   GPUCC_GPU_CC_GX_DOMAIN_MISC_GPU_GX_GMEM_CLAMP_IO_BMSK,
			   0U << GPUCC_GPU_CC_GX_DOMAIN_MISC_GPU_GX_GMEM_CLAMP_IO_SHFT);
	dsb();

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_GDSCR,
			   GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_SHFT);
	while ((((mmio_read_32(GPUCC_GPU_CC_GX_GDSCR) &
		  GPUCC_GPU_CC_GX_GDSCR_PWR_ON_BMSK) >>
		 GPUCC_GPU_CC_GX_GDSCR_PWR_ON_SHFT) == 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("Clock: GPU GX GDSC power-up timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static void clock_disable_gpu_gx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_GDSCR,
			   GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_SHFT);
	while ((((mmio_read_32(GPUCC_GPU_CC_GX_GDSCR) &
		  GPUCC_GPU_CC_GX_GDSCR_PWR_ON_BMSK) >>
		 GPUCC_GPU_CC_GX_GDSCR_PWR_ON_SHFT) != 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("Clock: GPU GX GDSC power-down timed out\n");
	}

	mmio_clrsetbits_32(GPUCC_GPU_CC_GX_DOMAIN_MISC,
			   GPUCC_GPU_CC_GX_DOMAIN_MISC_GPU_GX_GMEM_CLAMP_IO_BMSK,
			   1U << GPUCC_GPU_CC_GX_DOMAIN_MISC_GPU_GX_GMEM_CLAMP_IO_SHFT);
}

static int clock_enable_gpu_cx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPUCC_GPU_CC_CX_GDSCR,
			   GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
	while ((((mmio_read_32(GPUCC_GPU_CC_CX_CFG_GDSCR) &
		  GPUCC_GPU_CC_CX_CFG_GDSCR_GDSC_POWER_UP_COMPLETE_BMSK) >>
		 GPUCC_GPU_CC_CX_CFG_GDSCR_GDSC_POWER_UP_COMPLETE_SHFT) == 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("Clock: GPU CX GDSC power-up timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static void clock_disable_gpu_cx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPUCC_GPU_CC_CX_GDSCR,
			   GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
	while ((((mmio_read_32(GPUCC_GPU_CC_CX_CFG_GDSCR) &
		  GPUCC_GPU_CC_CX_CFG_GDSCR_GDSC_POWER_DOWN_COMPLETE_BMSK) >>
		 GPUCC_GPU_CC_CX_CFG_GDSCR_GDSC_POWER_DOWN_COMPLETE_SHFT) == 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("Clock: GPU CX GDSC power-down timed out\n");
	}
}

int clock_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	int ret;

	(void)drv_ctxt;

	/* Enable clocks required for init. */
	ret = clock_group_enable(CLOCK_GROUP_INIT);
	if (ret != 0) {
		return ret;
	}

	ret = clock_group_enable(CLOCK_GROUP_QDSS);
	if (ret != 0) {
		return ret;
	}

	mmio_clrsetbits_32(GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE,
			   GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_QDSS_CFG_AHB_CLK_ENA_BMSK,
			   1U << GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_QDSS_CFG_AHB_CLK_ENA_SHFT);

	if (!chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U)) {
		if (clock_enable_gpu_cx_gdsc() != 0) {
			ERROR("Clock: GPU CX GDSC enable failed\n");
			return -ETIMEDOUT;
		}
		if (clock_enable_gpu_gx_gdsc() != 0) {
			ERROR("Clock: GPU GX GDSC enable failed\n");
			return -ETIMEDOUT;
		}

		/*
		 * To ensure TF-A can always vote for the GDSCs, disallow
		 * SW-override mode, since overriding also disables the voting
		 * logic.
		 */
		mmio_clrsetbits_32(GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX,
				   GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_BMSK,
				   0U << GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_SHFT);
		mmio_clrsetbits_32(GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX,
				   GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_BMSK,
				   0U << GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_SHFT);

		/*
		 * Enable GDSC retention mode, required for SMMU to retain state
		 * across power collapse.
		 */
		mmio_clrsetbits_32(GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX,
				   GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_BMSK,
				   1U << GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_SHFT);
		mmio_clrsetbits_32(GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX,
				   GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_BMSK,
				   1U << GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_SHFT);
	}

	/* Camera. */
	if (!chipinfo_is_part_disabled(CHIPINFO_PART_CAMERA, 0U)) {
		mmio_clrsetbits_32(GCC_TZ_CAMSS_TOP_SGDSCR,
				   GCC_TZ_CAMSS_TOP_SGDSCR_SW_OVERRIDE_BMSK,
				   0U << GCC_TZ_CAMSS_TOP_SGDSCR_SW_OVERRIDE_SHFT);
		mmio_clrsetbits_32(GCC_TZ_CAMSS_TOP_SGDSCR,
				   GCC_TZ_CAMSS_TOP_SGDSCR_RETAIN_FF_ENABLE_BMSK,
				   1U << GCC_TZ_CAMSS_TOP_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	}

	/* Display. */
	if (!chipinfo_is_part_disabled(CHIPINFO_PART_DISPLAY, 0U)) {
		mmio_clrsetbits_32(DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
				   DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_BMSK,
				   0U << DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_SHFT);
		mmio_clrsetbits_32(DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
				   DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_BMSK,
				   1U << DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_SHFT);
	}

	/*
	 * SWA: Apps MMU_TCU is voted by SREG_PSCBC, not a GDSC, so a
	 * CLK_DISABLE from GCC would otherwise collapse the TCU memory along
	 * with the clock; keep it force-retained across VDDmin entry/exit.
	 */
	mmio_clrsetbits_32(GCC_MMU_TCU_CBCR,
			   GCC_MMU_TCU_CBCR_FORCE_MEM_CORE_ON_BMSK,
			   1U << GCC_MMU_TCU_CBCR_FORCE_MEM_CORE_ON_SHFT);

	/* Turn off the memcore for this otherwise-unused clock. */
	mmio_clrsetbits_32(GCC_BOOT_ROM_AHB_CBCR,
			   GCC_BOOT_ROM_AHB_CBCR_FORCE_MEM_CORE_ON_BMSK,
			   0U << GCC_BOOT_ROM_AHB_CBCR_FORCE_MEM_CORE_ON_SHFT);

	mmio_clrsetbits_32(GCC_TZ_USB30_PRIM_SGDSCR,
			   GCC_TZ_USB30_PRIM_SGDSCR_SW_OVERRIDE_BMSK,
			   0U << GCC_TZ_USB30_PRIM_SGDSCR_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(GCC_TZ_USB20_SGDSCR,
			   GCC_TZ_USB20_SGDSCR_SW_OVERRIDE_BMSK,
			   0U << GCC_TZ_USB20_SGDSCR_SW_OVERRIDE_SHFT);

	mmio_clrsetbits_32(GCC_TZ_USB30_PRIM_SGDSCR,
			   GCC_TZ_USB30_PRIM_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_TZ_USB30_PRIM_SGDSCR_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(GCC_TZ_USB20_SGDSCR,
			   GCC_TZ_USB20_SGDSCR_RETAIN_FF_ENABLE_BMSK,
			   1U << GCC_TZ_USB20_SGDSCR_RETAIN_FF_ENABLE_SHFT);

	return 0;
}

int clock_post_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	int ret;

	(void)drv_ctxt;

	/* BOOT_IMEM is no longer required by TF-A. */
	mmio_clrsetbits_32(TCSR_BOOT_IMEM_DISABLE,
			   TCSR_BOOT_IMEM_DISABLE_BOOT_IMEM_DISABLE_BMSK,
			   1U << TCSR_BOOT_IMEM_DISABLE_BOOT_IMEM_DISABLE_SHFT);

	/*
	 * Restore the MSS Q6 RCG XO input to actually be XO and disable the
	 * AXIS2 clock since it's no longer required to be used as IMEM.
	 */
	mmio_clrsetbits_32(GCC_MSS_Q6SS_BOOT_GPLL0_MUXR,
			   GCC_MSS_Q6SS_BOOT_GPLL0_MUXR_MUX_SEL_BMSK,
			   0U << GCC_MSS_Q6SS_BOOT_GPLL0_MUXR_MUX_SEL_SHFT);
	mmio_clrsetbits_32(GCC_MSS_AXIS2_CBCR,
			   GCC_MSS_AXIS2_CBCR_CLK_ENABLE_BMSK,
			   0U << GCC_MSS_AXIS2_CBCR_CLK_ENABLE_SHFT);

	/*
	 * Reset the modem after disabling BOOT_IMEM, since otherwise modem will
	 * generate a bus error on MEMNOC.
	 */
	mmio_write_32(GCC_MSS_RESTART, 1U);
	udelay(200U);
	mmio_write_32(GCC_MSS_RESTART, 0U);

	/* Restore clocks enabled during init to their original state. */
	if (!chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U)) {
		clock_disable_gpu_gx_gdsc();
		clock_disable_gpu_cx_gdsc();
	}

	ret = clock_group_disable(CLOCK_GROUP_QDSS);
	if (ret != 0) {
		return ret;
	}

	return clock_group_disable(CLOCK_GROUP_INIT);
}
