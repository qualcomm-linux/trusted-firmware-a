/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMEM_OS_H
#define SMEM_OS_H

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <common/debug.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include "smem_target.h"
#include "smem_toc.h"

/* Mask for smallest possible page mapping size */
#define SMEM_PAGE_ALIGN_MASK	0x00000FFF

/*
 * Copy bytes from the source buffer to the destination buffer.
 *
 * This function ensures that there will not be a copy beyond the size of
 * the destination buffer. The result of calling this on overlapping source
 * and destination buffers is undefined.
 */
static inline uint32_t smem_os_mem_copy(void *dst, uint32_t dst_size,
				      const void *src, uint32_t copy_size)
{
	uint32_t copy_len = (copy_size < dst_size) ? copy_size : dst_size;

	if (copy_len > 0) {
		memcpy(dst, src, copy_len);
	}

	return copy_len;
}

/*
 * Verify that the physical address is within the statically mapped region.
 * Returns true if the address is within the mapped range, false otherwise.
 */
bool smem_os_verify_addr_mapped(uintptr_t phys_addr, uint32_t size);

/*
 * Memory barrier to ensure memory operations are completed.
 */
static inline void smem_os_mem_barrier(void)
{
	__asm__ volatile("dsb sy" : : : "memory");
}

/*
 * Check if any other SMEM masters are up.
 */
bool smem_early_boot_check(void);

#endif /* SMEM_OS_H */
