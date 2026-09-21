/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * V2 windowed watchdog (WWDOG) register layout (nord): ENABLE@0x0, PET@0x4,
 * START_TIME@0x8, BARK_TIME@0xC, BITE_TIME@0x10, WINDOW_MIN_TIME@0x14,
 * WINDOW_MAX_TIME@0x18, ERROR_SYNDROME@0x24, FREEZE_CTRL@0x28. Unlike the
 * plain V2 layout (hamoa), there is no SECURE register and ENABLE/DISABLE
 * are direct value writes (1/2), not a bitfield set/clear. Nord runs the
 * WWDOG in legacy (non-windowed) mode: WINDOW_MIN_TIME=0,
 * WINDOW_MAX_TIME=0xFFFFFFFF. No BARK/BITE sync-bit polling.
 */

#include <lib/mmio.h>

#include <drivers/qti/watchdog/watchdog_generic.h>
#include <watchdog_defs.h>

static const struct qti_watchdog_caps watchdog_caps = {
	.frequency_hz = 19200000ULL,
	.tick_mask = 0xffffffffU,
};

const struct qti_watchdog_caps *qti_watchdog_get_caps(void)
{
	return &watchdog_caps;
}

static void qti_watchdog_freeze_control_configure(void)
{
	/* Freeze the WWDOG counter when halted by a debugger or in LPM. */
	mmio_setbits_32(WDOG_FREEZE_CTRL_ADDR,
			 WDOG_FREEZE_CTRL_DEBUG_MODE_FREEZE_BIT |
			 WDOG_FREEZE_CTRL_PWR_CTL_FREEZE_BIT);
}

void qti_watchdog_enable(void)
{
	mmio_write_32(WDOG_ENABLE_ADDR, WDOG_ENABLE_VAL);
}

void qti_watchdog_disable(void)
{
	mmio_write_32(WDOG_ENABLE_ADDR, WDOG_DISABLE_VAL);
}

void qti_watchdog_configure(uint32_t bark, uint32_t bite)
{
	qti_watchdog_disable();

	qti_watchdog_freeze_control_configure();

	mmio_write_32(WDOG_START_TIME_ADDR, WDOG_START_TIME_VAL);
	mmio_write_32(WDOG_BARK_TIME_ADDR, bark);
	mmio_write_32(WDOG_BITE_TIME_ADDR, bite);

	/* Legacy mode: windowing is programmed but functionally disabled. */
	mmio_write_32(WDOG_WINDOW_MIN_TIME_ADDR, WDOG_WINDOW_MIN_TIME_VAL);
	mmio_write_32(WDOG_WINDOW_MAX_TIME_ADDR, WDOG_WINDOW_MAX_TIME_VAL);
}

void qti_watchdog_bite_event_disable(void)
{
	mmio_clrbits_32(WDOG_BITE_INT0_CONFIG_ADDR, WDOG_BITE_INT_SEL);
}

void qti_watchdog_reset(void)
{
	/* Read ERROR_SYNDROME to clear any bark/window-min/window-max error
	 * latched since the last pet, per the WWDOG programming guide.
	 */
	(void)mmio_read_32(WDOG_ERROR_SYNDROME_ADDR);

	/* Program PET to restart the WWDOG timer. */
	mmio_write_32(WDOG_PET_ADDR, WDOG_PET_VAL);
}
