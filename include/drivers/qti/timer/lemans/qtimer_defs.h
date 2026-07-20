/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTIMER_DEFS_H
#define QTIMER_DEFS_H

#include <lemans_def.h>

/* QTimer AC base and frame offsets */
#define APSS_PRIMARY_QTMR_BASE			QTI_QTIMER_BASE
#define APSS_SECONDARY_QTMR_BASE		0

#define PRIMARY_QTMR_FRAME2_OFFSET_FROM_BASE	0x5000
#define PRIMARY_QTMR_FRAME3_OFFSET_FROM_BASE	0x7000
#define PRIMARY_QTMR_FRAME4_OFFSET_FROM_BASE	0x9000
#define PRIMARY_QTMR_FRAME5_OFFSET_FROM_BASE	0xB000

/* Topology */
#define NO_OF_QTIMERS_ONCHIP			1
#define NO_OF_QTIMER_FRAMES			7

/* Interrupt IDs */
#define TIMER_SEC_CP15_INT_ID			29
#define TIMER_SEC_QTMR_INT_ID			42
#define TIMER_SEC_QTMR_FR3_INT_ID		43
#define TIMER_SEC_QTMR_FR4_INT_ID		44
#define TIMER_SEC_QTMR_FR5_INT_ID		45

/*
 * NCC secure frame bitmask: frames 3 and 4 (bits 3,4) reserved for cluster
 * sleep timers. Frame 2 (bit 2) always secure.
 */
#define TZBSP_TIMER_NCC_SECURE_FRAMES		0x18U

/* Number of CPUs */
#define TZBSP_CPU_COUNT				8U

#endif /* QTIMER_DEFS_H */
