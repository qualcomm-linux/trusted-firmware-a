/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <sysdbg.h>
#include <sysdbg_crumbs.h>
#include <sysdbg_lock.h>
#include <sysdbg_main.h>
#include <sysdbg_mem_dump.h>
#include <sysdbg_system.h>
#include <sysdbg_target.h>

static dump_data_type *SysdbgLPASSETBDump;
static dump_data_type *SysdbgPMICDump;
static dump_data_type *SysdbgDCPDump;

bool SysdbgDumpTableInit(void)
{
	uint64_t *dump_table_ptr =
		(uint64_t *)(SysdbgGetSharedImemBase() + SYSDBG_DUMPTABLE_OFFSET);

	*dump_table_ptr = SYSDBG_MAGIC_NUMBER;

	*(uint32_t *)((uint8_t *)SysdbgGetSharedImemBase() + SYSDBG_COLDBOOTST_OFFSET) =
		SYSTEM_DEBUG_IMAGE_ID;

	return true;
}

static void sysdbg_save_apps_dump_entry(const dump_entry_type *entry)
{
	dump_data_type *buffer = (dump_data_type *)(uintptr_t)entry->start_addr;
	uint32_t count = entry->id & 0xFFFFFFF0U;
	uint32_t block = entry->id & 0xFU;

	if ((entry->start_addr == 0U) || (entry->type != MSM_DUMP_TYPE_DATA)) {
		return;
	}

	switch (count) {
	case MSM_LPASSETB_DUMP:
		if ((block == 4U) && (SysdbgLPASSETBDump == NULL)) {
			SysdbgLPASSETBDump = buffer;
		}
		break;
	case MSM_DCP_IMG_BACKUP:
		if (SysdbgDCPDump == NULL) {
			SysdbgDCPDump = buffer;
		}
		break;
	default:
		if (((entry->id & 0x1FFU) == MSM_PMIC_DUMP) && (SysdbgPMICDump == NULL)) {
			SysdbgPMICDump = buffer;
		}
		break;
	}
}

bool SysdbgMemoryDumpInit(void)
{
	static bool initialized;
	dump_table_type **dump_table;
	uint32_t i, j;

	if (initialized) {
		return initialized;
	}

	sysdbg_lock_obtain(get_mem_lock());

	dump_table = (dump_table_type **)((uint8_t *)SysdbgGetSharedImemBase() +
					  SYSDBG_DUMPTABLE_OFFSET);

	if ((*dump_table != NULL) &&
	    (*dump_table != (dump_table_type *)(uintptr_t)SYSDBG_MAGIC_NUMBER)) {
		for (i = 0; i < (*dump_table)->num_entries; i++) {
			dump_entry_type top_entry = (**dump_table).entries[i];
			dump_table_type *apps_dump_table;

			if ((top_entry.type != MSM_DUMP_TYPE_TABLE) ||
			    ((uint32_t)top_entry.id != (uint32_t)MSM_DUMP_CLIENT_APPS)) {
				continue;
			}

			apps_dump_table = (dump_table_type *)(uintptr_t)top_entry.start_addr;
			for (j = 0; j < apps_dump_table->num_entries; j++) {
				sysdbg_save_apps_dump_entry(&apps_dump_table->entries[j]);
			}
		}
		initialized = true;
	}

	sysdbg_lock_release(get_mem_lock());

	return initialized;
}

static bool sysdbg_is_validbuffer(dump_data_type *buffer, dump_type name)
{
	if (buffer == NULL) {
		return false;
	}

	return sysdbg_buffer_len_is_valid(name, buffer->len);
}

dump_data_type *sysdbg_get_buffer(dump_type name, uint32_t block)
{
	dump_data_type *buffer = NULL;

	if (!SysdbgMemoryDumpInit()) {
		return NULL;
	}

	switch (name) {
	case MSM_DCP_IMG_BACKUP:
		buffer = SysdbgDCPDump;
		break;
	case MSM_LPASSETB_DUMP:
		buffer = SysdbgLPASSETBDump;
		break;
	case MSM_PMIC_DUMP:
		buffer = SysdbgPMICDump;
		break;
	default:
		break;
	}

	if (!sysdbg_is_validbuffer(buffer, name)) {
		buffer = NULL;
	}

	return buffer;
}
