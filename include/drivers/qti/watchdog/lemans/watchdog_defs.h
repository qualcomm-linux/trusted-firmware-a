/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WATCHDOG_DEFS_H
#define WATCHDOG_DEFS_H

#include <drivers/qti/watchdog/qcom_wdt_layout.h>

#include "lemans_def.h"

/*
 * Classic AOSS windowed watchdog (AOSS_BASE + 0x01230000).
 *
 * Register map:
 *   +0x00  WDOG_RESET  write 1 to reset/pet the counter        (WDT_RST)
 *   +0x04  WDOG_CTL    bit0 = enable, bit31 = clock enable      (WDT_EN)
 *   +0x0c  WDOG_BARK   bark timeout [19:0], bit31 = sync        (WDT_BARK_TIME)
 *   +0x10  WDOG_BITE   bite timeout [19:0], bit31 = sync        (WDT_BITE_TIME)
 *
 * The enable and clock-enable bits share the WDT_EN control register, the time
 * registers latch asynchronously behind a sync bit, and a one-time control
 * write selects chip auto-pet + HW sleep/wakeup. These behaviours are selected
 * by the WDOG_HAS_* switches below.
 */
#define WDOG_BARK_INT_ID		(0x204)

#define WDOG_REG_BASE			(QTI_AOSS_BASE + 0x01230000)

/* enum wdt_reg -> byte offset within WDOG_REG_BASE. */
#define WDOG_REG_OFFSETS		{	\
	[WDT_RST]       = 0x00,		\
	[WDT_EN]        = 0x04,		\
	[WDT_STS]       = 0x08,		\
	[WDT_BARK_TIME] = 0x0c,		\
	[WDT_BITE_TIME] = 0x10,		\
}

#define WDOG_MAX_TICK_COUNT		0xFFFFFU
#define WDOG_FREQ_IN_HZ			32768ULL

/* Classic-block behaviours. */
#define WDOG_HAS_SYNC			1
#define WDOG_HAS_CLK_ENABLE		1
#define WDOG_HAS_CTL_INIT		1

#define WDOG_TIME_MASK			0xfffffU
#define WDOG_SYNC_BIT			BIT(31)
#define WDOG_CLK_ENABLE_BIT		BIT(31)

/* One-time WDT_EN (control) init: chip auto-pet (0x4) | HW sleep/wakeup (0x2). */
#define WDOG_CTL_INIT_VAL		(0x4U | 0x2U)

#endif /* WATCHDOG_DEFS_H */
