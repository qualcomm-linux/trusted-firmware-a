/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * System Debug Crumbs Header
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/inc/sysdbg_crumbs.h
 *
 * This header defines all the debug markers used in System Debug driver.
 * It is expected to work with assembly and C files.
 */

#ifndef SYSDBG_CRUMBS_H
#define SYSDBG_CRUMBS_H

/* SYSTEM_SHAREDIMEM_BASE comes from the chipset's sysdbg_target_config.h */
#include <sysdbg_target_config.h>

/* Offsets in Shared IMEM for various fields */
/* 8 bytes for dump table offset */
#define SYSDBG_DUMPTABLE_OFFSET       0x010
/* 8 bytes for pointer to the log buffer */
#define SYSDBG_LOGBUFPTR_OFFSET       0xAF8
/* 8 bytes for DCC logging */
#define SYSDBG_DCCLOGPTR_OFFSET       0xB00
/* 4 bytes for PMIC data struct pointer */
#define SYSDBG_PMICLOGPTR_OFFSET      0xB08
/* 8 bytes for future use */
#define SYSDBG_RESERVED_OFFSET        0xB0C
/* Cold boot completion reason offset */
#define SYSDBG_COLDBOOTST_OFFSET      0xB14
/* Pass 1 completion reason offset */
#define SYSDBG_PASS1COMPL_OFFSET      (SYSDBG_COLDBOOTST_OFFSET)

/* Magic numbers used in various scenarios to indicate status */
#define SYSDBG_EXCEPTION_COOKIE                 0xCAFEBABE
#define SYSDBG_BADWAKEUP_COOKIE                 0x5D100D0A
#define SYSDBG_DDRFAIL_COOKIE                   0xCAB0DEAD
#define SYSDBG_THERMFAIL_COOKIE                 0x6D74DEAD
#define SYSDBG_BOOT_SUCCESS_COOKIE              0x000005D1
#define SYSDBG_PASS1_SUCCESS_COOKIE             0xDEADD00D
#define SYSDBG_TZ_DIAG_ENCRYPT_SUCCESS_COOKIE   0xDEAD0075
#define SYSDBG_MAIN_ENTRY_START_COOKIE          0x5D1E1302
#define SYSDBG_MAIN_ENTRY_END_COOKIE            0x5D1E1303
#define SYSDBG_DISABLE_COOKIE                   0x5AB1E5D1

/* Various markers */
#define SYSDBG_ENTRY_HANDLER_MARKER1  0x0000FFFA
#define SYSDBG_ENTRY_HANDLER_MARKER2  0x0000FFF9
#define SYSDBG_ENTRY_HANDLER_MARKER3  0x0000FFF8
#define SYSDBG_ENTRY_HANDLER_MARKER4  0x0000FFF7
#define SYSDBG_ENTRY_MARKER1          0x0000FFAF
#define SYSDBG_ENTRY_MARKER2          0x0000FF9F
#define SYSDBG_ENTRY_MARKER3          0x0000FF8F
#define SYSDBG_ENTRY_MARKER4          0x0000FF7F

/* Other legacy IDs */
#define SYSTEM_DEBUG_IMAGE_ID         0x000005D1

/* Debug marker macros */
#define SYSDBG_SET_DEBUG_MARKER(x) \
	do { \
		volatile uint32_t *marker = (uint32_t *)(SYSTEM_SHAREDIMEM_BASE + SYSDBG_PASS1COMPL_OFFSET); \
		*marker = (x); \
	} while (0)

#endif /* SYSDBG_CRUMBS_H */
