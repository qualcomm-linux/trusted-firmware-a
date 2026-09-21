/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * V1 watchdog register layout (kodiak, lemans): RESET@0x0, CTL@0x4,
 * BARK@0xc, BITE@0x10. No SECURE register. BARK/BITE writes must be
 * followed by a poll on the per-register sync bit before the value is
 * guaranteed latched.
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
	mmio_setbits_32(WDOG_CTL_ADDR, ENABLE_BIT);
	mmio_setbits_32(WDOG_CTL_ADDR, CLK_ENABLE_BIT);
	mmio_write_32(WDOG_RESET_ADDR, RESET);
}

void qti_watchdog_disable(void)
{
	mmio_clrbits_32(WDOG_CTL_ADDR, ENABLE_BIT);
}

void qti_watchdog_configure(uint32_t bark, uint32_t bite)
{
	mmio_clrbits_32(WDOG_CTL_ADDR, ENABLE_BIT);

	mmio_clrsetbits_32(WDOG_BARK_ADDR, WDOG_BARK_MASK, bark);
	while ((mmio_read_32(WDOG_BARK_ADDR) & BARK_SYNC_BIT) == 0) {
	}

	mmio_clrsetbits_32(WDOG_BITE_ADDR, WDOG_BITE_MASK, bite);
	while ((mmio_read_32(WDOG_BITE_ADDR) & BITE_SYNC_BIT) == 0) {
	}
}

void qti_watchdog_bite_event_disable(void)
{
	mmio_setbits_32(WDOG_CTL_ADDR, CHIP_AUTOPET_EN | HW_SLEEP_WAKEUP_EN);
}

void qti_watchdog_reset(void)
{
	mmio_write_32(WDOG_RESET_ADDR, RESET);
}
