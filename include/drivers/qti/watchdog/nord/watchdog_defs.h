/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WATCHDOG_DEFS_H
#define WATCHDOG_DEFS_H

#include <lib/utils_def.h>

/*
 * NORD provides windowed and AP-SEC watchdog blocks selected at build time.
 * The windowed block uses 32-bit timeout fields clocked at 19.2 MHz. The
 * AP-SEC block uses the V2 register layout and 20-bit timeout fields.
 */
#define WDOG_WINDOWED_REG_BASE		0x17820000

#ifndef WDOG_APSEC_REG_BASE
#define WDOG_APSEC_REG_BASE		0x17825000
#endif

#ifdef WDOG_USE_APSEC
#define WDOG_REG_BASE			WDOG_APSEC_REG_BASE
#else
#define WDOG_REG_BASE			WDOG_WINDOWED_REG_BASE
#endif

#ifdef WDOG_USE_APSEC
/* Plain AP-SEC watchdog using the V2 register layout. */
#define WDOG_SECURE_ADDR		(WDOG_REG_BASE + 0x0)

#define WDOG_RESET_ADDR			(WDOG_REG_BASE + 0x4)
#define RESET				0x1

#define WDOG_CTL_ADDR			(WDOG_REG_BASE + 0x8)
#define ENABLE_BIT			BIT(0)
#define UNMASKED_INT_EN_BIT		BIT(1)

/* BARK/BITE COUNT fields are 20-bit. */
#define WDOG_BARK_ADDR			(WDOG_REG_BASE + 0x10)

#define WDOG_BITE_ADDR			(WDOG_REG_BASE + 0x14)
#else
/* Windowed watchdog with direct enable and disable register writes. */
#define WDOG_ENABLE_ADDR		(WDOG_REG_BASE + 0x0)
#define WDOG_ENABLE_VAL			0x1
#define WDOG_DISABLE_VAL		0x2

#define WDOG_PET_ADDR			(WDOG_REG_BASE + 0x4)
#define WDOG_PET_VAL			0x1

#define WDOG_START_TIME_ADDR		(WDOG_REG_BASE + 0x8)
#define WDOG_START_TIME_VAL		0x0

#define WDOG_BARK_TIME_ADDR		(WDOG_REG_BASE + 0xC)
#define WDOG_BITE_TIME_ADDR		(WDOG_REG_BASE + 0x10)

#define WDOG_WINDOW_MIN_TIME_ADDR	(WDOG_REG_BASE + 0x14)
#define WDOG_WINDOW_MIN_TIME_VAL	0x0

#define WDOG_WINDOW_MAX_TIME_ADDR	(WDOG_REG_BASE + 0x18)
#define WDOG_WINDOW_MAX_TIME_VAL	0xFFFFFFFFU

#define WDOG_ERROR_SYNDROME_ADDR	(WDOG_REG_BASE + 0x24)

#define WDOG_FREEZE_CTRL_ADDR		(WDOG_REG_BASE + 0x28)
#define WDOG_FREEZE_CTRL_DEBUG_MODE_FREEZE_BIT	BIT(1)
#define WDOG_FREEZE_CTRL_PWR_CTL_FREEZE_BIT	BIT(0)
#endif /* WDOG_USE_APSEC */

/* NORD watchdog interrupt IDs. */
#define WDOG_BARK_INT_ID			80U
#define WDOG_NSEC_BITE_INT_ID		4943U

/* APSS watchdog bite interrupt routing registers. */
#define APSS_INTU_REG_BASE		0x17822000
#define WDOG_BITE_INT0_CONFIG_ADDR	(APSS_INTU_REG_BASE + 0x8)
#define WDOG_BITE_INT_SEL		BIT(0)

#endif /* WATCHDOG_DEFS_H */
