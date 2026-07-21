/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/qti/timer/tzbsp_timer.h>

int tzbsp_cpu_cl_sleep_timer_init(void)
{
	return 0;
}

int tzbsp_cpu_core_sleep_timer_init(void)
{
	return 0;
}

uint32_t timer_get_sec_qtmr_frames_bitmask(uint8_t qtimer_idx)
{
	return 0;
}

int tzbsp_cpu_cl_sleep_timer_cancel(timer_sec_id_t tid)
{
	return 0;
}

int tzbsp_cpu_cl_sleep_timer_start(timer_sec_id_t tid, uint64_t timeout,
				   uint32_t target_cpu)
{
	return 0;
}
