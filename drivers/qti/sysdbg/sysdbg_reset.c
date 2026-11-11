/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <lib/utils_def.h>

#include <sysdbg_clock.h>
#include <sysdbg_main.h>
#include <sysdbg_mem_dump.h>
#include <sysdbg_reset.h>
#include <sysdbg_system.h>
#include <sysdbg_target.h>
#include <sysdbg_timer.h>

bool SysdbgMemoryReset(void)
{
	const mem_config_type *config_data = Sysdbg_MemConfig();
	uint32_t i, count;

	if (config_data == NULL) {
		return true;
	}

	count = ARRAY_SIZE(config_data->slices);

	for (i = 0; i < count; i++) {
		const mem_slice_type *slice = &config_data->slices[i];

		if (!slice->size) {
			continue;
		}

		if (slice->zinit) {
			memset((void *)(uintptr_t)(slice->base + slice->offset), 0, slice->size);
		}
	}

	return SysdbgDumpTableInit();
}

bool SysdbgColdBootInit(void)
{
	return SysdbgMemoryReset();
}

bool SysdbgRecoverRegisters(void)
{
	register_type *register_config = (register_type *)Sysdbg_RegConfig();
	static bool saved;
	uint32_t count = 0;

	if (register_config == NULL) {
		return true;
	}

	if (!saved) {
		while (register_config[count].register_addr) {
			register_config[count].register_val =
				*(uint32_t *)(uintptr_t)register_config[count].register_addr;
			count++;
		}
		saved = true;
	}

	return true;
}

bool SysdbgHWInit(void)
{
	bool success = true;

	success &= SysdbgTargetHWInit();
	success &= SysdbgTimerInit();
	success &= SysdbgClockInit();

	return success;
}

bool SysdbgResetCore(void)
{
	return SysdbgTargetResetCore();
}
