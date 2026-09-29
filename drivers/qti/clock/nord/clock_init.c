/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Clock driver back-end for nord: image init hooks and the per-instance
 * GPU GDSC sequencing.
 *
 * Scoped to clock-group bring-up/release, GDSC sequencing, the QDSS
 * debug-power handshake, boot-IMEM disable and PLL source enable. Generic
 * by-name clock control and DFS are not provided.
 *
 * This port targets a single nord device (dual GPU/display, multimedia
 * present); there is no chip-ID or chip-version variant to detect, so the
 * multimedia and GPU bring-up apply to every boot.
 *
 * Nord's per-instance parts (GPU, DISPLAY, NSP) are gated per physical
 * instance via chipinfo_is_part_disabled(), using part_idx 0-1 (GPU,
 * DISPLAY) or 0-3 (NSP) against the single corresponding chipinfo_part
 * enum value; see clock_cfg.c. The manual GPU_0/GPU_1 GX/CX GDSC
 * sequencing and MDSS/GPU retention setup below are not covered by that
 * declarative gating, so clock_init_image()/clock_post_init_image() check
 * chipinfo_is_part_disabled() directly before touching each instance.
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/clock/clock.h>
#include <drivers/qti/clock/clock_cfg.h>
#include <drivers/qti/clock/clock_driver.h>
#include <lib/mmio.h>

#include "clock_hwio.h"

/* Bounded retry count for GDSC power-up/down completion polling. */
#define CLOCK_GDSC_POLL_RETRIES	500U

static int clock_enable_gpu0_gx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_FF_CBCR,
			   GPU_0_GPUCC_GPU_CC_GX_FF_CBCR_CLK_ENABLE_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_GX_FF_CBCR_CLK_ENABLE_SHFT);
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_ACD_AHB_CBCR,
			   GPU_0_GPUCC_GPU_CC_ACD_AHB_CBCR_CLK_DIS_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_ACD_AHB_CBCR_CLK_DIS_SHFT);
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_ACD_CXO_CBCR,
			   GPU_0_GPUCC_GPU_CC_ACD_CXO_CBCR_CLK_DIS_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_ACD_CXO_CBCR_CLK_DIS_SHFT);
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_BCR,
			   GPU_0_GPUCC_GPU_CC_GX_BCR_BLK_ARES_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_GX_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_ACD_BCR,
			   GPU_0_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_ACD_MISC,
			   GPU_0_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(150U);

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_BCR,
			   GPU_0_GPUCC_GPU_CC_GX_BCR_BLK_ARES_BMSK,
			   0U << GPU_0_GPUCC_GPU_CC_GX_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_ACD_BCR,
			   GPU_0_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_BMSK,
			   0U << GPU_0_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_ACD_MISC,
			   GPU_0_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   0U << GPU_0_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();
	udelay(5U);
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   0U << GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5,
			   GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_RESTORE_FF_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_RESTORE_FF_SHFT);
	dsb();
	udelay(5U);
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5,
			   GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_RESTORE_FF_BMSK,
			   0U << GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_RESTORE_FF_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   0U << GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_GDSCR,
			   GPU_0_GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << GPU_0_GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(GPU_0_GPUCC_GPU_CC_GX_GDSCR) &
		 GPU_0_GPUCC_GPU_CC_GX_GDSCR_PWR_ON_BMSK) == 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("Clock: GPU_0 GX GDSC power-up timed out\n");
		return -1;
	}

	return 0;
}

static void clock_disable_gpu0_gx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_GDSCR,
			   GPU_0_GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(GPU_0_GPUCC_GPU_CC_GX_GDSCR) &
		 GPU_0_GPUCC_GPU_CC_GX_GDSCR_PWR_ON_BMSK) != 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("Clock: GPU_0 GX GDSC power-down timed out\n");
	}

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_BCR,
			   GPU_0_GPUCC_GPU_CC_GX_BCR_BLK_ARES_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_GX_BCR_BLK_ARES_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_ACD_AHB_CBCR,
			   GPU_0_GPUCC_GPU_CC_ACD_AHB_CBCR_CLK_DIS_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_ACD_AHB_CBCR_CLK_DIS_SHFT);
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_ACD_CXO_CBCR,
			   GPU_0_GPUCC_GPU_CC_ACD_CXO_CBCR_CLK_DIS_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_ACD_CXO_CBCR_CLK_DIS_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5,
			   GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_SAVE_FF_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_SAVE_FF_SHFT);
	dsb();
	udelay(5U);
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5,
			   GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_SAVE_FF_BMSK,
			   0U << GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_SAVE_FF_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_ACD_BCR,
			   GPU_0_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_SHFT);
	dsb();
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_ACD_MISC,
			   GPU_0_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(150U);

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);
	dsb();
}

