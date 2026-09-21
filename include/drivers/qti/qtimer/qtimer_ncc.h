/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTIMER_NCC_H
#define QTIMER_NCC_H

#include <stdint.h>

#include <drivers/qti/qtimer/qtimer_generic.h>
#include <lib/utils_def.h>

int qti_qtimer_ncc_cl_sleep_timer_init(void);
int qti_qtimer_ncc_cl_sleep_timer_start(qti_qtimer_id_t tid,
					uint64_t timeout_us,
					u_register_t target_cpu_mpidr);
int qti_qtimer_ncc_cl_sleep_timer_cancel(qti_qtimer_id_t tid);
int qti_qtimer_ncc_core_sleep_timer_init(void);
int qti_qtimer_ncc_core_sleep_timer_start(uint64_t timeout_us);
int qti_qtimer_ncc_core_sleep_timer_cancel(void);

#endif /* QTIMER_NCC_H */
