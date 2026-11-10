/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <lib/spinlock.h>

#include <sysdbg_crumbs.h>
#include <sysdbg_lock.h>
#include <sysdbg_log.h>
#include <sysdbg_target.h>

static SysdbgBufferType SysdbgLogBuffer = { 0 };
static spinlock_t syslog_lock;

void SysdbgLogInit(void)
{
	static bool init;
	uint64_t *sysdbg_log_ptr;

	if (!init) {
		SysdbgLogBuffer.index = 0x0;
		SysdbgLogBuffer.register_config = Sysdbg_RegConfig();

		sysdbg_log_ptr = (uint64_t *)(SysdbgGetSharedImemBase() + SYSDBG_LOGBUFPTR_OFFSET);
		*sysdbg_log_ptr = (uint64_t)(uintptr_t)&SysdbgLogBuffer;

		init = true;
	}
}

void sysdbg_log_event_id4(sysdbg_event_type id, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
	uint32_t entry;

	sysdbg_lock_obtain(&syslog_lock);

	entry = SysdbgLogBuffer.index;

	SysdbgLogBuffer.buffer[entry].id = id;
	SysdbgLogBuffer.buffer[entry].arg1 = arg1;
	SysdbgLogBuffer.buffer[entry].arg2 = arg2;
	SysdbgLogBuffer.buffer[entry].arg3 = arg3;

	SysdbgLogBuffer.index = (SysdbgLogBuffer.index + 1) % SYSDBG_LOG_SIZE;

	sysdbg_lock_release(&syslog_lock);
}

void SysdbgLogDeInit(void)
{
	volatile uint32_t *sysdbg_complete_cookie =
		(uint32_t *)(SysdbgGetSharedImemBase() + SYSDBG_PASS1COMPL_OFFSET);

	*sysdbg_complete_cookie = SYSDBG_PASS1_SUCCESS_COOKIE;
}