static int clock_enable_gpu0_cx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_CX_GMU_CBCR,
			   GPU_0_GPUCC_GPU_CC_CX_GMU_CBCR_CLK_ENABLE_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_CX_GMU_CBCR_CLK_ENABLE_SHFT);
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_HUB_CX_INT_CBCR,
			   GPU_0_GPUCC_GPU_CC_HUB_CX_INT_CBCR_CLK_ENABLE_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_HUB_CX_INT_CBCR_CLK_ENABLE_SHFT);

	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_CX_GDSCR,
			   GPU_0_GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << GPU_0_GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(GPU_0_GPUCC_GPU_CC_CX_GDSCR) &
		 GPU_0_GPUCC_GPU_CC_CX_GDSCR_PWR_ON_BMSK) == 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("Clock: GPU_0 CX GDSC power-up timed out\n");
		return -1;
	}

	return 0;
}

static void clock_disable_gpu0_cx_gdsc(void)
{
	mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_CX_GDSCR,
			   GPU_0_GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << GPU_0_GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
}

static int clock_enable_gpu1_gx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_FF_CBCR,
			   GPU_1_GPUCC_GPU_2_CC_GX_FF_CBCR_CLK_ENABLE_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_GX_FF_CBCR_CLK_ENABLE_SHFT);
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_ACD_AHB_CBCR,
			   GPU_1_GPUCC_GPU_2_CC_ACD_AHB_CBCR_CLK_DIS_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_ACD_AHB_CBCR_CLK_DIS_SHFT);
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_ACD_CXO_CBCR,
			   GPU_1_GPUCC_GPU_2_CC_ACD_CXO_CBCR_CLK_DIS_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_ACD_CXO_CBCR_CLK_DIS_SHFT);
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_BCR,
			   GPU_1_GPUCC_GPU_2_CC_GX_BCR_BLK_ARES_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_GX_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_ACD_BCR,
			   GPU_1_GPUCC_GPU_2_CC_ACD_BCR_BLK_ARES_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_ACD_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_ACD_MISC,
			   GPU_1_GPUCC_GPU_2_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(150U);

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_BCR,
			   GPU_1_GPUCC_GPU_2_CC_GX_BCR_BLK_ARES_BMSK,
			   0U << GPU_1_GPUCC_GPU_2_CC_GX_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_ACD_BCR,
			   GPU_1_GPUCC_GPU_2_CC_ACD_BCR_BLK_ARES_BMSK,
			   0U << GPU_1_GPUCC_GPU_2_CC_ACD_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_ACD_MISC,
			   GPU_1_GPUCC_GPU_2_CC_ACD_MISC_IROOT_ARES_BMSK,
			   0U << GPU_1_GPUCC_GPU_2_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3,
			   GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();
	udelay(5U);
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3,
			   GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   0U << GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5,
			   GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_RESTORE_FF_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_RESTORE_FF_SHFT);
	dsb();
	udelay(5U);
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5,
			   GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_RESTORE_FF_BMSK,
			   0U << GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_RESTORE_FF_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3,
			   GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   0U << GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_GDSCR,
			   GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(GPU_1_GPUCC_GPU_2_CC_GX_GDSCR) &
		 GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_PWR_ON_BMSK) == 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("Clock: GPU_1 GX GDSC power-up timed out\n");
		return -1;
	}

	return 0;
}

