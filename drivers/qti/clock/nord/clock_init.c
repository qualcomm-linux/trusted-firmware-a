/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Clock driver back-end for nord: image init hooks and the per-instance
 * GPU GDSC sequencing.
 *
 * Scoped to clock-group bring-up/release, GDSC sequencing, boot-IMEM
 * disable and PLL source enable. Generic by-name clock control and DFS are
 * not provided.
 *
 * This port targets a single nord device (dual GPU/display, multimedia
 * present); there is no chip-ID or chip-version variant to detect, so the
 * multimedia and GPU bring-up apply to every boot.
 *
 * Nord's per-instance parts (GPU_0/GPU_1, DISPLAY_0/DISPLAY_1, NSP0-3) are
 * all treated as always present, since the chipinfo driver has no
 * part-disable query.
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/clock/clock.h>
#include <drivers/qti/clock/clock_bsp.h>
#include <drivers/qti/clock/clock_driver.h>
#include <lib/mmio.h>

#include "clock_hwio.h"

/* Bounded retry count for GDSC power-up/down completion polling. */
#define CLOCK_GDSC_POLL_RETRIES	500U

static int clock_enable_gpu0_gx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_FF_CBCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_FF_CBCR_CLK_ENABLE_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_FF_CBCR_CLK_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_ACD_AHB_CBCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_ACD_AHB_CBCR_CLK_DIS_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_ACD_AHB_CBCR_CLK_DIS_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_ACD_CXO_CBCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_ACD_CXO_CBCR_CLK_DIS_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_ACD_CXO_CBCR_CLK_DIS_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_BCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_BCR_BLK_ARES_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_ACD_BCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_ACD_MISC,
			   CLOCK_GPU_0_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(150U);

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_BCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_BCR_BLK_ARES_BMSK,
			   0U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_ACD_BCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_BMSK,
			   0U << CLOCK_GPU_0_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_ACD_MISC,
			   CLOCK_GPU_0_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   0U << CLOCK_GPU_0_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();
	udelay(5U);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   0U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_RESTORE_FF_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_RESTORE_FF_SHFT);
	dsb();
	udelay(5U);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_RESTORE_FF_BMSK,
			   0U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_RESTORE_FF_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   0U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_GDSCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_GDSCR) &
		 CLOCK_GPU_0_GPUCC_GPU_CC_GX_GDSCR_PWR_ON_BMSK) == 0U) &&
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

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_GDSCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_GDSCR) &
		 CLOCK_GPU_0_GPUCC_GPU_CC_GX_GDSCR_PWR_ON_BMSK) != 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("Clock: GPU_0 GX GDSC power-down timed out\n");
	}

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_BCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_BCR_BLK_ARES_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_BCR_BLK_ARES_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_ACD_AHB_CBCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_ACD_AHB_CBCR_CLK_DIS_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_ACD_AHB_CBCR_CLK_DIS_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_ACD_CXO_CBCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_ACD_CXO_CBCR_CLK_DIS_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_ACD_CXO_CBCR_CLK_DIS_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_SAVE_FF_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_SAVE_FF_SHFT);
	dsb();
	udelay(5U);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_SAVE_FF_BMSK,
			   0U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC5_SAVE_FF_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_ACD_BCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_ACD_BCR_BLK_ARES_SHFT);
	dsb();
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_ACD_MISC,
			   CLOCK_GPU_0_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(150U);

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);
	dsb();
}

static int clock_enable_gpu0_cx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_CX_GMU_CBCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_CX_GMU_CBCR_CLK_ENABLE_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_CX_GMU_CBCR_CLK_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_HUB_CX_INT_CBCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_HUB_CX_INT_CBCR_CLK_ENABLE_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_HUB_CX_INT_CBCR_CLK_ENABLE_SHFT);

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_CX_GDSCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << CLOCK_GPU_0_GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(CLOCK_GPU_0_GPUCC_GPU_CC_CX_GDSCR) &
		 CLOCK_GPU_0_GPUCC_GPU_CC_CX_GDSCR_PWR_ON_BMSK) == 0U) &&
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
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_CX_GDSCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
}

