/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <lib/mmio.h>

#include <qti_interrupt_svc.h>
#include <drivers/qti/qtimer/qtimer_generic.h>
#include <qtimer_defs.h>

#define VOFF_FG0_LO_OFFSET(n)		(0x80 + 0x8 * (n))
#define VOFF_FG0_HI_OFFSET(n)		(0x84 + 0x8 * (n))
#define ACR_FG0_OFFSET(n)		(0x40 + 0x4 * (n))
#define SAR_FG0_OFFSET			(0x4)

#define QTIMER_SEC_FRAME		2

static void *cp15_timer_isr(void *ctx)
{
	return ctx;
}

static void *qtmr_frame2_isr(void *ctx)
{
	return ctx;
}

int qti_qtimer_platform_init(void)
{
	int ret;

	mmio_write_32(QTI_QTIMER_BASE + SAR_FG0_OFFSET, 0x7F);

	mmio_write_32(QTI_QTIMER_BASE + ACR_FG0_OFFSET(QTIMER_SEC_FRAME), 0x3F);
	mmio_write_32(QTI_QTIMER_BASE + VOFF_FG0_LO_OFFSET(QTIMER_SEC_FRAME), 0x0);
	mmio_write_32(QTI_QTIMER_BASE + VOFF_FG0_HI_OFFSET(QTIMER_SEC_FRAME), 0x0);

	dsbsy();
	isb();

	ret = qti_register_intr_type_el3(TIMER_SEC_CP15_INT_ID, cp15_timer_isr,
				   NULL);
	if (ret != 0) {
		return ret;
	}

	ret = qti_register_intr_type_el3(TIMER_SEC_QTMR_INT_ID, qtmr_frame2_isr,
				   NULL);
	if (ret != 0) {
		qti_unregister_intr_type_el3(TIMER_SEC_CP15_INT_ID);
		return ret;
	}

	return 0;
}
