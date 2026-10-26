/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APSS_WATCHDOG_H
#define APSS_WATCHDOG_H

#include <stdint.h>

/* Secure watchdog: independent safety net, owned entirely by secure world */
void qti_apss_watchdog_init(void);
void qti_apss_watchdog_start(uint32_t bark_ms, uint32_t bite_ms);

#endif /* APSS_WATCHDOG_H */
