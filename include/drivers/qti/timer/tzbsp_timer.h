/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TZBSP_TIMER_H
#define TZBSP_TIMER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	TIMER_SEC_CP15 = 0,		/* CPU specific secure CP15 QTimer */
	TIMER_SEC_QTIMER = 1,		/* Common secure QTimer */
	TIMER_SEC_QTIMER_FR3 = 2,	/* Secure QTimer Frame 3 */
	TIMER_SEC_QTIMER_FR4 = 3,	/* Secure QTimer Frame 4 */
	TIMER_SEC_QTIMER_FR5 = 4,	/* Secure QTimer Frame 5 */
	TIMER_SEC_MAX_TID,		/* This needs to be last entry */
} timer_sec_id_t;

void tzbsp_timer_init(void);
int timer_one_shot_start(timer_sec_id_t tid, uint64_t timeout);
int timer_stop(timer_sec_id_t tid);
uint64_t timer_get_count_in_us(timer_sec_id_t tid);
int timer_install_isr(timer_sec_id_t tid, void *(*fn)(void *), void *ctx);
int timer_enable_int(timer_sec_id_t tid);
int timer_disable_int(timer_sec_id_t tid);
uint32_t timer_get_timer_secure_int_id(timer_sec_id_t tid);
int tzbsp_cpu_cl_sleep_timer_init(void);
int tzbsp_cpu_core_sleep_timer_init(void);
uint32_t timer_get_sec_qtmr_frames_bitmask(uint8_t qtimer_idx);
bool timer_is_tid_valid(timer_sec_id_t tid);

#endif /* TZBSP_TIMER_H */
