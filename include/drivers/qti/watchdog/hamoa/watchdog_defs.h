/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WATCHDOG_DEFS_H
#define WATCHDOG_DEFS_H

#include <lib/utils_def.h>

/*
 * Hamoa uses the V2 watchdog register layout. The SECURE register must be
 * cleared before programming; BARK and BITE writes take effect immediately.
 */
#define WDOG_REG_BASE			0x17414000

#define WDOG_SECURE_ADDR		(WDOG_REG_BASE + 0x0)

#define WDOG_RESET_ADDR			(WDOG_REG_BASE + 0x4)
#define RESET				0x1

#define WDOG_CTL_ADDR			(WDOG_REG_BASE + 0x8)
#define ENABLE_BIT			BIT(0)
#define UNMASKED_INT_EN_BIT		BIT(1)

/* BARK/BITE COUNT fields are 20-bit. */
#define WDOG_BARK_ADDR			(WDOG_REG_BASE + 0x10)

#define WDOG_BITE_ADDR			(WDOG_REG_BASE + 0x14)

#define WDOG_BARK_INT_ID		58U
#define WDOG_NSEC_BITE_INT_ID		33U

/* APSS watchdog bite interrupt routing registers. */
#define APSS_INTU_REG_BASE		0x17400000
#define WDOG_BITE_INT0_CONFIG_ADDR	(APSS_INTU_REG_BASE + 0x38)
#define WDOG_BITE_INT_SEL		BIT(0)

#endif /* WATCHDOG_DEFS_H */
