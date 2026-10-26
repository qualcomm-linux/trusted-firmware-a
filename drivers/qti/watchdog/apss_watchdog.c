/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <drivers/qti/watchdog/apss_watchdog.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <watchdog_defs.h>

#define APSS_WDOG_BITE_TIME_MS		22000U
#define APSS_WDOG_BARK_TIME_MS		6000U
#define APSS_WDOG_FREQ_IN_HZ		32000ULL

static uint32_t ms_to_apss_wdt_ticks(uint32_t ms)
{
	/* Secure watchdog runs off the 32KHz reference clock. */
	return (uint32_t)((APSS_WDOG_FREQ_IN_HZ * (uint64_t)ms) / 1000ULL);
}

/* Prevent the non-secure WDOG bite event from being forwarded to other blocks */
static void qti_apss_wdog_bite_event_disable(void)
{
	mmio_clrbits_32(WDOG_BITE_INT0_CONFIG_ADDR, WDOG_BITE_INT_SEL_BIT);
}

/* WDOG can be controlled only by the secure world */
static void qti_apss_configure_wdog(void)
{
	mmio_clrbits_32(WDOG_SECURE_ADDR, WDOG_SECURE_WDT_BIT);
}

static void qti_apss_wdog_enable(bool enable)
{
	mmio_write_32(WDOG_CTL_ADDR,
		      WDOG_UNMASKED_INT_ENABLE_BIT |
		      (enable ? WDOG_ENABLE_BIT : 0U));
}

static void qti_apss_wdog_reset(void)
{
	mmio_write_32(WDOG_RESET_ADDR, WDOG_RESET_VAL);
}

void qti_apss_watchdog_start(uint32_t bark_ms, uint32_t bite_ms)
{
	uint32_t bark, bite;

	/* Zero timeouts are not allowed, ensure timeouts are > 0. */
	bark = ms_to_apss_wdt_ticks(MAX(bark_ms, 0x1U));
	bite = ms_to_apss_wdt_ticks(MAX(bite_ms, 0x1U));

	/* Timeouts have a ceiling value. */
	bark = MIN(bark, (uint32_t)WDOG_BARK_MASK);
	bite = MIN(bite, (uint32_t)WDOG_BITE_MASK);

	/* Make sure the watchdog is disabled before reprogramming it. */
	qti_apss_wdog_enable(false);

	/* Program bark and bite timeouts. */
	mmio_write_32(WDOG_BARK_ADDR, bark);
	mmio_write_32(WDOG_BITE_ADDR, bite);

	/* Enable the wdog now. */
	qti_apss_wdog_enable(true);

	/* Reset to make sure the count starts at 0. */
	qti_apss_wdog_reset();
	dsb();
}

void qti_apss_watchdog_init(void)
{
	/*
	 * Secure and non-secure WDOG bite interrupts are OR'd together onto
	 * a single bite output. TZ needs to finish its own error handling
	 * before the non-secure WDOG bite reaches that output, so
	 * WDOG_BITE_INT_SEL is cleared here to keep the NS WDOG from
	 * reaching it until TZ is done.
	 */
	qti_apss_wdog_bite_event_disable();

	qti_apss_configure_wdog();

	/* Start the secure watchdog. */
	qti_apss_watchdog_start(APSS_WDOG_BARK_TIME_MS, APSS_WDOG_BITE_TIME_MS);
}
