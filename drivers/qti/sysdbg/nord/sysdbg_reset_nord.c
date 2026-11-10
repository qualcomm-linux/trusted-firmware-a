/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/mmio.h>

#include <sysdbg_hwio.h>

/*
 * Returns non-zero when the current cold boot was triggered by the SDI
 * (secure debug) reset-debug mechanism rather than a normal reset.
 *
 * unsigned int (not bool) to match the signature dst_repo already compiles
 * against in qti_helpers.S's plat_reset_handler (bl sysdbg_reset_check /
 * cmp x0, #0 / bne sysdbg_entry_handler) - the caller only tests zero vs
 * non-zero.
 */
unsigned int sysdbg_reset_check(void)
{
	uint32_t val;

	/* Any reset-debug enable bit set? (PMIC resin or secure wdog) */
	val = mmio_read_32(HWIO_AOSS_CC_RESET_DEBUG_ENABLE_ADDR);
	val = (val >> HWIO_AOSS_CC_RESET_DEBUG_ENABLE_PMIC_RESIN_RESET_DEBUG_EN_SHFT) &
	      ((1U << (HWIO_AOSS_CC_RESET_DEBUG_ENABLE_SECURE_WDOG_RESET_DEBUG_EN_SHFT -
		       HWIO_AOSS_CC_RESET_DEBUG_ENABLE_PMIC_RESIN_RESET_DEBUG_EN_SHFT + 1)) - 1U);
	if (val == 0U) {
		return 0U;
	}

	/* Any debug reset cause set, excluding CTI-halt and SRST? */
	val = mmio_read_32(HWIO_AOSS_CC_RESET_STATUS_ADDR);
	val = (val >> HWIO_AOSS_CC_RESET_STATUS_SECURE_WDOG_EXPIRE_RESET_STATUS_SHFT) &
	      ((1U << (HWIO_AOSS_CC_RESET_STATUS_PMIC_ABNORMAL_RESIN_RESET_STATUS_SHFT -
		       HWIO_AOSS_CC_RESET_STATUS_SECURE_WDOG_EXPIRE_RESET_STATUS_SHFT + 1)) - 1U);
	val &= ~HWIO_AOSS_CC_RESET_STATUS_PROC_HALT_CTI_STATUS_BMSK;
	val &= ~HWIO_AOSS_CC_RESET_STATUS_SRST_RESET_STATUS_BMSK;
	if (val == 0U) {
		return 0U;
	}

	/* Reset FSM settled in the "debug pass 1" state (0x6)? */
	val = mmio_read_32(HWIO_AOSS_CC_RESET_FSM_STATUS_ADDR);
	val &= HWIO_AOSS_CC_RESET_FSM_STATUS_FSM_STATE_BMSK;

	return (val == 0x6U) ? 1U : 0U;
}
