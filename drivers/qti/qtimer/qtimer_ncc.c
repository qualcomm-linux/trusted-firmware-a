/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <arch_helpers.h>
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <platform.h>

#include <drivers/qti/qgic/qgic.h>
#include <drivers/qti/qtimer/qtimer_generic.h>
#include <qtimer_defs.h>

#define VOFF_FG0_LO_OFFSET(n)		(0x80 + 0x8 * (n))
#define VOFF_FG0_HI_OFFSET(n)		(0x84 + 0x8 * (n))
#define ACR_FG0_OFFSET(n)		(0x40 + 0x4 * (n))

#define QTIMER_NCC_FRAME_FR3		3
#define QTIMER_NCC_FRAME_FR4		4
#define QTIMER_NCC_FRAME_FR5		5

/*
 * Per-cluster-frame sleep-timer state. One entry per NCC QTimer frame
 * (3/4/5); guarded by its own lock so a frame's start/cancel calls issued
 * concurrently by different cores in the same cluster serialise correctly.
 */
typedef enum {
	NCC_FRAME_STATE_FR3 = 0,
	NCC_FRAME_STATE_FR4 = 1,
	NCC_FRAME_STATE_FR5 = 2,
	NCC_FRAME_STATE_CNT = 3,
} qti_qtimer_ncc_frame_state_idx_t;

static struct {
	bool active[NCC_FRAME_STATE_CNT];
	spinlock_t lock[NCC_FRAME_STATE_CNT];
	bool init_done;
} cl_timer_handle;

/*
 * Per-core CP15 sleep-timer state, sized to the platform's core count.
 * There is no shared mutable state across cores for the CP15 timer (each
 * core owns its own EL3 physical secure timer sysregs), so no lock is
 * needed - only this core's own array slot is ever touched.
 */
static struct {
	bool active[PLATFORM_CORE_COUNT];
	bool init_done[PLATFORM_CORE_COUNT];
} core_timer_handle;

static int qti_qtimer_ncc_frame_state_idx(qti_qtimer_id_t tid)
{
	switch (tid) {
	case QTI_TID_QTMR_FR3:
		return NCC_FRAME_STATE_FR3;
	case QTI_TID_QTMR_FR4:
		return NCC_FRAME_STATE_FR4;
	case QTI_TID_QTMR_FR5:
		return NCC_FRAME_STATE_FR5;
	default:
		return -1;
	}
}

/*
 * Wake ISR for the cluster (FR3/FR4/FR5) and per-core (CP15) sleep timers.
 * This is intentionally a no-op: the timer only fires the SPI/PPI that wakes
 * a sleeping core from WFI, so no additional servicing is required.
 */
static void *qti_qtimer_ncc_sleep_isr(void *ctx)
{
	return ctx;
}

static void ncc_frame_config(uint8_t frame)
{
	mmio_write_32(QTI_QTIMER_BASE + ACR_FG0_OFFSET(frame), 0x3F);
	mmio_write_32(QTI_QTIMER_BASE + VOFF_FG0_LO_OFFSET(frame), 0x0);
	mmio_write_32(QTI_QTIMER_BASE + VOFF_FG0_HI_OFFSET(frame), 0x0);
}

/*
 * NCC extension: configures the additional secure frames (3/4/5) present on
 * NCC (multi-cluster) targets. Compiled in only under QTI_USE_NCC_QTIMER.
 * Interrupt registration always goes through the qgic shim, never the
 * family-native interrupt-service API directly.
 */
void qti_qtimer_ncc_init(void)
{
	ncc_frame_config(QTIMER_NCC_FRAME_FR3);
	ncc_frame_config(QTIMER_NCC_FRAME_FR4);
	ncc_frame_config(QTIMER_NCC_FRAME_FR5);

	dsbsy();
	isb();

	qti_gic_register_isr(TIMER_SEC_QTMR_FR3_INT_ID, qti_qtimer_ncc_sleep_isr, NULL);
	qti_gic_register_isr(TIMER_SEC_QTMR_FR4_INT_ID, qti_qtimer_ncc_sleep_isr, NULL);
	qti_gic_register_isr(TIMER_SEC_QTMR_FR5_INT_ID, qti_qtimer_ncc_sleep_isr, NULL);
}

/*
 * ---------------------------------------------------------------------------
 * Cluster (multi-cluster NCC QTimer frame 3/4/5) sleep/wake one-shot timer
 * service.
 *
 * The service is available to platform suspend implementations that require
 * a cluster wake timer.
 * ---------------------------------------------------------------------------
 */

/*
 * Installs the wake ISR for each present NCC QTimer frame. Must be called
 * once (cold boot) before any qti_qtimer_ncc_cl_sleep_timer_start() call.
 */
