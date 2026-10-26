/*
 * Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WATCHDOG_DEFS_H
#define WATCHDOG_DEFS_H

#include <lib/utils_def.h>

#define WDOG_REG_BASE			(0x0B117000)
#define WDOG_SECURE_ADDR		(WDOG_REG_BASE + 0x0)
#define WDOG_SECURE_WDT_BIT		BIT(0)
#define WDOG_RESET_ADDR			(WDOG_REG_BASE + 0x4)
#define WDOG_RESET_VAL			0x1
#define WDOG_CTL_ADDR			(WDOG_REG_BASE + 0x8)
#define WDOG_ENABLE_BIT			BIT(0)
#define WDOG_UNMASKED_INT_ENABLE_BIT	BIT(1)
#define WDOG_BARK_ADDR			(WDOG_REG_BASE + 0x10)
#define WDOG_BARK_MASK			0xfffff
#define WDOG_BITE_ADDR			(WDOG_REG_BASE + 0x14)
#define WDOG_BITE_MASK			0xfffff

/* APSS global registers, used to steer the secure watchdog bite event */
#define APSS_GLB_REG_BASE		(0x0B111000)
#define WDOG_BITE_INT0_CONFIG_ADDR	(APSS_GLB_REG_BASE + 0x2d4)
#define WDOG_BITE_INT_SEL_BIT		BIT(0)

#endif /* WATCHDOG_DEFS_H */
