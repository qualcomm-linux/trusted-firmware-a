/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTIMER_DEFS_H
#define QTIMER_DEFS_H

/*
 * APSS_QTMR0_QTMR_AC base -> QTI_QTIMER_BASE (AC regs); the frame-2 counter
 * view sits at a fixed offset above the AC base (nord layout differs from
 * other family members). Secure timer interrupt IDs below are confirmed
 * against the platform interrupt map.
 *   QTMR_qgicFrm2PhyIrq[0] = 72 (secure frame 2)
 *   QTMR_qgicFrm3PhyIrq[0] = 73 (NCC frame 3)
 *   QTMR_qgicFrm4PhyIrq[0] = 74 (NCC frame 4)
 *   QTMR_qgicFrm5PhyIrq[0] = 75 (NCC frame 5)
 * CP15 secure physical timer is architectural PPI 29 (unchanged).
 */
#define QTI_QTIMER_BASE			0x17810000

#define QTIMER0_F2V1_BASE_ADDRESS	QTI_QTIMER_BASE + 0x5000

/* NCC cluster sleep-timer frames use a 0x2000-byte frame stride. */
#define QTIMER0_F3V1_BASE_ADDRESS	QTI_QTIMER_BASE + 0x7000
#define QTIMER0_F4V1_BASE_ADDRESS	QTI_QTIMER_BASE + 0x9000
#define QTIMER0_F5V1_BASE_ADDRESS	QTI_QTIMER_BASE + 0xb000

#define QTIMER_NBR_FRAMES		7

/* nord is Wildcat NCC: frame-2 secure interrupts plus NCC frames 3/4/5. */
#define TIMER_SEC_CP15_INT_ID		29
#define TIMER_SEC_QTMR_INT_ID		72
#define TIMER_SEC_QTMR_FR3_INT_ID	73
#define TIMER_SEC_QTMR_FR4_INT_ID	74
#define TIMER_SEC_QTMR_FR5_INT_ID	75

/*
 * NCC-specific secure QTimer frame bitmask (frame index mapped LSB->MSB,
 * i.e. bit N corresponds to frame N). This tracks which of frames 3/4/5 are
 * implemented, one per cluster.
 */
#define QTI_QTIMER_NCC_SECURE_FRAMES	0x38

#endif /* QTIMER_DEFS_H */
