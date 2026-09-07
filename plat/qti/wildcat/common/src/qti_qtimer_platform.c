/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <lib/mmio.h>

#include <drivers/qti/qgic/qgic.h>
#include <drivers/qti/qtimer/qtimer_generic.h>
#include <qtimer_defs.h>

#define VOFF_FG0_LO_OFFSET(n)		(0x80 + 0x8 * (n))
#define VOFF_FG0_HI_OFFSET(n)		(0x84 + 0x8 * (n))
#define ACR_FG0_OFFSET(n)		(0x40 + 0x4 * (n))
#define SAR_FG0_OFFSET			(0x4)

/*
 * Frame 2 (CP15/QTMR secure frame) is always configured. NCC frames (3/4/5)
 * are configured by qtimer_ncc.c on Wildcat NCC targets.
 */
#define QTIMER_SEC_FRAME		2

static void *cp15_timer_isr(void *ctx)
{
	return ctx;
}

static void *qtmr_frame2_isr(void *ctx)
{
	return ctx;
}

void qti_qtimer_platform_init(void)
{
	mmio_write_32(QTI_QTIMER_BASE + SAR_FG0_OFFSET, 0x7F);

	mmio_write_32(QTI_QTIMER_BASE + ACR_FG0_OFFSET(QTIMER_SEC_FRAME), 0x3F);
	mmio_write_32(QTI_QTIMER_BASE + VOFF_FG0_LO_OFFSET(QTIMER_SEC_FRAME), 0x0);
	mmio_write_32(QTI_QTIMER_BASE + VOFF_FG0_HI_OFFSET(QTIMER_SEC_FRAME), 0x0);

	dsbsy();
	isb();

	qti_gic_register_isr(TIMER_SEC_CP15_INT_ID, cp15_timer_isr, NULL);
	qti_gic_register_isr(TIMER_SEC_QTMR_INT_ID, qtmr_frame2_isr, NULL);
}
