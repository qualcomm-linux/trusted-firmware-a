/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TIMER_NCC_H
#define TIMER_NCC_H

#include <drivers/qti/timer/tzbsp_timer.h>

int tzbsp_cpu_cl_sleep_timer_start(timer_sec_id_t tid, uint64_t timeout,
				   uint32_t target_cpu);
int tzbsp_cpu_cl_sleep_timer_cancel(timer_sec_id_t tid);

#endif /* TIMER_NCC_H */