static void clock_disable_gpu1_gx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_GDSCR,
			   GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(GPU_1_GPUCC_GPU_2_CC_GX_GDSCR) &
		 GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_PWR_ON_BMSK) != 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("Clock: GPU_1 GX GDSC power-down timed out\n");
	}

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_BCR,
			   GPU_1_GPUCC_GPU_2_CC_GX_BCR_BLK_ARES_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_GX_BCR_BLK_ARES_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_ACD_AHB_CBCR,
			   GPU_1_GPUCC_GPU_2_CC_ACD_AHB_CBCR_CLK_DIS_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_ACD_AHB_CBCR_CLK_DIS_SHFT);
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_ACD_CXO_CBCR,
			   GPU_1_GPUCC_GPU_2_CC_ACD_CXO_CBCR_CLK_DIS_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_ACD_CXO_CBCR_CLK_DIS_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5,
			   GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_SAVE_FF_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_SAVE_FF_SHFT);
	dsb();
	udelay(5U);
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5,
			   GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_SAVE_FF_BMSK,
			   0U << GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_SAVE_FF_SHFT);
	dsb();

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_ACD_BCR,
			   GPU_1_GPUCC_GPU_2_CC_ACD_BCR_BLK_ARES_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_ACD_BCR_BLK_ARES_SHFT);
	dsb();
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_ACD_MISC,
			   GPU_1_GPUCC_GPU_2_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(150U);

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3,
			   GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3,
			   GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);
	dsb();
}

static int clock_enable_gpu1_cx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_CX_GMU_CBCR,
			   GPU_1_GPUCC_GPU_2_CC_CX_GMU_CBCR_CLK_ENABLE_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_CX_GMU_CBCR_CLK_ENABLE_SHFT);
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_HUB_CX_INT_CBCR,
			   GPU_1_GPUCC_GPU_2_CC_HUB_CX_INT_CBCR_CLK_ENABLE_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_HUB_CX_INT_CBCR_CLK_ENABLE_SHFT);

	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_CX_GDSCR,
			   GPU_1_GPUCC_GPU_2_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << GPU_1_GPUCC_GPU_2_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(GPU_1_GPUCC_GPU_2_CC_CX_GDSCR) &
		 GPU_1_GPUCC_GPU_2_CC_CX_GDSCR_PWR_ON_BMSK) == 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("Clock: GPU_1 CX GDSC power-up timed out\n");
		return -1;
	}

	return 0;
}

static void clock_disable_gpu1_cx_gdsc(void)
{
	mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_CX_GDSCR,
			   GPU_1_GPUCC_GPU_2_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << GPU_1_GPUCC_GPU_2_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
}

/*
 * GCC_DEBUG_EN is a CDBGPWRUPREQ/CDBGPWRUPACK debug-power handshake: write
 * CDBGPWRUPREQ=1 to request, then wait for hardware to set CDBGPWRUPACK=1
 * on grant. This is the opposite polarity of the standard CBCR CLK_OFF
 * branch-clock semantics, so it cannot be driven through the generic
 * clock-group path.
 */
static int clock_wait_qdss_debug_en_ack(uint32_t addr, uint32_t req_bmsk,
					uint32_t ack_bmsk, const char *name)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	while (((mmio_read_32(addr) & ack_bmsk) == 0U) && (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		uint32_t raw = mmio_read_32(addr);

		ERROR("Clock: %s QDSS power-up timed out raw=0x%08x req=%u ack=%u\n",
		      name, raw, (raw & req_bmsk) != 0U,
		      (raw & ack_bmsk) != 0U);
		return -1;
	}

	return 0;
}

static int clock_enable_qdss_debug_en(void)
{
	mmio_clrsetbits_32(GCC_DEBUG_EN, GCC_DEBUG_EN_CDBGPWRUPREQ_BMSK,
			   1U << GCC_DEBUG_EN_CDBGPWRUPREQ_SHFT);

	if (clock_wait_qdss_debug_en_ack(GCC_DEBUG_EN,
					 GCC_DEBUG_EN_CDBGPWRUPREQ_BMSK,
					 GCC_DEBUG_EN_CDBGPWRUPACK_BMSK,
					 "GCC_DEBUG_EN") != 0) {
		return -1;
	}

	return 0;
}

static void clock_disable_qdss_debug_en(void)
{
	mmio_clrsetbits_32(GCC_DEBUG_EN, GCC_DEBUG_EN_CDBGPWRUPREQ_BMSK,
			   0U << GCC_DEBUG_EN_CDBGPWRUPREQ_SHFT);
}

/*
 * Register-state dump used only when clock_init_image() fails: reads back
 * the real HW status bit for every clock/GDSC/QDSS-accessor/PLL touched
 * above, so a boot-log postmortem is possible without physical register
 * access.
 */