static int clock_enable_gpu1_gx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_FF_CBCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_FF_CBCR_CLK_ENABLE_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_FF_CBCR_CLK_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_AHB_CBCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_AHB_CBCR_CLK_DIS_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_AHB_CBCR_CLK_DIS_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_CXO_CBCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_CXO_CBCR_CLK_DIS_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_CXO_CBCR_CLK_DIS_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_BCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_BCR_BLK_ARES_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_BCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_BCR_BLK_ARES_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_MISC,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(150U);

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_BCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_BCR_BLK_ARES_BMSK,
			   0U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_BCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_BCR_BLK_ARES_BMSK,
			   0U << CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_BCR_BLK_ARES_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_MISC,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_MISC_IROOT_ARES_BMSK,
			   0U << CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();
	udelay(5U);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   0U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_RESTORE_FF_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_RESTORE_FF_SHFT);
	dsb();
	udelay(5U);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_RESTORE_FF_BMSK,
			   0U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_RESTORE_FF_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   0U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_GDSCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_GDSCR) &
		 CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_PWR_ON_BMSK) == 0U) &&
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

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_GDSCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_GDSCR) &
		 CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_GDSCR_PWR_ON_BMSK) != 0U) &&
	       (--retry > 0U)) {
		udelay(1U);
	}

	if (retry == 0U) {
		WARN("Clock: GPU_1 GX GDSC power-down timed out\n");
	}

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_BCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_BCR_BLK_ARES_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_BCR_BLK_ARES_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_AHB_CBCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_AHB_CBCR_CLK_DIS_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_AHB_CBCR_CLK_DIS_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_CXO_CBCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_CXO_CBCR_CLK_DIS_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_CXO_CBCR_CLK_DIS_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_SAVE_FF_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_SAVE_FF_SHFT);
	dsb();
	udelay(5U);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_SAVE_FF_BMSK,
			   0U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC5_SAVE_FF_SHFT);
	dsb();

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_BCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_BCR_BLK_ARES_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_BCR_BLK_ARES_SHFT);
	dsb();
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_MISC,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_MISC_IROOT_ARES_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_ACD_MISC_IROOT_ARES_SHFT);
	dsb();
	udelay(150U);

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_RESET_SHFT);
	dsb();
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_GX_DOMAIN_MISC3_GPU_GX_RAIL_CLAMP_IO_SHFT);
	dsb();
}

static int clock_enable_gpu1_cx_gdsc(void)
{
	uint32_t retry = CLOCK_GDSC_POLL_RETRIES;

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GMU_CBCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GMU_CBCR_CLK_ENABLE_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GMU_CBCR_CLK_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_HUB_CX_INT_CBCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_HUB_CX_INT_CBCR_CLK_ENABLE_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_HUB_CX_INT_CBCR_CLK_ENABLE_SHFT);

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GDSCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   0U << CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
	while (((mmio_read_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GDSCR) &
		 CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GDSCR_PWR_ON_BMSK) == 0U) &&
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
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GDSCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GDSCR_SW_COLLAPSE_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GDSCR_SW_COLLAPSE_SHFT);
}

