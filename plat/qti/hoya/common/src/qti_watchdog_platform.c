/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include <drivers/qti/watchdog/watchdog.h>
#include <qti_interrupt_svc.h>
#include <watchdog_defs.h>

static void *bark_handler(void *ctx)
{
	qti_watchdog_pet();

	return ctx;
}

int qti_watchdog_platform_init(void)
{
	int ret;

	ret = qti_register_intr_type_el3(WDOG_BARK_INT_ID, bark_handler, NULL);
	if (ret != 0) {
		ERROR("Failure registering watchdog: %d\n", ret);
		return ret;
	}

	qti_watchdog_set_target(QTI_WATCHDOG_CPU_WAKEUP);
	return 0;
}
