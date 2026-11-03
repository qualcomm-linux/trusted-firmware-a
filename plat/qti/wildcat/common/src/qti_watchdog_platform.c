/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <platform.h>

#include <drivers/qti/watchdog/watchdog.h>
#include <qti_interrupt_svc.h>
#include <qti_plat.h>
#include <watchdog_defs.h>

#define WDOG_NSEC_RESET_DELAY_MS	500U

static void *bark_handler(void *ctx)
{
	qti_watchdog_pet();

	return ctx;
}

static void *nsec_bite_handler(void *ctx)
{
	(void)ctx;

	ERROR("AP-NSEC watchdog bite\n");
	qti_watchdog_force_bite(WDOG_NSEC_RESET_DELAY_MS);

	while (true) {
		/* Wait for the secure watchdog to reset the device. */
	}

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

	ret = qti_register_intr_type_el3(WDOG_NSEC_BITE_INT_ID,
					 nsec_bite_handler, NULL);
	if (ret != 0) {
		ERROR("Failure registering AP-NSEC watchdog bite: %d\n", ret);
		qti_unregister_intr_type_el3(WDOG_BARK_INT_ID);
		return ret;
	}

	gic_set_spi_routing(WDOG_NSEC_BITE_INT_ID, GICV3_IRM_ANY, 0U);

	return 0;
}
