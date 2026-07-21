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

/*
 * Interrupt flags for timer_plat_ops_t.register_isr.
 */
#define TIMER_INTF_TRIGGER_LEVEL	0x00000000U
#define TIMER_INTF_NON_FATAL_INT	0x00002000U
#define TIMER_INT_TARGET_SELF		0x02000000U
#define TIMER_INTF_ALL_CPUS		0x40000000U

/*
 * Platform callbacks for timer interrupt operations.
 * Must be registered via timer_register_plat_ops() before tzbsp_timer_init().
 *
 * register_isr signature matches bl31qtilib_cb_int_register_isr so it can be
 * assigned directly on Wildcat without a wrapper.
 */
typedef struct {
	int (*register_isr)(uint32_t int_id, const char *int_desc,
			    void *(*fn)(void *), void *ctx,
			    uint32_t flags, bool enable);
	int (*enable_int)(uint32_t int_id);
	int (*disable_int)(uint32_t int_id);
	int (*set_int_targets)(uint32_t int_id, uint32_t target_cpu);
} timer_plat_ops_t;

void timer_register_plat_ops(const timer_plat_ops_t *ops);

/*
 * Initialize QTimer hardware.
 * needs_frame_config: true on cold boot or quick boot (frame security
 * registers must be (re-)programmed); false on warm boot.
 */
void tzbsp_timer_init(bool needs_frame_config);

int timer_one_shot_start(timer_sec_id_t tid, uint64_t timeout);
int timer_stop(timer_sec_id_t tid);
uint64_t timer_get_count_in_us(timer_sec_id_t tid);
uint64_t timer_get_uptime_count_raw(void);
int timer_install_isr(timer_sec_id_t tid, void *(*fn)(void *), void *ctx);
int timer_enable_int(timer_sec_id_t tid);
int timer_disable_int(timer_sec_id_t tid);
uint32_t timer_get_timer_secure_int_id(timer_sec_id_t tid);
int tzbsp_cpu_cl_sleep_timer_init(void);
int tzbsp_cpu_core_sleep_timer_init(void);
uint32_t timer_get_sec_qtmr_frames_bitmask(uint8_t qtimer_idx);
bool timer_is_tid_valid(timer_sec_id_t tid);

#endif /* TZBSP_TIMER_H */
