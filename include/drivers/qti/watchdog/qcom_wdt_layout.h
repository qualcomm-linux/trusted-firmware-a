/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCOM_WDT_LAYOUT_H
#define QCOM_WDT_LAYOUT_H

#include <lib/utils_def.h>

/*
 * Register-block layout abstraction for the various Qualcomm watchdog
 * hardware blocks, modelled on the upstream Linux driver
 * drivers/watchdog/qcom-wdt.c: an enum names the logical registers and each
 * target supplies a `qcom_wdt_reg_offset[]` table (indexed by this enum) plus
 * a WDOG_MAX_TICK_COUNT saturation value in its watchdog_defs.h.
 *
 * WDT_RST        write 1 to reset/pet (ping) the watchdog counter
 * WDT_EN         enable register; QCOM_WDT_ENABLE (bit 0) turns the WDT on
 * WDT_STS        status register (unused by this EL3 driver, kept for parity)
 * WDT_BARK_TIME  bark (pre-timeout interrupt) timeout, in WDT ticks
 * WDT_BITE_TIME  bite (reset) timeout, in WDT ticks
 */
enum wdt_reg {
	WDT_RST,
	WDT_EN,
	WDT_STS,
	WDT_BARK_TIME,
	WDT_BITE_TIME,
	WDT_REG_COUNT,
};

#define QCOM_WDT_ENABLE		BIT(0)

#endif /* QCOM_WDT_LAYOUT_H */
