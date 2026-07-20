/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <bl31qtilib_cb_interface.h>
#include <bl31qtilib_defs.h>
#include <bl31qtilib_interface.h>

#include <drivers/qti/timer/tzbsp_timer.h>
#include <drivers/qti/timer/timer_defs.h>

/*
 * QTimers run on 19.2 MHz, which is 19.2 times per us. Use of x10
 * in multiplier/divider allows more fixed point precision.
 */
#define us_to_ticks_qtimer(us)		((QTMR_FREQ_IN_100KHZ * (us)) / 10ULL)
#define ticks_to_us_qtimer(ticks)	(((ticks) * 10ULL) / QTMR_FREQ_IN_100KHZ)

#define LS32(xx)		((uint32_t)(xx))
#define MS32(xx)		((uint32_t)((xx) >> 32))

#define INVALID_INT_ID		0U

static uint32_t read_qtimer_lo_wrapper(timer_sec_id_t tid)
{
	uint32_t lo = 0;

	switch (tid) {
	case TIMER_SEC_QTIMER:
		lo = read_qtimer_lo();
		break;
	case TIMER_SEC_QTIMER_FR3:
		lo = read_qtimer_fr3_lo();
		break;
	case TIMER_SEC_QTIMER_FR4:
		lo = read_qtimer_fr4_lo();
		break;
	case TIMER_SEC_QTIMER_FR5:
		lo = read_qtimer_fr5_lo();
		break;
	default:
		break;
	}
	return lo;
}

static uint32_t read_qtimer_hi_wrapper(timer_sec_id_t tid)
{
	uint32_t hi = 0;

	switch (tid) {
	case TIMER_SEC_QTIMER:
		hi = read_qtimer_hi();
		break;
	case TIMER_SEC_QTIMER_FR3:
		hi = read_qtimer_fr3_hi();
		break;
	case TIMER_SEC_QTIMER_FR4:
		hi = read_qtimer_fr4_hi();
		break;
	case TIMER_SEC_QTIMER_FR5:
		hi = read_qtimer_fr5_hi();
		break;
	default:
		break;
	}
	return hi;
}

static void write_qtimer_wrapper(timer_sec_id_t tid, uint64_t ticks)
{
	switch (tid) {
	case TIMER_SEC_QTIMER:
		write_qtimer_lo(ticks);
		write_qtimer_hi(ticks);
		break;
	case TIMER_SEC_QTIMER_FR3:
		write_qtimer_fr3_lo(ticks);
		write_qtimer_fr3_hi(ticks);
		break;
	case TIMER_SEC_QTIMER_FR4:
		write_qtimer_fr4_lo(ticks);
		write_qtimer_fr4_hi(ticks);
		break;
	case TIMER_SEC_QTIMER_FR5:
		write_qtimer_fr5_lo(ticks);
		write_qtimer_fr5_hi(ticks);
		break;
	default:
		break;
	}
}

static void enable_qtimer_wrapper(timer_sec_id_t tid)
{
	switch (tid) {
	case TIMER_SEC_QTIMER:
		enable_qtimer();
		break;
	case TIMER_SEC_QTIMER_FR3:
		enable_qtimer_fr3();
		break;
	case TIMER_SEC_QTIMER_FR4:
		enable_qtimer_fr4();
		break;
	case TIMER_SEC_QTIMER_FR5:
		enable_qtimer_fr5();
		break;
	default:
		break;
	}
}

static void disable_qtimer_wrapper(timer_sec_id_t tid)
{
	switch (tid) {
	case TIMER_SEC_QTIMER:
		disable_qtimer();
		break;
	case TIMER_SEC_QTIMER_FR3:
		disable_qtimer_fr3();
		break;
	case TIMER_SEC_QTIMER_FR4:
		disable_qtimer_fr4();
		break;
	case TIMER_SEC_QTIMER_FR5:
		disable_qtimer_fr5();
		break;
	default:
		break;
	}
}

/* Returns Qtimer counter value handling carry-over between lo/hi reads. */
static uint64_t safe_read_cntpct(timer_sec_id_t tid)
{
	uint32_t hi = read_qtimer_hi_wrapper(tid);
	uint32_t lo = read_qtimer_lo_wrapper(tid);
	uint32_t hi2 = read_qtimer_hi_wrapper(tid);
	uint32_t lo2 = read_qtimer_lo_wrapper(tid);

	if (hi2 > hi) {
		/*
		 * A bit carried over from low 32 bits to high 32 bits during
		 * the reads. Grab the second lo/hi pair as its lo value could
		 * not have overflown in the time window between the reads.
		 */
		return ((uint64_t)hi2 << 32) | (uint64_t)lo2;
	}

	return ((uint64_t)hi << 32) | (uint64_t)lo;
}