static void clock_dump_clk_desc_status(const char *label,
				       struct clock_desc *clocks)
{
	struct clock_desc *clock;

	if (clocks == NULL) {
		return;
	}

	for (clock = clocks; clock->cbcr_addr != 0U; clock++) {
		uint32_t val = mmio_read_32(clock->cbcr_addr);

		NOTICE("Clock: %s CBCR 0x%lx enable=%u hw_ctl=%u CLK_OFF=%u raw=0x%08x\n",
		       label, (unsigned long)clock->cbcr_addr,
		       (val & HAL_CLK_BRANCH_CTRL_REG_CLK_ENABLE_FMSK) != 0U,
		       (val & HAL_CLK_BRANCH_CTRL_REG_CLK_HW_CTL_FMSK) != 0U,
		       (val & HAL_CLK_BRANCH_CTRL_REG_CLK_OFF_FMSK) != 0U,
		       val);

		if (clock->vote_reg.addr != 0U) {
			uint32_t vote = mmio_read_32(clock->vote_reg.addr);

			NOTICE("Clock: %s CBCR 0x%lx vote_reg=0x%lx voted=%u raw=0x%08x\n",
			       label, (unsigned long)clock->cbcr_addr,
			       (unsigned long)clock->vote_reg.addr,
			       (vote & clock->vote_reg.mask) != 0U, vote);
		}
	}
}

static void clock_dump_power_domain_status(struct clock_power_domain_desc *pds)
{
	struct clock_power_domain_desc *pd;

	if (pds == NULL) {
		return;
	}

	for (pd = pds; (pd->gdscr_addr != 0U) || (pd->vote_reg.addr != 0U);
	     pd++) {
		uint32_t val;

		if (pd->vote_reg.addr != 0U) {
			val = mmio_read_32(pd->vote_reg.addr);
			NOTICE("Clock: GDSC vote 0x%lx voted=%u raw=0x%08x\n",
			       (unsigned long)pd->vote_reg.addr,
			       (val & pd->vote_reg.mask) != 0U, val);
			continue;
		}

		val = mmio_read_32(pd->gdscr_addr);
		NOTICE("Clock: GDSCR 0x%lx PWR_ON=%u raw=0x%08x\n",
		       (unsigned long)pd->gdscr_addr,
		       (val & HAL_CLK_GDSCR_PWR_ON_FMSK) != 0U, val);
	}
}

/* GPU GX/CX GDSCRs are sequenced manually above, not via .pwr_domains. */
static void clock_dump_gpu_gdscr_status(void)
{
	uint32_t val;

	val = mmio_read_32(GPU_0_GPUCC_GPU_CC_GX_GDSCR);
	NOTICE("Clock: GPU_0 GX GDSCR 0x%x PWR_ON=%u raw=0x%08x\n",
	       GPU_0_GPUCC_GPU_CC_GX_GDSCR,
	       (val & GPU_0_GPUCC_GPU_CC_GX_GDSCR_PWR_ON_BMSK) != 0U,
	       val);

	val = mmio_read_32(GPU_0_GPUCC_GPU_CC_CX_GDSCR);
	NOTICE("Clock: GPU_0 CX GDSCR 0x%x PWR_ON=%u raw=0x%08x\n",
	       GPU_0_GPUCC_GPU_CC_CX_GDSCR,
	       (val & GPU_0_GPUCC_GPU_CC_CX_GDSCR_PWR_ON_BMSK) != 0U,
	       val);

	val = mmio_read_32(GPU_1_GPUCC_GPU_2_CC_GX_GDSCR);
	NOTICE("Clock: GPU_1 GX GDSCR 0x%x PWR_ON=%u raw=0x%08x\n",
	       GPU_1_GPUCC_GPU_2_CC_GX_GDSCR,
	       (val & GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_PWR_ON_BMSK) != 0U,
	       val);

	val = mmio_read_32(GPU_1_GPUCC_GPU_2_CC_CX_GDSCR);
	NOTICE("Clock: GPU_1 CX GDSCR 0x%x PWR_ON=%u raw=0x%08x\n",
	       GPU_1_GPUCC_GPU_2_CC_CX_GDSCR,
	       (val & GPU_1_GPUCC_GPU_2_CC_CX_GDSCR_PWR_ON_BMSK) != 0U,
	       val);
}

