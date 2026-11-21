/*
 * Copyright (c) 2020, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <qti_plat.h>
#include <spmi_arb.h>

/*
 * This driver implements PON support for PM8998-compatible PMICs. This can
 * include other part numbers like PM6150.
 */

#define RESET_TYPE_WARM_RESET		1
#define RESET_TYPE_SHUTDOWN		4
#define RESET_TYPE_HARD_RESET		7

#define S2_RESET_EN			BIT(7)

/*
 * Vendor SYSTEM_RESET2 sub-types, selected by the low bits of the PSCI
 * reset_type argument. Mirrors the sub-types of downstream TZ's
 * pm_app_pon_reset_cfg_type enum that shikra's PMIC PON config actually
 * distinguishes; any other value is rejected as unsupported.
 */
#define QTI_PON_CFG_WARM_RESET		0
#define QTI_PON_CFG_HARD_RESET		1

#define QTI_RESET2_TYPE_MASK		0x7fU

/* Set when the vendor payload carries a reboot-reason cookie update. */
#define QTI_RESET2_COOKIE		BIT(16)

static void configure_ps_hold(uint32_t reset_type)
{
	/* QTI recommends disabling reset for 10 cycles before reconfiguring. */
	spmi_arb_write8(PON_PS_HOLD_RESET_CTL2, 0);
	mdelay(1);

	spmi_arb_write8(PON_PS_HOLD_RESET_CTL, reset_type);
	spmi_arb_write8(PON_PS_HOLD_RESET_CTL2, S2_RESET_EN);
	mdelay(1);
}

void qti_pmic_prepare_reset(void)
{
	configure_ps_hold(RESET_TYPE_HARD_RESET);
}

void qti_pmic_prepare_shutdown(void)
{
	configure_ps_hold(RESET_TYPE_SHUTDOWN);
}

int qti_pmic_prepare_reset2(uint32_t reset_type, uint64_t cookie)
{
	uint32_t vendor_type = reset_type & QTI_RESET2_TYPE_MASK;

#if defined(PON_SOFT_RB_SPARE) && defined(TCSR_BOOT_MISC_DETECT)
	/*
	 * Stash the reboot-reason cookie the same way downstream TZ does, so
	 * bootloader/HLOS reboot-reason detection keeps working: bit 16
	 * selects a PMIC PON scratch-register write, otherwise the cookie
	 * goes into the TCSR scratch register. The scratch registers this
	 * relies on are only defined for targets that need this behavior.
	 */
	if ((reset_type & QTI_RESET2_COOKIE) != 0U) {
		spmi_arb_write8(PON_SOFT_RB_SPARE, (uint8_t)(cookie << 1));
		mmio_write_32(TCSR_BOOT_MISC_DETECT, 0);
	} else if (cookie != 0U) {
		mmio_write_32(TCSR_BOOT_MISC_DETECT, (uint32_t)cookie);
		spmi_arb_write8(PON_SOFT_RB_SPARE, 0);
	}
#endif

	switch (vendor_type) {
	case QTI_PON_CFG_WARM_RESET:
		configure_ps_hold(RESET_TYPE_WARM_RESET);
		return PSCI_E_SUCCESS;
	case QTI_PON_CFG_HARD_RESET:
		configure_ps_hold(RESET_TYPE_HARD_RESET);
		return PSCI_E_SUCCESS;
	default:
		return PSCI_E_NOT_SUPPORTED;
	}
}
