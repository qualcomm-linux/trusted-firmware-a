/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_LOG_H
#define SYSDBG_LOG_H

#include <stdint.h>

#define SYSDBG_LOG_SIZE	48

#define SYSDBG_LOG_EVENT(id, arg1, arg2, arg3)	sysdbg_log_event_id4(id, arg1, arg2, arg3)

typedef enum {
	INVALID_EVENT = 0x0,
	BOOT_STARTED = 0x1,
	BOOT_ENDED = 0x2,
	FATAL_ERROR = 0x3,
	PASS_ENTER = 0x4,
	PASS_EXIT = 0x5,
	ENTER_FCN = 0x6,
	EXIT_FCN = 0x7,
	ENTER_SLEEP = 0x8,
	EXIT_SLEEP = 0x9,
	GENERIC_EVENT = 0xA,
	REG_DUMP = 0xB,
	EVENT_END = 0x7FFFFFFF,
} sysdbg_event_type;

typedef struct {
	sysdbg_event_type id;
	uint64_t arg1;
	uint64_t arg2;
	uint64_t arg3;
} sysdbg_log_entry_type;

typedef struct {
	volatile uint32_t index;
	sysdbg_log_entry_type buffer[SYSDBG_LOG_SIZE];
	void *register_config;
} SysdbgBufferType;

void SysdbgLogInit(void);
void sysdbg_log_event_id4(sysdbg_event_type event, uint64_t arg1, uint64_t arg2, uint64_t arg3);
void SysdbgLogDeInit(void);

#endif /* SYSDBG_LOG_H */
