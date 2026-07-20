/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <common/debug.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <bl31qtilib_cb_interface.h>

#include <drivers/qti/timer/timer_ncc.h>
#include <drivers/qti/timer/tzbsp_timer.h>
#include <drivers/qti/timer/timer_defs.h>

typedef enum {
	TIMER_STATE_FR3 = 0,
	TIMER_STATE_FR4 = 1,
	TIMER_STATE_FR5 = 2,
	TIMER_STATE_CNT = 3,
} timer_state_t;

typedef struct tzbsp_cl_timer_handle_s {
	bool cl_timer_active[TIMER_STATE_CNT];
	spinlock_t cl_timer_mutex[TIMER_STATE_CNT];
	bool cl_timer_init_done;
} tzbsp_cl_timer_handle_t;

typedef struct tzbsp_core_timer_handle_s {
	bool core_timer_active[TZBSP_CPU_COUNT];
	bool core_timer_init_done[TZBSP_CPU_COUNT];
} tzbsp_core_timer_handle_t;

static tzbsp_cl_timer_handle_t cl_timer_handle = { 0 };
static tzbsp_core_timer_handle_t core_timer_handle = { 0 };

static void cl_timer_init(void)
{
	if (cl_timer_handle.cl_timer_init_done) {
		return;
	}

	for (int i = 0; i < TIMER_STATE_CNT; i++) {
		cl_timer_handle.cl_timer_active[i] = false;
	}
	cl_timer_handle.cl_timer_init_done = true;
}

static void cl_timer_set_active(timer_sec_id_t tid, bool state)
{
	switch (tid) {
	case TIMER_SEC_QTIMER_FR3:
		cl_timer_handle.cl_timer_active[TIMER_STATE_FR3] = state;
		break;
	case TIMER_SEC_QTIMER_FR4:
		cl_timer_handle.cl_timer_active[TIMER_STATE_FR4] = state;
		break;
	case TIMER_SEC_QTIMER_FR5:
		cl_timer_handle.cl_timer_active[TIMER_STATE_FR5] = state;
		break;
	default:
		break;
	}
}

static bool cl_timer_check_active(timer_state_t index)
{
	bool state = false;

	switch (index) {
	case TIMER_STATE_FR3:
	case TIMER_STATE_FR4:
	case TIMER_STATE_FR5:
		state = cl_timer_handle.cl_timer_active[index];
		break;
	default:
		break;
	}
	return state;
}

int tzbsp_cpu_cl_sleep_timer_start(timer_sec_id_t tid, uint64_t timeout,
				   uint32_t target_cpu)
{
	int ret = 0;
	uint32_t int_id;
	timer_state_t state_idx;
	bool valid_tid = true;

	if (!timer_is_tid_valid(tid)) {
		ERROR("Invalid qtimer frame: %u\n", tid);
		return -1;
	}

	if (!cl_timer_handle.cl_timer_init_done) {
		ERROR("Qtimer start failed, init not done, tid: %u\n", tid);
		return -1;
	}

	switch (tid) {
	case TIMER_SEC_QTIMER_FR3:
		int_id = TIMER_SEC_QTMR_FR3_INT_ID;
		state_idx = TIMER_STATE_FR3;
		break;
	case TIMER_SEC_QTIMER_FR4:
		int_id = TIMER_SEC_QTMR_FR4_INT_ID;
		state_idx = TIMER_STATE_FR4;
		break;
	case TIMER_SEC_QTIMER_FR5:
		int_id = TIMER_SEC_QTMR_FR5_INT_ID;
		state_idx = TIMER_STATE_FR5;
		break;
	default:
		valid_tid = false;
		break;
	}

	if (!valid_tid) {
		ERROR("Qtimer start failed, invalid tid: %u\n", tid);
		return -1;
	}

	spin_lock(&cl_timer_handle.cl_timer_mutex[state_idx]);

	do {
		if (cl_timer_check_active(state_idx)) {
			ERROR("Qtimer start failed, already active, tid: %u\n", tid);
			ret = -1;
			break;
		}

		bl31qtilib_cb_set_int_targets(int_id, target_cpu);

		ret = timer_enable_int(tid);
		if (ret != 0) {
			ERROR("Qtimer start failed, int enable failed, tid: %u\n", tid);
			ret = -1;
			break;
		}

		ret = timer_one_shot_start(tid, timeout);
		if (ret != 0) {
			timer_disable_int(tid);
			ERROR("Qtimer start failed, tid: %u, ret: %d\n", tid, ret);
			ret = -1;
			break;
		}

		cl_timer_set_active(tid, true);
	} while (0);

	spin_unlock(&cl_timer_handle.cl_timer_mutex[state_idx]);

	return ret;
}

