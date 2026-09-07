/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <arch_helpers.h>
#include <drivers/qti/qtimer/qtimer.h>
#include <lib/mmio.h>

#include <drivers/qti/qgic/qgic.h>
#include <drivers/qti/qtimer/qtimer_generic.h>
#include <qtimer_defs.h>

#define	QTIMER_FREQ_IN_100KHZ		192ULL

#define us_to_ticks_qtimer(us)		((QTIMER_FREQ_IN_100KHZ * (us)) / 10ULL)
#define ticks_to_us_qtimer(ticks)	(((ticks) * 10ULL) / QTIMER_FREQ_IN_100KHZ)

#define INVALID_QTIMER_INT_ID		((uint32_t)0x0)

/*
 * Return the CNTBaseN frame base address for a validated, non-CP15 timer.
 * Frames 3 through 5 are only available on NCC targets.
 */
static uintptr_t qtimer_frame_base(qti_qtimer_id_t tid)
{
	switch (tid) {
	case QTI_TID_QTMR:
		return QTIMER0_F2V1_BASE_ADDRESS;
#if QTI_USE_NCC_QTIMER
	case QTI_TID_QTMR_FR3:
		return QTIMER0_F3V1_BASE_ADDRESS;
	case QTI_TID_QTMR_FR4:
		return QTIMER0_F4V1_BASE_ADDRESS;
	case QTI_TID_QTMR_FR5:
		return QTIMER0_F5V1_BASE_ADDRESS;
#endif
	default:
		return 0U;
	}
}

/* Disables (stops) the given frame's physical timer. */
static void qtimer_frame_disable(qti_qtimer_id_t tid)
{
	mmio_write_32(qtimer_frame_base(tid) + CNTP_CTL, 0U);
}

/* Enables the given frame's physical timer. */
static void qtimer_frame_enable(qti_qtimer_id_t tid)
{
	mmio_write_32(qtimer_frame_base(tid) + CNTP_CTL, CNTP_CTL_ENABLE_BIT);
}

/* Reads the frame's 64-bit physical counter (CNTPCT). */
static uint64_t qtimer_frame_read_safe(qti_qtimer_id_t tid)
{
	return mmio_read_64(qtimer_frame_base(tid) + CNTPCT_LO);
}

/* Programs the frame's 64-bit physical compare value (CNTP_CVAL). */
static void qtimer_frame_write(qti_qtimer_id_t tid, uint64_t ticks)
{
	mmio_write_64(qtimer_frame_base(tid) + CNTP_CVAL_LO, ticks);
}

/*
 * Check whether a timer identifier is valid for the selected platform.
 * CP15 and frame 2 are always available; frames 3 through 5 require NCC.
 */
bool qti_qtimer_is_tid_valid(qti_qtimer_id_t tid)
{
	switch (tid) {
	case QTI_TID_CP15:
	case QTI_TID_QTMR:
		return true;
#if QTI_USE_NCC_QTIMER
	case QTI_TID_QTMR_FR3:
	case QTI_TID_QTMR_FR4:
	case QTI_TID_QTMR_FR5:
		return true;
#endif
	default:
		return false;
	}
}

/*
 * Return the secure interrupt ID associated with a timer identifier.
 */
uint32_t qti_qtimer_get_secure_int_id(qti_qtimer_id_t tid)
{
	if (!qti_qtimer_is_tid_valid(tid)) {
		return INVALID_QTIMER_INT_ID;
	}

	switch (tid) {
	case QTI_TID_CP15:
		return (uint32_t)TIMER_SEC_CP15_INT_ID;
	case QTI_TID_QTMR:
		return (uint32_t)TIMER_SEC_QTMR_INT_ID;
#if QTI_USE_NCC_QTIMER
	case QTI_TID_QTMR_FR3:
		return (uint32_t)TIMER_SEC_QTMR_FR3_INT_ID;
	case QTI_TID_QTMR_FR4:
		return (uint32_t)TIMER_SEC_QTMR_FR4_INT_ID;
	case QTI_TID_QTMR_FR5:
		return (uint32_t)TIMER_SEC_QTMR_FR5_INT_ID;
#endif
	default:
		return INVALID_QTIMER_INT_ID;
	}
}

/*
 * Install an ISR for the given timer through the QGIC interface.
 */
int qti_qtimer_install_isr(qti_qtimer_id_t tid, qti_qtimer_isr_t fn, void *ctx)
{
	uint32_t int_id = qti_qtimer_get_secure_int_id(tid);

	if (int_id == INVALID_QTIMER_INT_ID) {
		return -EINVAL;
	}

	return qti_gic_register_isr(int_id, fn, ctx);
}

int qti_qtimer_enable_int(qti_qtimer_id_t tid)
{
	uint32_t int_id = qti_qtimer_get_secure_int_id(tid);

	if (int_id == INVALID_QTIMER_INT_ID) {
		return -EINVAL;
	}

	qti_gic_enable_intr(int_id);
	return 0;
}

int qti_qtimer_disable_int(qti_qtimer_id_t tid)
{
	uint32_t int_id = qti_qtimer_get_secure_int_id(tid);

	if (int_id == INVALID_QTIMER_INT_ID) {
		return -EINVAL;
	}

	qti_gic_disable_intr(int_id);
	return 0;
}

/*
 * Starts a one-shot timer: reads the current counter, adds the requested
 * timeout (converted to ticks), and programs + enables the compare value.
 * CP15 uses the architectural EL3 secure physical timer sysregs; the QTMR
 * frames use their frame-local CNTP_CVAL/CNTP_CTL registers.
 */
int qti_qtimer_one_shot_start(qti_qtimer_id_t tid, uint64_t timeout_us)
{
	uint64_t ticks;

	if (!qti_qtimer_is_tid_valid(tid)) {
		return -EINVAL;
	}

	if (tid == QTI_TID_CP15) {
		write_cntps_ctl_el1(0U);
		ticks = read_cntpct_el0() + us_to_ticks_qtimer(timeout_us);
		write_cntps_cval_el1(ticks);
		write_cntps_ctl_el1(1U);
		return 0;
	}

	qtimer_frame_disable(tid);
	ticks = qtimer_frame_read_safe(tid) + us_to_ticks_qtimer(timeout_us);
	qtimer_frame_write(tid, ticks);
	qtimer_frame_enable(tid);

	return 0;
}

/** Stops (disables) the given one-shot timer. */
int qti_qtimer_stop(qti_qtimer_id_t tid)
{
	if (!qti_qtimer_is_tid_valid(tid)) {
		return -EINVAL;
	}

	if (tid == QTI_TID_CP15) {
		write_cntps_ctl_el1(0U);
		return 0;
	}

	qtimer_frame_disable(tid);
	return 0;
}

/*
 * Core driver: hardware-agnostic dispatch only. AC register programming and
 * interrupt registration live in the per-family platform timer service
 * (qti_qtimer_platform_init) and, for NCC targets, the NCC extension
 * (qti_qtimer_ncc_init). This function must not call
 * generic_delay_timer_init(); platform bl31_setup() owns that call.
 */
void qti_qtimer_init(void)
{
	qti_qtimer_platform_init();

#if QTI_USE_NCC_QTIMER
	qti_qtimer_ncc_init();
#endif
}

uint64_t qti_qtimer_get_raw(void)
{
	/* Assumes atomics reads */
	return mmio_read_64(QTIMER0_F2V1_BASE_ADDRESS);
}

uint64_t qti_qtimer_get_usec(void)
{
	uint64_t ticks = qti_qtimer_get_raw();

	return ((ticks * 10ULL) / QTIMER_FREQ_IN_100KHZ);
}