int clock_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	struct clock_source *gpll0 = &drv_ctxt->bsp->sources[CLOCK_SOURCE_GPLL0];
	struct clock_source *ne_gpll0 =
		&drv_ctxt->bsp->sources[CLOCK_SOURCE_NE_GCC_GPLL0];
	struct clock_source *nw_gpll0 =
		&drv_ctxt->bsp->sources[CLOCK_SOURCE_NW_GCC_GPLL0];
	struct clock_source *se_gpll0 =
		&drv_ctxt->bsp->sources[CLOCK_SOURCE_SE_GCC_GPLL0];

	/* Enable clocks required for init. */
	if (clock_enable_clock_group(CLOCK_GROUP_INIT) != 0) {
		return -1;
	}
	if (clock_enable_clock_group(CLOCK_GROUP_QDSS) != 0) {
		return -1;
	}

	/*
	 * Disallow SW-override mode and enable retention on the MDSS
	 * GDSCs so they always remain vote-controlled, and so SMMU state
	 * is retained across power collapse.
	 */
	mmio_clrsetbits_32(CLOCK_MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
			   CLOCK_MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_BMSK,
			   0U << CLOCK_MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
			   CLOCK_MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_MDSS_0_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_SHFT);

	mmio_clrsetbits_32(CLOCK_MDSS_1_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
			   CLOCK_MDSS_1_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_BMSK,
			   0U << CLOCK_MDSS_1_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_MDSS_1_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL,
			   CLOCK_MDSS_1_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_MDSS_1_DISP_CC_TZ_MDSS_CORE_GDSC_SW_CTL_RETAIN_FF_ENABLE_SHFT);

	if ((clock_enable_gpu0_cx_gdsc() != 0) ||
	    (clock_enable_gpu1_cx_gdsc() != 0) ||
	    (clock_enable_gpu0_gx_gdsc() != 0) ||
	    (clock_enable_gpu1_gx_gdsc() != 0)) {
		return -1;
	}

	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX,
			   CLOCK_GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_BMSK,
			   0U << CLOCK_GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX,
			   CLOCK_GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_BMSK,
			   0U << CLOCK_GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX,
			   CLOCK_GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX,
			   CLOCK_GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_SHFT);
	mmio_write_32(CLOCK_GPU_0_GPUCC_GPU_CC_RCG_SRC_ACTIVE_CTL, 0U);
	/*
	 * Workaround for GPU BIMC error / stage-2 faults: MMU TLB
	 * corruption due to the SMMU SRAM clock turning on before
	 * power-on.
	 */
	mmio_clrsetbits_32(CLOCK_GPU_0_GPUCC_GPU_CC_CX_GMU_CBCR,
			   CLOCK_GPU_0_GPUCC_GPU_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_BMSK,
			   1U << CLOCK_GPU_0_GPUCC_GPU_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_SHFT);

	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_CX,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_BMSK,
			   0U << CLOCK_GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_CX_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_GX,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_BMSK,
			   0U << CLOCK_GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_GX_SW_OVERRIDE_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_CX,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_CX_RETAIN_FF_ENABLE_SHFT);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_GX,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_TZ_GDSC_CTRL_GPU_GX_RETAIN_FF_ENABLE_SHFT);
	mmio_write_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_RCG_SRC_ACTIVE_CTL, 0U);
	mmio_clrsetbits_32(CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GMU_CBCR,
			   CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_BMSK,
			   1U << CLOCK_GPU_1_GPUCC_GPU_2_CC_CX_GMU_CBCR_FORCE_MEM_PERIPH_ON_SHFT);

	/*
	 * Crashes were observed during CX power collapse without this. Since
	 * DPM (always sourcing from GPLL0) handshakes with the CX ARC during
	 * CXPC, GPLL0 must be kept on -- for all four independent GCC domains.
	 */
	if ((clock_enable_source(gpll0) != 0) ||
	    (clock_enable_source(ne_gpll0) != 0) ||
	    (clock_enable_source(nw_gpll0) != 0) ||
	    (clock_enable_source(se_gpll0) != 0)) {
		return -1;
	}

	/*
	 * BOOT_IMEM is no longer required once boot has completed. This
	 * is a sticky bit: once 1 is written, only the tcsr_fp_alt_ares
	 * reset can return it to 0.
	 */
	mmio_clrsetbits_32(CLOCK_TCSR_BOOT_IMEM_DISABLE,
			   CLOCK_TCSR_BOOT_IMEM_DISABLE_BOOT_IMEM_DISABLE_BMSK,
			   1U << CLOCK_TCSR_BOOT_IMEM_DISABLE_BOOT_IMEM_DISABLE_SHFT);

	return 0;
}

int clock_post_init_image(struct clock_drv_ctxt *drv_ctxt)
{
	(void)drv_ctxt;

	/* Restore clocks enabled during init to their original state. */
	clock_disable_gpu0_gx_gdsc();
	clock_disable_gpu1_gx_gdsc();
	clock_disable_gpu0_cx_gdsc();
	clock_disable_gpu1_cx_gdsc();
	clock_disable_clock_group(CLOCK_GROUP_QDSS);
	clock_disable_clock_group(CLOCK_GROUP_INIT);

	return 0;
}
