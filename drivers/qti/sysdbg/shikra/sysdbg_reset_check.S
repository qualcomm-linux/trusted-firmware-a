/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * System Debug - Crash Reset Detection (Shikra)
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/common/src/target/shikra/sysdbg_common_shikra_asm.s
 *
 * This file contains target-specific crash reset detection logic for Shikra.
 */

#include <sysdbg_hwio.h>

	.globl	sysdbg_reset_check

	/*
	 * Place in IMEM section - must execute before DDR is accessible
	 */
	.section .text_imem, "ax"
	.p2align 3

/*
 * sysdbg_reset_check - Check if reset was due to crash/debug
 *
 * This function determines if the current reset was triggered by a crash
 * or debug condition (watchdog, PMIC reset, etc.) by reading GCC registers.
 *
 * Must execute from IMEM since DDR is in self-refresh and not accessible.
 *
 * Called from: plat_reset_handler (qti_helpers.S)
 *
 * Returns:
 *   x0 = 1 if crash/debug reset detected
 *   x0 = 0 if normal cold boot
 *
 * Registers modified: x0
 *
 * TODO: Verify all register addresses against Shikra IPCAT before final integration
 */
	.type	sysdbg_reset_check, %function
sysdbg_reset_check:
	/*
	 * Step 1: Check if reset debug is enabled
	 * Read GCC_RESET_DEBUG_ENABLE register
	 */
	ldr	x0, =HWIO_GCC_RESET_DEBUG_ENABLE_ADDR
	ldr	w0, [x0]

	/*
	 * Extract debug enable bits using UBFM (Unsigned Bit Field Move)
	 * Checks PMIC_RESIN and SECURE_WDOG debug enable bits
	 */
	ubfm	x0, x0, #(HWIO_GCC_RESET_DEBUG_ENABLE_PMIC_RESIN_RESET_DEBUG_EN_SHFT), #(HWIO_GCC_RESET_DEBUG_ENABLE_SECURE_WDOG_RESET_DEBUG_EN_SHFT)
	cmp	x0, #0
	beq	1f		/* Not a debug reset if debug not enabled */

	/*
	 * Step 2: Check reset status register
	 * Read GCC_RESET_STATUS to see what triggered the reset
	 */
	ldr	x0, =HWIO_GCC_RESET_STATUS_ADDR
	ldr	w0, [x0]

	/*
	 * Extract reset status bits
	 * Checks SECURE_WDOG, PMIC_ABNORMAL_RESIN, etc.
	 */
	ubfm	x0, x0, #(HWIO_GCC_RESET_STATUS_SECURE_WDOG_EXPIRE_RESET_STATUS_SHFT), #(HWIO_GCC_RESET_STATUS_PMIC_ABNORMAL_RESIN_RESET_STATUS_SHFT)

	/*
	 * Mask out PROC_HALT_CTI and SRST status bits
	 * (These don't indicate crash)
	 */
	and	x0, x0, #(~HWIO_GCC_RESET_STATUS_PROC_HALT_CTI_STATUS_BMSK)
	and	x0, x0, #(~HWIO_GCC_RESET_STATUS_SRST_RESET_STATUS_BMSK)
	cmp	x0, #0
	beq	1f		/* Not a crash if no reset status bits set */

	/*
	 * Step 3: Check FSM state
	 * Read GCC_RESET_FSM_STATUS to verify FSM is in correct state
	 */
	ldr	x0, =HWIO_GCC_RESET_FSM_STATUS_ADDR
	ldr	w0, [x0]
	and	x0, x0, #(HWIO_GCC_RESET_FSM_STATUS_FSM_STATE_BMSK)

	/*
	 * FSM state 0x6 indicates crash/debug reset path
	 */
	cmp	x0, #0x6
	cset	x0, eq		/* Set x0=1 if FSM state is 0x6, else x0=0 */

1:
	ret

	.size	sysdbg_reset_check, . - sysdbg_reset_check