static uint32_t tzbsp_get_sec_qtmr_frames_common(uint8_t qtimer_idx)
{
	uint32_t sec_frame_bitmask = 0U;

	switch (qtimer_idx) {
	case 0:
	case 1:
		/* Frame 2 is protected from non-secure access */
		sec_frame_bitmask |= 0x4U;
		sec_frame_bitmask |= timer_get_sec_qtmr_frames_bitmask(qtimer_idx);
		break;
	default:
		break;
	}

	return sec_frame_bitmask;
}

void tzbsp_timer_init(void)
{
	int ret = 0;
	uint8_t i, j;
	uint32_t base_addresses_for_qtmr[] = { APSS_PRIMARY_QTMR_BASE,
					       APSS_SECONDARY_QTMR_BASE };

	do {
		if (!bl31qtilib_is_cold_boot_done() || bl31qtilib_is_quick_boot()) {
			for (i = 0; i < NO_OF_QTIMERS_ONCHIP; i++) {
				uint32_t sec_frame_bitmask =
					tzbsp_get_sec_qtmr_frames_common(i);
				uint32_t max_frame_mask =
					(1U << NO_OF_QTIMER_FRAMES) - 1U;

				if (sec_frame_bitmask == 0U) {
					ERROR("Invalid qtimer index: %u\n", i);
					ret = -1;
					break;
				}

				/* Allow non-secure access only for non-secure frames */
				mmio_write_32(base_addresses_for_qtmr[i] +
					      APSS_QTMR_AC_CNTNSAR_FG0_ADDR_OFFSET,
					      sec_frame_bitmask ^ max_frame_mask);

				for (j = 0; j < NO_OF_QTIMER_FRAMES; j++) {
					if (!(sec_frame_bitmask & (1U << j))) {
						mmio_write_32(
							base_addresses_for_qtmr[i] +
							APSS_QTMR_AC_CNTACRn_FG0_ADDR_OFFSET(j),
							0x3FU);
					}

					/* Set Qtimer CNTVOFF to 0 for all frames */
					mmio_write_32(base_addresses_for_qtmr[i] +
						      APSS_QTMR_AC_CNTVOFF_FG0_LO_n_ADDR_OFFSET(j),
						      0x0U);
					mmio_write_32(base_addresses_for_qtmr[i] +
						      APSS_QTMR_AC_CNTVOFF_FG0_HI_n_ADDR_OFFSET(j),
						      0x0U);
				}
			}
		}

		/* CNTFRQ can only be written by EL3; configure on every boot */
		uint32_t cntfrq = mmio_read_32(APSS_PRIMARY_QTMR_BASE +
					       APSS_QTMR_AC_CNTFRQ_ADDR_OFFSET);

		write_cntfrq_el0(cntfrq);

		if (!bl31qtilib_is_cold_boot_done() &&
		    (tzbsp_cpu_cl_sleep_timer_init() != 0)) {
			ret = -1;
			break;
		}

		/* CP15 per-core sleep timer must be initialised on every boot */
		if (tzbsp_cpu_core_sleep_timer_init() != 0) {
			ret = -1;
			break;
		}

	} while (0);

	if (ret != 0) {
		plat_error_handler(-EINVAL);
	}
}

int timer_one_shot_start(timer_sec_id_t tid, uint64_t timeout)
{
	uint64_t ticks = 0;

	if (tid == TIMER_SEC_CP15) {
		write_cntps_ctl_el1(0x0);
		ticks = read_cntpct_el0() + us_to_ticks_qtimer(timeout);
		write_cntps_cval_el1(ticks);
		write_cntps_ctl_el1(0x1);
	} else if ((tid == TIMER_SEC_QTIMER) || (tid == TIMER_SEC_QTIMER_FR3) ||
		   (tid == TIMER_SEC_QTIMER_FR4) || (tid == TIMER_SEC_QTIMER_FR5)) {
		if (!timer_is_tid_valid(tid)) {
			ERROR("Invalid qtimer frame: %u\n", tid);
			return -1;
		}

		disable_qtimer_wrapper(tid);
		ticks = safe_read_cntpct(tid) + us_to_ticks_qtimer(timeout);
		write_qtimer_wrapper(tid, ticks);
		enable_qtimer_wrapper(tid);
	}

	return 0;
}

int timer_stop(timer_sec_id_t tid)
{
	if (!timer_is_tid_valid(tid)) {
		ERROR("Invalid qtimer frame: %u\n", tid);
		return -1;
	}

	switch (tid) {
	case TIMER_SEC_CP15:
		write_cntps_ctl_el1(0x0);
		break;
	case TIMER_SEC_QTIMER:
	case TIMER_SEC_QTIMER_FR3:
	case TIMER_SEC_QTIMER_FR4:
	case TIMER_SEC_QTIMER_FR5:
		disable_qtimer_wrapper(tid);
		break;
	default:
		return -1;
	}

	return 0;
}

