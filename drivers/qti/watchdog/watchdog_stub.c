/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch_helpers.h>

#include <drivers/qti/watchdog/watchdog.h>
#include <drivers/qti/watchdog/watchdog_generic.h>

uint64_t qti_watchdog_pet_ticks __section("tzfw_coherent_mem");

static const struct qti_watchdog_caps watchdog_caps = {
	.frequency_hz = 32768ULL,
	.tick_mask = 0xfffffU,
};

const struct qti_watchdog_caps *qti_watchdog_get_caps(void)
{
	return &watchdog_caps;
}

void qti_watchdog_set_target(qti_watchdog_cpu_state_t state)
{
	(void)state;
}

void qti_watchdog_start(uint32_t bark, uint32_t bite)
{
	(void)bark;
	(void)bite;
}

void qti_watchdog_force_bite(uint32_t bite_ms)
{
	(void)bite_ms;
}

void qti_watchdog_stop(void)
{
}

void qti_watchdog_pet(void)
{
}

int qti_watchdog_init(void)
{
	return 0;
}

void qti_watchdog_enable(void)
{
}

void qti_watchdog_disable(void)
{
}

void qti_watchdog_configure(uint32_t bark, uint32_t bite)
{
	(void)bark;
	(void)bite;
}

void qti_watchdog_bite_event_disable(void)
{
}

void qti_watchdog_reset(void)
{
}

void qti_watchdog_platform_init(void)
{
}

bool qti_watchdog_handle_stop_on_fail(void)
{
	return false;
}

void qti_watchdog_set_stop_on_fail(uint32_t err)
{
	(void)err;
}
