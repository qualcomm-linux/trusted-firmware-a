/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTIMER_DEFS_H
#define QTIMER_DEFS_H

#include "nord_def.h"

/*
 * Nord primary QTimer (APSS_PRIMARY_QTMR_BASE = 0x17810000).
 * Frame 2 is the general TZ frame at offset +0x5000 -> 0x17815000.
 * The chip exposes 7 frames.
 */
#define QTIMER0_F2V1_BASE_ADDRESS	QTI_QTIMER_BASE + 0x5000

#define QTIMER_NBR_FRAMES		7

#endif /* QTIMER_DEFS_H */
