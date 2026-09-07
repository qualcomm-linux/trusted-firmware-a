/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WATCHDOG_GENERIC_H
#define WATCHDOG_GENERIC_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Internal hooks between the family-agnostic core driver (watchdog.c) and the
 * per-hardware-variant register implementation (v1/v2/watchdog_ver.c) and the
 * per-family platform watchdog service. Not part of the public watchdog.h API
 * and not for use outside the watchdog driver itself.
 */

struct qti_watchdog_caps {
	uint64_t frequency_hz;
	uint32_t tick_mask;
};

/* Hardware-variant hooks (v1/watchdog_ver.c, v2/watchdog_ver.c) */
const struct qti_watchdog_caps *qti_watchdog_get_caps(void);
void qti_watchdog_enable(void);
void qti_watchdog_disable(void);
void qti_watchdog_configure(uint32_t bark, uint32_t bite);
void qti_watchdog_bite_event_disable(void);
void qti_watchdog_reset(void);

/* Platform-service hooks (qti_watchdog_platform.c) */
void qti_watchdog_platform_init(void);
bool qti_watchdog_handle_stop_on_fail(void);
void qti_watchdog_set_stop_on_fail(uint32_t err);

#endif /* WATCHDOG_GENERIC_H */
