/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <drivers/delay_timer.h>
#include <plat/common/platform.h>

#include <drivers/qti/timer/qti_timer.h>
#include <drivers/qti/timer/tzbsp_timer.h>
#include <drivers/qti/timer/timer_defs.h>

#define QTIMER_CNT_MSK		0x00000000ffffffffULL

static timer_ops_t ops;

static inline uint64_t us_to_ticks_qtimer(uint64_t us)
{
	return (QTMR_FREQ_IN_100KHZ * us) / 10ULL;
}

static uint64_t qti_delay_timeout_init_us(uint32_t us)
{
	uint64_t cnt = us_to_ticks_qtimer(us);

	cnt += us_to_ticks_qtimer(timer_get_count_in_us(TIMER_SEC_QTIMER));
	return cnt;
}

static bool qti_delay_timeout_elapsed(uint64_t expire_cnt)
{
	return us_to_ticks_qtimer(timer_get_count_in_us(TIMER_SEC_QTIMER)) >
	       expire_cnt;
}

static uint32_t qti_delay_get_timer_value(void)
{
	/*
	 * Generic delay timer implementation expects the timer to be a down
	 * counter. We apply bitwise NOT operator to the tick values returned
	 * by safe_read_cntpct() to simulate the down counter. The value is
	 * clipped from 64 to 32 bits.
	 */
	uint64_t cnt = us_to_ticks_qtimer(timer_get_count_in_us(TIMER_SEC_QTIMER));

	return (uint32_t)(~(cnt & QTIMER_CNT_MSK));
}

static void qti_delay_timer_init_args(uint32_t mult, uint32_t div)
{
	ops.get_timer_value = qti_delay_get_timer_value;
	ops.clk_mult = mult;
	ops.clk_div = div;
	ops.timeout_init_us = qti_delay_timeout_init_us;
	ops.timeout_elapsed = qti_delay_timeout_elapsed;

	timer_init(&ops);
}

void qti_timer_init(void)
{
	tzbsp_timer_init();
}

void qti_delay_timer_init(void)
{
	unsigned int mult = 1000000;
	unsigned int div = plat_get_syscnt_freq2();

	while (((mult % 10U) == 0U) && ((div % 10U) == 0U)) {
		mult /= 10U;
		div /= 10U;
	}

	qti_delay_timer_init_args(mult, div);
}
