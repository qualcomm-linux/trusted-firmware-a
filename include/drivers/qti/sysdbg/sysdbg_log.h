/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * System Debug Logging Interface
 *
 * Ported from: TZ.XF.5.36-00138-SHIKRA-1
 * trustzone_images/core/api/systemdebug/sysdbg_log.h
 */

#ifndef SYSDBG_LOG_H
#define SYSDBG_LOG_H

#include <stdint.h>
#include <drivers/qti/sysdbg/sysdbg_crumbs.h>

/* Main API or Macro to be used for logging */
#define SYSDBG_LOG_EVENT(id, arg1, arg2, arg3) sysdbg_log_event_id4(id, arg1, arg2, arg3)

/*
 * Cookie values and SDI markers are defined in sysdbg_crumbs.h:
 * - SYSDBG_MAIN_ENTRY_START_COOKIE
 * - SYSDBG_MAIN_ENTRY_END_COOKIE
 * - SYSDBG_DISABLE_COOKIE
 * - SYSDBG_SET_DEBUG_MARKER macro
 * - SYSTEM_SHAREDIMEM_BASE
 */

/* SDI pass#1 debug markers */
#define SYSDBG_WARMBOOT_MARKER_ADDR    (SYSTEM_SHAREDIMEM_BASE + 0xb14)

/* Log buffer size - can be overridden before including this header */
#ifndef SYSDBG_LOG_SIZE
#define SYSDBG_LOG_SIZE 128
#endif

/* Enumeration containing unique ids for every "event" to be logged */
typedef enum {
	INVALID_EVENT = 0x0,

	/* DBI Boot events */
	BOOT_STARTED = 0x1,
	BOOT_ENDED = 0x2,

	/* Fatal error */
	FATAL_ERROR = 0x3,

	/* DBI Pass information */
	PASS_ENTER = 0x4,
	PASS_EXIT = 0x5,

	/* Events for entering and exiting a DBI function in a pass */
	ENTER_FCN = 0x6,
	EXIT_FCN = 0x7,

	/* Sleep event */
	ENTER_SLEEP = 0x8,
	EXIT_SLEEP = 0x9,

	/* Generic event */
	GENERIC_EVENT = 0xA,

	/* Register dump */
	REG_DUMP = 0xB,

	EVENT_END = 0x7FFFFFFF,
} sysdbg_event_type;

/* Structure for logging info */
typedef struct {
	sysdbg_event_type id;
	uint64_t arg1;
	uint64_t arg2;
	uint64_t arg3;
} sysdbg_log_entry_type;

/* Main Buffer that contains all the logs */
typedef struct {
	/* Index into the buffer */
	volatile uint32_t index;
	sysdbg_log_entry_type buffer[SYSDBG_LOG_SIZE];
	/* Additional register logging */
	void *register_config;
} sysdbg_buffer_type;

/**
 * sysdbg_log_init() - Initialize the logging buffer for the DBI
 */
void sysdbg_log_init(void);

/**
 * sysdbg_log_event_id4() - Log an entry into the buffer
 *
 * This API should NOT be used externally. Use SYSDBG_LOG_EVENT instead.
 *
 * @param event - Event ID to be logged
 * @param arg1 - First argument
 * @param arg2 - Second argument
 * @param arg3 - Third argument
 */
void sysdbg_log_event_id4(sysdbg_event_type event, uint64_t arg1, uint64_t arg2, uint64_t arg3);

/**
 * sysdbg_log_deinit() - De-initialize the logging buffer
 *
 * Saves useful information to another memory location.
 */
void sysdbg_log_deinit(void);

#endif /* SYSDBG_LOG_H */
