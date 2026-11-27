/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <common/debug.h>
#include <drivers/qti/sysdbg/sysdbg_reset.h>
#include <drivers/qti/sysdbg/sysdbg_main.h>
#include <sysdbg_target_config.h>
#include <sysdbg_hwio.h>
#include <drivers/qti/sysdbg/sysdbg_crumbs.h>

/**
 * @file sysdbg_reset.c
 * @brief Shikra SDI cold boot/reset functions
 */

/*===========================================================================
 * Constants
 *===========================================================================
 */

/* Magic number to identify dump table */
#define SYSDBG_MAGIC_NUMBER        0x42445953U  /* "SYBD" in hex */

/*
 * Memory slices to zero-initialize during cold boot
 */
typedef struct {
	uintptr_t base;
	uint32_t offset;
	uint32_t size;
	bool zinit;
} mem_slice_t;

#define MEM_SLICE_COUNT 3

static const mem_slice_t mem_slices[MEM_SLICE_COUNT] = {
	/* SHARED_IMEM_REGION1: Log buffer pointer at 0x0C11EAF8 */
	{ .base = SYSTEM_SHAREDIMEM_BASE, .offset = 0xAF8, .size = 0x20, .zinit = true },
	/* SHARED_IMEM_REGION2: Reserved region at 0x0C11E764 */
	{ .base = SYSTEM_SHAREDIMEM_BASE, .offset = 0x764, .size = 0x20, .zinit = true },
	/* DUMP_TABLE_REGION: Dump table pointer at 0x0C11E010 */
	{ .base = SYSTEM_SHAREDIMEM_BASE, .offset = 0x10, .size = 0x8, .zinit = true }
};

/* Spinlock for dump table access */
static spinlock_t dump_table_lock = { .lock = 0 };

/*===========================================================================
 * Functions
 *===========================================================================
 */

static bool sysdbg_dump_table_init(void)
{
	uint64_t *dump_table_ptr;
	uint32_t *coldboot_status_ptr;

	INFO("SDI: Memory initialization (cold boot path)\n");

	/* Write magic number to dump table at SHARED_IMEM+0x10 (0x0C11E010) */
	dump_table_ptr = (uint64_t *)(SYSTEM_SHAREDIMEM_BASE + SYSDBG_DUMPTABLE_OFFSET);
	*dump_table_ptr = SYSDBG_MAGIC_NUMBER;
	INFO("SDI: Dump table initialized (magic=0x%llx at 0x%lx)\n",
	     (unsigned long long)SYSDBG_MAGIC_NUMBER,
	     (unsigned long)dump_table_ptr);

	/* Write cold boot success cookie at SHARED_IMEM+0xB14 (0x0C11EB14) */
	coldboot_status_ptr = (uint32_t *)(SYSTEM_SHAREDIMEM_BASE + SYSDBG_COLDBOOTST_OFFSET);
	*coldboot_status_ptr = SYSTEM_DEBUG_IMAGE_ID;
	INFO("SDI: Cold boot status written to 0x%lx (ID=0x%x)\n",
	     (unsigned long)coldboot_status_ptr, SYSTEM_DEBUG_IMAGE_ID);

	/* Initialize spinlock */
	dump_table_lock.lock = 0;

	INFO("SDI: Cold boot init complete\n");
	return true;
}

static bool sysdbg_memory_reset(void)
{
	uint32_t i;

	/* Zero-initialize all memory slices */
	for (i = 0; i < MEM_SLICE_COUNT; i++) {
		const mem_slice_t *slice = &mem_slices[i];

		if (slice->size && slice->zinit) {
			uintptr_t addr = slice->base + slice->offset;

			memset((void *)addr, 0, slice->size);
		}
	}

	return sysdbg_dump_table_init();
}

bool sysdbg_cold_boot_init(void)
{
	return sysdbg_memory_reset();
}

uintptr_t sysdbg_get_shared_imem_base(void)
{
	return SYSTEM_SHAREDIMEM_BASE;
}
