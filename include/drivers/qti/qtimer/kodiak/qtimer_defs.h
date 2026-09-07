/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTIMER_DEFS_H
#define QTIMER_DEFS_H

#include "kodiak_def.h"

#define QTIMER0_F2V1_BASE_ADDRESS	QTI_QTIMER_BASE + 0x5000

#define QTIMER_NBR_FRAMES		7

/* kodiak is non-NCC: only the CP15/QTMR frame-2 secure interrupts exist. */
#define TIMER_SEC_CP15_INT_ID		29
#define TIMER_SEC_QTMR_INT_ID		42

#endif /* QTIMER_DEFS_H */