static void clock_dump_qdss_status(void)
{
	uint32_t val;

	val = mmio_read_32(GCC_DEBUG_EN);
	NOTICE("Clock: GCC_DEBUG_EN 0x%x CDBGPWRUPACK=%u raw=0x%08x\n",
	       GCC_DEBUG_EN,
	       (val & GCC_DEBUG_EN_CDBGPWRUPACK_BMSK) != 0U, val);
}

static void clock_dump_source_status(const char *label,
				     struct clock_source *source)
{
	uint32_t val;

	if (source->hw_source.mode_addr == 0U) {
		return;
	}

	val = mmio_read_32(source->hw_source.mode_addr);
	NOTICE("Clock: %s PLL_MODE 0x%lx PLL_LOCK_DET=%u raw=0x%08x\n", label,
	       (unsigned long)source->hw_source.mode_addr,
	       (val & HAL_CLK_PLL_MODE_PLL_LOCK_DET_BMSK) != 0U, val);
}

static void clock_dump_status(struct clock_drv_ctxt *drv_ctxt)
{
	struct clock_group *init_group =
		&drv_ctxt->cfg->clock_groups[CLOCK_GROUP_INIT];

	clock_dump_clk_desc_status("INIT", init_group->clks);
	clock_dump_clk_desc_status("INIT-ACCESS", init_group->access_clks);
	clock_dump_power_domain_status(init_group->pwr_domains);
	clock_dump_gpu_gdscr_status();

	clock_dump_qdss_status();

	clock_dump_source_status("GPLL0",
				 &drv_ctxt->cfg->sources[CLOCK_SOURCE_GPLL0]);
	clock_dump_source_status("NE_GCC_GPLL0",
				 &drv_ctxt->cfg->sources[CLOCK_SOURCE_NE_GCC_GPLL0]);
	clock_dump_source_status("NW_GCC_GPLL0",
				 &drv_ctxt->cfg->sources[CLOCK_SOURCE_NW_GCC_GPLL0]);
	clock_dump_source_status("SE_GCC_GPLL0",
				 &drv_ctxt->cfg->sources[CLOCK_SOURCE_SE_GCC_GPLL0]);
}

