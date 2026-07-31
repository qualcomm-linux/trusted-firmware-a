/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/qcom/qtimer/qtimer.h>

void qcom_qtimer_init(void)
{
}

uint64_t qcom_qtimer_get_raw(void)
{
	WARN("Qtimer not supported\n");
	return 0LL;
}

uint64_t qcom_qtimer_get_usec(void)
{
	WARN("Qtimer not supported.\n");
	return 0LL;
}

