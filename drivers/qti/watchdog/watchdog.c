/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>

#include <drivers/qti/qtimer/qtimer.h>
#include <drivers/qti/watchdog/watchdog.h>
#include <drivers/qti/watchdog/watchdog_generic.h>

#define WDOG_BARK_TIME_MS	6000U
#define WDOG_BITE_TIME_MS	22000U

uint64_t qti_watchdog_pet_ticks __section("tzfw_coherent_mem");

static uint32_t ms_to_wdt_ticks(uint32_t ms)
{
	const struct qti_watchdog_caps *caps = qti_watchdog_get_caps();
	uint64_t ticks;

	ticks = (caps->frequency_hz * (uint64_t)ms) / 1000ULL;
	if (ticks > caps->tick_mask) {
		return caps->tick_mask;
	}

	return (uint32_t)ticks;
}

void qti_watchdog_start(uint32_t bark_ms, uint32_t bite_ms)
{
	uint32_t bark, bite;

	if (qti_watchdog_handle_stop_on_fail()) {
		return;
	}

	bark = ms_to_wdt_ticks(MAX(bark_ms, 0x1U));
	bite = ms_to_wdt_ticks(MAX(bite_ms, 0x1U));

	qti_watchdog_configure(bark, bite);
	qti_watchdog_enable();
}

void qti_watchdog_force_bite(uint32_t bite_ms)
{
	const struct qti_watchdog_caps *caps = qti_watchdog_get_caps();
	uint32_t bark, bite;

	bite = ms_to_wdt_ticks(MAX(bite_ms, 0x1U));
	bite = MIN(bite, caps->tick_mask / 2U);
	bite = MAX(bite, 0x1U);
	bark = bite * 2U;

	qti_watchdog_configure(bark, bite);
	qti_watchdog_enable();
	dsb();
}

void qti_watchdog_stop(void)
{
	qti_watchdog_disable();
	qti_watchdog_reset();
	dsb();
}

void qti_watchdog_pet(void)
{
	qti_watchdog_pet_ticks = qti_qtimer_get_raw();
	qti_watchdog_reset();
	dsb();
}

int qti_watchdog_init(void)
{
	qti_watchdog_platform_init();
	qti_watchdog_bite_event_disable();
	qti_watchdog_start(WDOG_BARK_TIME_MS, WDOG_BITE_TIME_MS);

	return 0;
}
