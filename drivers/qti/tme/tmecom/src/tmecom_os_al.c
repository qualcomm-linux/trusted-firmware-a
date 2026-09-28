/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Minimal OS-abstraction shim retained for the upstream tmecom driver.
 * The qti_mbox-based tmecom.c handles all transport internally, so only
 * tmecom_sleep() is needed here (called from tmecom_interfaces.c).
 */

#include <stdint.h>
#include <drivers/delay_timer.h>

#include <tmecom_os_al.h>

void tmecom_sleep(uint32_t msec)
{
	while (msec > 0U) {
		msec -= 1;
		udelay(1000U);
	}
}