int qti_qtimer_ncc_cl_sleep_timer_init(void)
{
	int ret;

	if (cl_timer_handle.init_done) {
		return 0;
	}

	ret = qti_qtimer_install_isr(QTI_TID_QTMR_FR3, qti_qtimer_ncc_sleep_isr, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = qti_qtimer_install_isr(QTI_TID_QTMR_FR4, qti_qtimer_ncc_sleep_isr, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = qti_qtimer_install_isr(QTI_TID_QTMR_FR5, qti_qtimer_ncc_sleep_isr, NULL);
	if (ret != 0) {
		return ret;
	}

	cl_timer_handle.init_done = true;

	return 0;
}

/*
 * Starts a cluster (NCC frame 3/4/5) sleep timer, routing its wake
 * interrupt to target_cpu and arming it to expire once after timeout_us.
 */
int qti_qtimer_ncc_cl_sleep_timer_start(qti_qtimer_id_t tid, uint64_t timeout_us,
					 u_register_t target_cpu_mpidr)
{
	int ret = 0;
	int state_idx = qti_qtimer_ncc_frame_state_idx(tid);
	uint32_t int_id;

	if (state_idx < 0 || !qti_qtimer_is_tid_valid(tid)) {
		return -EINVAL;
	}

	if (!cl_timer_handle.init_done) {
		return -EPERM;
	}

	int_id = qti_qtimer_get_secure_int_id(tid);

	spin_lock(&cl_timer_handle.lock[state_idx]);

	do {
		if (cl_timer_handle.active[state_idx]) {
			ret = -EALREADY;
			break;
		}

		/* Route the wake interrupt to the target core. */
		qti_gic_set_spi_routing(int_id, GICV3_IRM_PE, target_cpu_mpidr);

		ret = qti_qtimer_enable_int(tid);
		if (ret != 0) {
			break;
		}

		ret = qti_qtimer_one_shot_start(tid, timeout_us);
		if (ret != 0) {
			qti_qtimer_disable_int(tid);
			break;
		}

		cl_timer_handle.active[state_idx] = true;
	} while (0);

	spin_unlock(&cl_timer_handle.lock[state_idx]);

	return ret;
}

/*
 * Cancels (stops and disables the interrupt for) a previously-started
 * cluster sleep timer.
 */
int qti_qtimer_ncc_cl_sleep_timer_cancel(qti_qtimer_id_t tid)
{
	int ret = 0;
	int err;
	int state_idx = qti_qtimer_ncc_frame_state_idx(tid);

	if (state_idx < 0 || !qti_qtimer_is_tid_valid(tid)) {
		return -EINVAL;
	}

	if (!cl_timer_handle.init_done) {
		return -EPERM;
	}

	spin_lock(&cl_timer_handle.lock[state_idx]);

	err = qti_qtimer_stop(tid);
	if (err != 0) {
		ret = err;
	}

	err = qti_qtimer_disable_int(tid);
	if (err != 0) {
		ret = err;
	}

	cl_timer_handle.active[state_idx] = false;

	spin_unlock(&cl_timer_handle.lock[state_idx]);

	return ret;
}

/*
 * ---------------------------------------------------------------------------
 * Per-core CP15 physical secure timer sleep/wake one-shot service.
 * ---------------------------------------------------------------------------
 */

/*
 * Installs the wake ISR for the CP15 timer. Multi-client (per-core) ISR
 * registration is not required; the ISR is installed once at cold boot.
 */
int qti_qtimer_ncc_core_sleep_timer_init(void)
{
	unsigned int core = plat_my_core_pos();
	int ret;

	if (core_timer_handle.init_done[core]) {
		return 0;
	}

	ret = qti_qtimer_install_isr(QTI_TID_CP15, qti_qtimer_ncc_sleep_isr, NULL);
	if (ret != 0) {
		return ret;
	}

	core_timer_handle.active[core] = false;
	core_timer_handle.init_done[core] = true;

	return 0;
}

/* Starts this core's CP15 sleep timer to expire once after timeout_us. */
int qti_qtimer_ncc_core_sleep_timer_start(uint64_t timeout_us)
{
	unsigned int core = plat_my_core_pos();
	int ret;

	if (!core_timer_handle.init_done[core]) {
		return -EPERM;
	}

	if (core_timer_handle.active[core]) {
		return -EALREADY;
	}

	ret = qti_qtimer_enable_int(QTI_TID_CP15);
	if (ret != 0) {
		return ret;
	}

	ret = qti_qtimer_one_shot_start(QTI_TID_CP15, timeout_us);
	if (ret != 0) {
		qti_qtimer_disable_int(QTI_TID_CP15);
		return ret;
	}

	core_timer_handle.active[core] = true;

	return 0;
}

/* Cancels this core's CP15 sleep timer. */
int qti_qtimer_ncc_core_sleep_timer_cancel(void)
{
	unsigned int core = plat_my_core_pos();
	int ret = 0;
	int err;

	if (!core_timer_handle.init_done[core]) {
		return -EPERM;
	}

	err = qti_qtimer_stop(QTI_TID_CP15);
	if (err != 0) {
		ret = err;
	}

	err = qti_qtimer_disable_int(QTI_TID_CP15);
	if (err != 0) {
		ret = err;
	}

	core_timer_handle.active[core] = false;

	return ret;
}
