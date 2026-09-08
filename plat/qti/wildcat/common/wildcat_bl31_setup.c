/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>

#include <platform_def.h>
#include <qti_plat.h>

void plat_qti_bl31_setup_post(void)
{
	/*
	 * Mask TME_WDOG_EXPIRED (and defensively PDC_WDOG_EXPIRED) at the final
	 * MPM_PS_HOLD_MASK gate. The direct TME_WDOG disable does NOT prevent
	 * TME_WDOG_EXPIRED from being observed set - A/B boot testing confirms
	 * this mask is what actually stops the ~12s cold-boot PS_HOLD reset.
	 * Bring-up mitigation, not a safety-compliant fix: it masks the reset
	 * request rather than resolving why TME still asserts it.
	 */
	mmio_clrbits_32(QTI_MPM_PS_HOLD_MASK,
			QTI_MPM_PS_HOLD_MASK_TME_WDOG_EXPIRED |
			QTI_MPM_PS_HOLD_MASK_PDC_WDOG_EXPIRED);

	/*
	 * Bring up the APSS INTU so peripheral SPIs (UFS SPI265/INTID297, geni
	 * 615, RSC 61) reach the GIC-700 with the correct per-SPI level/edge
	 * type; our stub did not program it, so level SPIs never delivered. Runs
	 * before bl31_plat_runtime_setup() enables G1NS SPI forwarding at the
	 * distributor.
	 */
	plat_intu_init();
}

/*******************************************************************************
 * Perform any platform specific runtime setup prior to cold boot exit
 * from BL31
 ******************************************************************************/
void bl31_plat_runtime_setup(void)
{
	/*
	 * Enable distributor forwarding of G1NS SPIs as the last cold-boot
	 * action, after all BL31 setup (secure SPIs) is complete.
	 * gicv3_distif_init() clears EnableGrp1NS and never restores it;
	 * this GIC-700 also resets it to 0, so it must be set explicitly here.
	 */
	mmio_setbits_32(QTI_GICD_BASE + GICD_CTLR, CTLR_ENABLE_G1NS_BIT);
	while ((mmio_read_32(QTI_GICD_BASE + GICD_CTLR) &
		GICD_CTLR_RWP_BIT) != 0U) {
	}
}
