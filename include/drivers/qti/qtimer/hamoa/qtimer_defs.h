/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTIMER_DEFS_H
#define QTIMER_DEFS_H

/*
 * QTI_QTIMER_BASE addresses the access-control registers. The frame 2
 * counter view is located at offset 0x4000.
 */
#define QTI_QTIMER_BASE			0xF9020000

#define QTIMER0_F2V1_BASE_ADDRESS	QTI_QTIMER_BASE + 0x4000

/* NCC cluster sleep-timer frames use a 0x2000-byte frame stride. */
#define QTIMER0_F3V1_BASE_ADDRESS	QTI_QTIMER_BASE + 0x6000
#define QTIMER0_F4V1_BASE_ADDRESS	QTI_QTIMER_BASE + 0x8000
#define QTIMER0_F5V1_BASE_ADDRESS	QTI_QTIMER_BASE + 0xa000

#define QTIMER_NBR_FRAMES		7

/* hamoa is Wildcat NCC: frame-2 secure interrupts plus NCC frames 3/4/5. */
#define TIMER_SEC_CP15_INT_ID		29
#define TIMER_SEC_QTMR_INT_ID		42
#define TIMER_SEC_QTMR_FR3_INT_ID	43
#define TIMER_SEC_QTMR_FR4_INT_ID	44
#define TIMER_SEC_QTMR_FR5_INT_ID	45

#endif /* QTIMER_DEFS_H */
