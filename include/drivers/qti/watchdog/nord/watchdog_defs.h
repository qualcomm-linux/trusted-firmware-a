/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WATCHDOG_DEFS_H
#define WATCHDOG_DEFS_H

#include <drivers/qti/watchdog/qcom_wdt_layout.h>

#include "nord_def.h"

/*
 * Nord secure watchdog: the APSS secure windowed watchdog (APSS_WDT_SEC_WWDOG)
 * at 0x17820000. This replaces the legacy AOSS-block watchdog (AOSS_BASE +
 * 0x01230000) used by the older hoya SoCs.
 *
 * Register map (verified against IP Catalog, SA8797P NordAU v2):
 *   +0x00  APSS_WDT_SEC_WWDOG_ENABLE     RW  bit0 = enable
 *   +0x04  APSS_WDT_SEC_WWDOG_PET        W   bit0 = pet/reset the counter
 *   +0x08  APSS_WDT_SEC_WWDOG_START_TIME RW  COUNT[30:0]
 *   +0x0c  APSS_WDT_SEC_WWDOG_BARK_TIME  RW  COUNT[30:0]
 *   +0x10  APSS_WDT_SEC_WWDOG_BITE_TIME  RW  COUNT[30:0]
 *
 * Unlike the classic AOSS windowed watchdog this block has no combined control
 * register (separate ENABLE / PET), no clock-enable bit, and the time
 * registers are plain 31-bit counters with no async sync bit. The logical
 * WDT_RST role is served by the PET register (write 1 to pet), and WDT_EN by
 * the ENABLE register.
 *
 * The bark interrupt id is NORD_INT_ID_SEC_WDOG_BARK, which the GIC
 * driver (qti_gic_v3.c) already configures as a secure edge interrupt.
 * Nord's value (0x50, GIC vector 80 = SPI 48, WDT_barkInt_sec per IPCAT
 * interrupt map 3711) differs from the Hoya/kodiak-lemans value (0x204)
 * that a stale local copy of this macro used to hardcode - that mismatch
 * meant the driver registered its bark handler on an interrupt ID the GIC
 * never actually routes here, so the pet-on-bark path was silently dead
 * even though qti_watchdog_init() reported success.
 */
#define WDOG_BARK_INT_ID		NORD_INT_ID_SEC_WDOG_BARK

#define WDOG_REG_BASE			0x17820000

/* enum wdt_reg -> byte offset within WDOG_REG_BASE. */
#define WDOG_REG_OFFSETS		{	\
	[WDT_EN]        = 0x00,		\
	[WDT_RST]       = 0x04,		\
	[WDT_STS]       = 0x08,		\
	[WDT_BARK_TIME] = 0x0c,		\
	[WDT_BITE_TIME] = 0x10,		\
}

/* COUNT field is bits[30:0]; cap timeouts to the 31-bit counter range. */
#define WDOG_MAX_TICK_COUNT		0x7FFFFFFFU

/*
 * IPCAT clock field for this block is "rif_clk" (chip 781). JTAG-confirmed:
 * XBL leaves BARK_TIME=BITE_TIME=0x22551000, which is exactly 30.000s at
 * 19.2MHz - not the 32768Hz sleep clock the classic AOSS windowed watchdog
 * (kodiak/lemans) uses.
 */
#define WDOG_FREQ_IN_HZ			19200000ULL

/*
 * Simple enable/pet/timeout block: no control-register init, no clock-enable
 * bit, no async time-register sync bit.
 */
#define WDOG_HAS_SYNC			0
#define WDOG_HAS_CLK_ENABLE		0
#define WDOG_HAS_CTL_INIT		0

#endif /* WATCHDOG_DEFS_H */
