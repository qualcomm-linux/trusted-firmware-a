/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Minimal OS-abstraction shim retained for the upstream tmecom driver.
 * The qcom_mbox-based tmecom.c handles all transport internally, so only
 * tmecom_sleep() is needed here (called from tmecom_interfaces.c).
 */

#include <stdint.h>

#include "tmecom_os_al.h"

/* Forward-declare udelay to avoid pulling in delay_timer.h -> arch_helpers.h */
void udelay(uint32_t usec);

void tmecom_sleep(uint32_t msec)
{
	udelay(msec * 1000U);
}
