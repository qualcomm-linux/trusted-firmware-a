/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_HWIO_H
#define SYSDBG_HWIO_H

/*
 * Nord AOSS_CC_RST_CTL_REG block (verified against IP Catalog, SA8797P
 * NordAU v2 / nordschleife_2.0). Base address matches hawi/hamoa/kaanapali
 * (0xc2f0000) - the block base is stable across this SoC lineage - but the
 * AOSS_CC_RESET_STATUS bit layout below is NOT: nord's SRST/PROC_HALT_CTI/
 * PMIC_ABNORMAL_RESIN bit positions match hamoa (bits 3/4/6), not
 * hawi/kaanapali (bits 2/3/4). Do not copy this table onto another chip
 * without re-verifying against IP Catalog for that chip.
 */
#define AOSS_CC_RST_CTL_REG_BASE		0xc2f0000

/*
 * AOSS_CC_RESET_DEBUG_ENABLE (+0x0): gates whether reset-debug is enabled
 * at all. Bit layout identical to hawi/hamoa/kaanapali.
 */
#define HWIO_AOSS_CC_RESET_DEBUG_ENABLE_ADDR			(AOSS_CC_RST_CTL_REG_BASE + 0x0)
#define HWIO_AOSS_CC_RESET_DEBUG_ENABLE_PMIC_RESIN_RESET_DEBUG_EN_SHFT		4
#define HWIO_AOSS_CC_RESET_DEBUG_ENABLE_SECURE_WDOG_RESET_DEBUG_EN_SHFT	6

/*
 * AOSS_CC_RESET_STATUS (+0x1000): identifies which reset cause fired.
 * Writing '1' to a bit clears it (W1C), per the IP Catalog register
 * description - matches source usage (read-only interpretation here).
 */
#define HWIO_AOSS_CC_RESET_STATUS_ADDR			(AOSS_CC_RST_CTL_REG_BASE + 0x1000)
#define HWIO_AOSS_CC_RESET_STATUS_SECURE_WDOG_EXPIRE_RESET_STATUS_SHFT		0
#define HWIO_AOSS_CC_RESET_STATUS_PMIC_ABNORMAL_RESIN_RESET_STATUS_SHFT	6
#define HWIO_AOSS_CC_RESET_STATUS_PROC_HALT_CTI_STATUS_BMSK			0x10
#define HWIO_AOSS_CC_RESET_STATUS_SRST_RESET_STATUS_BMSK			0x8

/*
 * AOSS_CC_RESET_FSM_STATUS (+0x1004): reset FSM state. FSM_STATE occupies
 * bits[3:0] - identical bit width/position across all four chips checked
 * (hawi/hamoa/kaanapali/nord).
 */
#define HWIO_AOSS_CC_RESET_FSM_STATUS_ADDR			(AOSS_CC_RST_CTL_REG_BASE + 0x1004)
#define HWIO_AOSS_CC_RESET_FSM_STATUS_FSM_STATE_BMSK		0xf

/*
 * AOSS_CC_FIRST_PASS_TIMEOUT (+0x18): single TIMER_VAL field, bits[31:0].
 * Used by tzbsp_set_sdi_pass1_timeout().
 */
#define HWIO_AOSS_CC_FIRST_PASS_TIMEOUT_ADDR			(AOSS_CC_RST_CTL_REG_BASE + 0x18)

/*
 * GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE: APCS TrustZone clock-enable voting
 * register for votable clocks (verified against IP Catalog by exact
 * register address, nordschleife_2.0 - the SWI text search does not index
 * this symbol under the AOSS_CC_RST_CTL_REG module, so it must be looked
 * up by address: GCC_CLK_CTL_REG_REG_BASE (0x110000) + 0x8e030 = 0x19e030.
 * Cross-checked against nord's own TZ build
 * (core.tz.2.34.nord_tz/trustzone_images/core/systemdebug/sysdbg/inc/target/
 * nord/sysdbg_hwio.h) - same address, same QDSS_CFG_AHB_CLK_ENA bit 15.
 */
#define HWIO_GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_ADDR		0x19e030
#define HWIO_GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_QDSS_CFG_AHB_CLK_ENA_BMSK	0x8000

/*
 * MPM global counter (verified against IP Catalog, nordschleife_2.0).
 * Same register names as hawi/hamoa/kaanapali and nord's own TZ build
 * (sysdbg_timer.c) - EN field confirmed at bit 0.
 */
#define HWIO_MPM_CONTROL_CNTCR_ADDR		0xc210000
#define HWIO_MPM_CONTROL_CNTCR_EN_BMSK		0x1
#define HWIO_MPM_CONTROL_CNTCV_LO_ADDR		0xc210008
#define HWIO_MPM_CONTROL_CNTCV_HI_ADDR		0xc21000c

#endif /* SYSDBG_HWIO_H */
