/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef QTI_BOARD_DEF_H
#define QTI_BOARD_DEF_H

/*
 * Required platform porting definitions common to all ARM
 * development platforms
 */

/*
 * Defines used to retrieve QTI SOC Version
 */
#define JEDEC_QTI_BKID	U(0x0)
#define JEDEC_QTI_MFID	U(0x70)
#define QTI_SOC_CONTINUATION_SHIFT	U(24)
#define QTI_SOC_IDENTIFICATION_SHIFT	U(16)

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE	0x1000

/*
 * Maximum mmap region capacity required by the supported QTI platforms.
 * Individual platforms may use fewer entries.
 */
#define PLAT_QTI_MMAP_ENTRIES	19

/*
 * Common platform specific system counter frequency
 */
#define PLAT_SYSCNT_FREQ	UL(19200000)

#endif /* QTI_BOARD_DEF_H */