uint64_t timer_get_count_in_us(timer_sec_id_t tid)
{
	if (!timer_is_tid_valid(tid)) {
		ERROR("Invalid qtimer frame: %u\n", tid);
		return -1;
	}

	switch (tid) {
	case TIMER_SEC_CP15:
		return ticks_to_us_qtimer(read_cntpct_el0());
	case TIMER_SEC_QTIMER:
	case TIMER_SEC_QTIMER_FR3:
	case TIMER_SEC_QTIMER_FR4:
	case TIMER_SEC_QTIMER_FR5:
		return ticks_to_us_qtimer(safe_read_cntpct(tid));
	default:
		return 0;
	}
}

int timer_install_isr(timer_sec_id_t tid, void *(*fn)(void *), void *ctx)
{
	int err = -1;

	if (!timer_is_tid_valid(tid)) {
		ERROR("Invalid qtimer frame: %u\n", tid);
		return -1;
	}

	switch (tid) {
	case TIMER_SEC_CP15:
		err = bl31qtilib_cb_int_register_isr(
			(uint32_t)TIMER_SEC_CP15_INT_ID, "CP15Tmr Sec", fn, ctx,
			BL31QTILIB_INTR_ROUTING_MODE_SELF, true);
		break;
	case TIMER_SEC_QTIMER:
		err = bl31qtilib_cb_int_register_isr(
			(uint32_t)TIMER_SEC_QTMR_INT_ID, "QTIMER Sec", fn, ctx,
			BL31QTILIB_INTR_ROUTING_MODE_ALL, true);
		break;
	case TIMER_SEC_QTIMER_FR3:
		err = bl31qtilib_cb_int_register_isr(
			(uint32_t)TIMER_SEC_QTMR_FR3_INT_ID, "QTIMER FR3", fn, ctx,
			BL31QTILIB_INTR_ROUTING_MODE_ALL, true);
		break;
	case TIMER_SEC_QTIMER_FR4:
		err = bl31qtilib_cb_int_register_isr(
			(uint32_t)TIMER_SEC_QTMR_FR4_INT_ID, "QTIMER FR4", fn, ctx,
			BL31QTILIB_INTR_ROUTING_MODE_ALL, true);
		break;
	case TIMER_SEC_QTIMER_FR5:
		err = bl31qtilib_cb_int_register_isr(
			(uint32_t)TIMER_SEC_QTMR_FR5_INT_ID, "QTIMER FR5", fn, ctx,
			BL31QTILIB_INTR_ROUTING_MODE_ALL, true);
		break;
	default:
		return -1;
	}

	return err;
}

int timer_enable_int(timer_sec_id_t tid)
{
	int retval = -1;
	uint32_t int_id = timer_get_timer_secure_int_id(tid);

	if (int_id != INVALID_INT_ID) {
		retval = bl31qtilib_cb_int_enable(int_id);
	}
	return retval;
}

int timer_disable_int(timer_sec_id_t tid)
{
	int retval = -1;
	uint32_t int_id = timer_get_timer_secure_int_id(tid);

	if (int_id != INVALID_INT_ID) {
		retval = bl31qtilib_cb_int_disable(int_id);
	}
	return retval;
}

uint32_t timer_get_timer_secure_int_id(timer_sec_id_t tid)
{
	if (!timer_is_tid_valid(tid)) {
		ERROR("Invalid qtimer frame: %u\n", tid);
		return -1;
	}

	switch (tid) {
	case TIMER_SEC_CP15:
		return (uint32_t)TIMER_SEC_CP15_INT_ID;
	case TIMER_SEC_QTIMER:
		return (uint32_t)TIMER_SEC_QTMR_INT_ID;
	case TIMER_SEC_QTIMER_FR3:
		return (uint32_t)TIMER_SEC_QTMR_FR3_INT_ID;
	case TIMER_SEC_QTIMER_FR4:
		return (uint32_t)TIMER_SEC_QTMR_FR4_INT_ID;
	case TIMER_SEC_QTIMER_FR5:
		return (uint32_t)TIMER_SEC_QTMR_FR5_INT_ID;
	default:
		return INVALID_INT_ID;
	}
}

bool timer_is_tid_valid(timer_sec_id_t tid)
{
	bool ret = false;
	uint32_t sec_frame_bitmask = 0U;

	if (tid > TIMER_SEC_MAX_TID) {
		return false;
	}

	switch (tid) {
	/* Common timers available on all targets */
	case TIMER_SEC_CP15:
	case TIMER_SEC_QTIMER:
		ret = true;
		break;
	/* Timers available only on NCC CPU based targets */
	case TIMER_SEC_QTIMER_FR3:
	case TIMER_SEC_QTIMER_FR4:
	case TIMER_SEC_QTIMER_FR5:
		sec_frame_bitmask = timer_get_sec_qtmr_frames_bitmask(0);
		if (sec_frame_bitmask != 0U) {
			/*
			 * tid + 1 converts timer_sec_id_t to actual qtimer
			 * frame number.
			 */
			ret = (sec_frame_bitmask &
			       (UINT32_C(1) << (tid + UINT32_C(1)))) != 0U;
		}
		break;
	default:
		break;
	}

	return ret;
}
