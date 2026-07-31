/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>

typedef enum {
	QCOM_WATCHDOG_CPU_HOTPLUG = 0U,
	QCOM_WATCHDOG_CPU_SUSPEND = 1U,
	QCOM_WATCHDOG_CPU_WAKEUP = 2U,
} qcom_watchdog_cpu_state_t;

/* Provide watchdog support to the PSCI framework */
void qcom_watchdog_set_target(qcom_watchdog_cpu_state_t state);

void qcom_watchdog_start(uint32_t bark, uint32_t bite);
void qcom_watchdog_stop(void);
void qcom_watchdog_pet(void);
int qcom_watchdog_init(void);

#endif /* WATCHDOG_H */
