/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include <drivers/qti/icb/icb.h>
#include <drivers/qti/icb/icb_error.h>
#include <drivers/qti/icb/icbcfg.h>
#include <drivers/qti/icb/icbuarb.h>

void qti_icb_init(void)
{
	if (!qti_icbuarb_init()) {
		WARN("ICB: micro-arbiter initialization error\n");
	}

	qti_icb_error_init();
	qti_icbcfg_init();
}
