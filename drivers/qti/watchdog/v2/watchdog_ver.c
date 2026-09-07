/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * V2 watchdog register layout (hamoa, firewheel): SECURE@0x0, RESET@0x4,
 * CTL@0x8, BARK@0x10, BITE@0x14. The SECURE register must be cleared before
 * any other register is programmed. No BARK/BITE sync-bit polling -
 * unlike V1, writes to BARK/BITE take effect immediately.
 */

#include <lib/mmio.h>

#include <drivers/qti/watchdog/watchdog_generic.h>
#include <watchdog_defs.h>

static const struct qti_watchdog_caps watchdog_caps = {
	.frequency_hz = 32768ULL,
	.tick_mask = 0xfffffU,
};

const struct qti_watchdog_caps *qti_watchdog_get_caps(void)
{
	return &watchdog_caps;
}

void qti_watchdog_enable(void)
{
	mmio_write_32(WDOG_CTL_ADDR, UNMASKED_INT_EN_BIT | ENABLE_BIT);
}

void qti_watchdog_disable(void)
{
	mmio_write_32(WDOG_CTL_ADDR, UNMASKED_INT_EN_BIT);
}

void qti_watchdog_configure(uint32_t bark, uint32_t bite)
{
	/* Clear security ownership before programming the watchdog. */
	mmio_write_32(WDOG_SECURE_ADDR, 0);

	qti_watchdog_disable();

	mmio_write_32(WDOG_BARK_ADDR, bark);
	mmio_write_32(WDOG_BITE_ADDR, bite);
}

void qti_watchdog_bite_event_disable(void)
{
	mmio_clrbits_32(WDOG_BITE_INT0_CONFIG_ADDR, WDOG_BITE_INT_SEL);
}

void qti_watchdog_reset(void)
{
	mmio_write_32(WDOG_RESET_ADDR, RESET);
}