int clock_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	struct clock_source *gpll0 = &drv_ctxt->cfg->sources[CLOCK_SOURCE_GPLL0];
	struct clock_source *ne_gpll0 =
		&drv_ctxt->cfg->sources[CLOCK_SOURCE_NE_GCC_GPLL0];
	struct clock_source *nw_gpll0 =
		&drv_ctxt->cfg->sources[CLOCK_SOURCE_NW_GCC_GPLL0];
	struct clock_source *se_gpll0 =
		&drv_ctxt->cfg->sources[CLOCK_SOURCE_SE_GCC_GPLL0];
	bool gpu0_present = !chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U);
	bool gpu1_present = !chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 1U);
	bool disp0_present = !chipinfo_is_part_disabled(CHIPINFO_PART_DISPLAY, 0U);
	bool disp1_present = !chipinfo_is_part_disabled(CHIPINFO_PART_DISPLAY, 1U);

	/* Enable clocks required for init. */
	if (clock_group_enable(CLOCK_GROUP_INIT) != 0) {
		/* Dump status now: clock_dump_status() below is otherwise
		 * unreachable on this path, hiding the raw register state
		 * behind the timeout that just occurred. */
		clock_dump_status(drv_ctxt);
		return -1;
	}

	if (clock_enable_qdss_debug_en() != 0) {
		clock_dump_status(drv_ctxt);
		return -1;
	}

	/*
	 * Disallow SW-override mode and enable retention on the MDSS
	 * GDSCs so they always remain vote-controlled, and so SMMU state
	 * is retained across power collapse. Skipped per instance when
	 * chipinfo reports that display isn't present on this SKU.
	 */
	if (disp0_present) {
		mmio_clrsetbits_32(MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
				   MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_BMSK,
				   0U << MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_SHFT);
		mmio_clrsetbits_32(MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
				   MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_BMSK,
				   1U << MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_SHFT);
	}

	if (disp1_present) {
		mmio_clrsetbits_32(MDSS_1_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
				   MDSS_1_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_BMSK,
				   0U << MDSS_1_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_SHFT);
		mmio_clrsetbits_32(MDSS_1_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
				   MDSS_1_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_BMSK,
				   1U << MDSS_1_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_SHFT);
	}

	/*
	 * GPU GX/CX GDSC sequencing is manual (not part of .pwr_domains),
	 * so gate each instance explicitly against chipinfo -- a GPU
	 * instance that isn't present on this SKU would otherwise time out
	 * polling a GDSCR that never asserts PWR_ON and fail the whole
	 * clock_init_image() call.
	 */
	if (gpu0_present && (clock_enable_gpu0_cx_gdsc() != 0)) {
		return -1;
	}
	if (gpu1_present && (clock_enable_gpu1_cx_gdsc() != 0)) {
		return -1;
	}
	if (gpu0_present && (clock_enable_gpu0_gx_gdsc() != 0)) {
		return -1;
	}
	if (gpu1_present && (clock_enable_gpu1_gx_gdsc() != 0)) {
		return -1;
	}

	if (gpu0_present) {
		mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX,
				   GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_BMSK,
				   0U << GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_SHFT);
		mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX,
				   GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_BMSK,
				   0U << GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_SHFT);
		mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX,
				   GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_BMSK,
				   1U << GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_SHFT);
		mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX,
				   GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_BMSK,
				   1U << GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_SHFT);
		mmio_write_32(GPU_0_GPUCC_GPU_CC_RCG_SRC_ACTIVE_CTL, 0U);
		/*
		 * Workaround for GPU BIMC error / stage-2 faults: MMU TLB
		 * corruption due to the SMMU SRAM clock turning on before
		 * power-on.
		 */
		mmio_clrsetbits_32(GPU_0_GPUCC_GPU_CC_CX_GMU_CBCR,
				   GPU_0_GPUCC_GPU_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_BMSK,
				   1U << GPU_0_GPUCC_GPU_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_SHFT);
	}

	if (gpu1_present) {
		mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_CX,
				   GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_BMSK,
				   0U << GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_SHFT);
		mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_GX,
				   GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_BMSK,
				   0U << GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_SHFT);
		mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_CX,
				   GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_BMSK,
				   1U << GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_SHFT);
		mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_GX,
				   GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_BMSK,
				   1U << GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_SHFT);
		mmio_write_32(GPU_1_GPUCC_GPU_2_CC_RCG_SRC_ACTIVE_CTL, 0U);
		mmio_clrsetbits_32(GPU_1_GPUCC_GPU_2_CC_CX_GMU_CBCR,
				   GPU_1_GPUCC_GPU_2_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_BMSK,
				   1U << GPU_1_GPUCC_GPU_2_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_SHFT);
	}

	/*
	 * Crashes were observed during CX power collapse without this. Since
	 * DPM (always sourcing from GPLL0) handshakes with the CX ARC during
	 * CXPC, GPLL0 must be kept on -- for all four independent GCC domains.
	 */
	if ((clock_source_enable(gpll0) != 0) ||
	    (clock_source_enable(ne_gpll0) != 0) ||
	    (clock_source_enable(nw_gpll0) != 0) ||
	    (clock_source_enable(se_gpll0) != 0)) {
		return -1;
	}

	/*
	 * BOOT_IMEM is no longer required once boot has completed. This
	 * is a sticky bit: once 1 is written, only the tcsr_fp_alt_ares
	 * reset can return it to 0.
	 */
	mmio_clrsetbits_32(TCSR_BOOT_IMEM_DISABLE,
			   TCSR_BOOT_IMEM_DISABLE_BOOT_IMEM_DISABLE_BMSK,
			   1U << TCSR_BOOT_IMEM_DISABLE_BOOT_IMEM_DISABLE_SHFT);

	return 0;
}

int clock_post_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	bool gpu0_present = !chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U);
	bool gpu1_present = !chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 1U);

	(void)drv_ctxt;

	/* Restore clocks enabled during init to their original state. */
	if (gpu0_present) {
		clock_disable_gpu0_gx_gdsc();
	}
	if (gpu1_present) {
		clock_disable_gpu1_gx_gdsc();
	}
	if (gpu0_present) {
		clock_disable_gpu0_cx_gdsc();
	}
	if (gpu1_present) {
		clock_disable_gpu1_cx_gdsc();
	}
	clock_disable_qdss_debug_en();
	clock_group_disable(CLOCK_GROUP_INIT);

	return 0;
}
