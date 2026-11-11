/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <sysdbg.h>

unsigned int sysdbg_reset_check(void)
{
	return 0U;
}

void sysdbg_entry_handler(void)
{
}

dump_data_type *sysdbg_get_buffer(dump_type name, uint32_t block)
{
	(void)name;
	(void)block;

	return NULL;
}