int tzbsp_cpu_cl_sleep_timer_cancel(timer_sec_id_t tid)
{
	int ret = 0;
	int err = 0;
	timer_state_t state_idx;
	bool valid_tid = true;

	if (!timer_is_tid_valid(tid)) {
		ERROR("Invalid qtimer frame: %u\n", tid);
		return -1;
	}

	if (!cl_timer_handle.cl_timer_init_done) {
		ERROR("Qtimer cancel failed, init not done, tid: %u\n", tid);
		return -1;
	}

	switch (tid) {
	case TIMER_SEC_QTIMER_FR3:
		state_idx = TIMER_STATE_FR3;
		break;
	case TIMER_SEC_QTIMER_FR4:
		state_idx = TIMER_STATE_FR4;
		break;
	case TIMER_SEC_QTIMER_FR5:
		state_idx = TIMER_STATE_FR5;
		break;
	default:
		valid_tid = false;
		break;
	}

	if (!valid_tid) {
		ERROR("Qtimer cancel failed, invalid tid: %u\n", tid);
		return -1;
	}

	spin_lock(&cl_timer_handle.cl_timer_mutex[state_idx]);

	err = timer_stop(tid);
	if (err != 0) {
		ERROR("Qtimer cancel failed, stop failed, tid: %u, err: %d\n",
		      tid, err);
		ret = -1;
	}

	err = timer_disable_int(tid);
	if (err != 0) {
		ERROR("Qtimer cancel failed, int disable failed, tid: %u, err: %d\n",
		      tid, err);
		ret = -1;
	}

	cl_timer_set_active(tid, false);
	spin_unlock(&cl_timer_handle.cl_timer_mutex[state_idx]);

	return ret;
}

static void *tzbsp_sleep_timer_isr(void *ctx)
{
	return NULL;
}

int tzbsp_cpu_cl_sleep_timer_init(void)
{
	int err = 0;

	if (cl_timer_handle.cl_timer_init_done) {
		return 0;
	}

	do {
		if (timer_is_tid_valid(TIMER_SEC_QTIMER_FR3)) {
			err = timer_install_isr(TIMER_SEC_QTIMER_FR3,
						tzbsp_sleep_timer_isr, NULL);
			if (err != 0) {
				ERROR("Qtimer FR3 ISR install failed, err: %d\n", err);
				err = -1;
				break;
			}
		}

		if (timer_is_tid_valid(TIMER_SEC_QTIMER_FR4)) {
			err = timer_install_isr(TIMER_SEC_QTIMER_FR4,
						tzbsp_sleep_timer_isr, NULL);
			if (err != 0) {
				ERROR("Qtimer FR4 ISR install failed, err: %d\n", err);
				err = -1;
				break;
			}
		}

		if (timer_is_tid_valid(TIMER_SEC_QTIMER_FR5)) {
			err = timer_install_isr(TIMER_SEC_QTIMER_FR5,
						tzbsp_sleep_timer_isr, NULL);
			if (err != 0) {
				ERROR("Qtimer FR5 ISR install failed, err: %d\n", err);
				err = -1;
				break;
			}
		}

		cl_timer_init();
	} while (0);

	return err;
}

int tzbsp_cpu_core_sleep_timer_init(void)
{
	uint32_t cur_cpu = plat_my_core_pos();

	if (core_timer_handle.core_timer_init_done[cur_cpu]) {
		return 0;
	}

	int err = timer_install_isr(TIMER_SEC_CP15, tzbsp_sleep_timer_isr, NULL);
	if (err != 0) {
		ERROR("CP15 timer ISR install failed, cpu: %u, err: %d\n",
		      cur_cpu, err);
		return -1;
	}

	core_timer_handle.core_timer_active[cur_cpu] = false;
	core_timer_handle.core_timer_init_done[cur_cpu] = true;
	return 0;
}

/*
 * Returns NCC specific secure qtimer frame bitmask. Frame 2 is protected
 * in Qtimer-0 & Qtimer-1 for all targets. For NCC targets, frames 3-5 are
 * also protected in Qtimer-0 from non-secure access.
 */
uint32_t timer_get_sec_qtmr_frames_bitmask(uint8_t qtimer_idx)
{
	switch (qtimer_idx) {
	case 0:
		return TZBSP_TIMER_NCC_SECURE_FRAMES;
	default:
		return 0x0U;
	}
}
