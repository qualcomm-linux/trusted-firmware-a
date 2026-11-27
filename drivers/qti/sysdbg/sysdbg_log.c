/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <common/debug.h>
#include <drivers/qti/sysdbg/sysdbg_target.h>
#include <drivers/qti/sysdbg/sysdbg_log.h>

void sysdbg_log_init(void)
{
	INFO("SDI: Logging initialized (using TF-A logging)\n");
}

/*
 * sysdbg_log_event_id4 - Log event with 4 parameters
 *
 * Ported from TZ: core/api/systemdebug/sysdbg_log.h
 *
 * Phase 1: Stub implementation
 * Phase 2: Implement actual logging to buffer
 */
void sysdbg_log_event_id4(sysdbg_event_type event, uint64_t arg1,
			  uint64_t arg2, uint64_t arg3)
{
	/* Phase 1 stub: no-op */
	(void)event;
	(void)arg1;
	(void)arg2;
	(void)arg3;
}
