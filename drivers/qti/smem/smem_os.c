/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include "smem_internal.h"
#include "smem_os.h"
#include "smem_target.h"
#include "smem_type.h"

bool smem_os_verify_addr_mapped(uintptr_t phys_addr, uint32_t size)
{
	uintptr_t pa = phys_addr & (~SMEM_PAGE_ALIGN_MASK);
	size_t len = (((phys_addr + size + SMEM_PAGE_ALIGN_MASK) &
		       (~SMEM_PAGE_ALIGN_MASK)) - pa);

	/*
	 * Verify the region is covered by QTI_DEVICE static mapping.
	 * QTI_DEVICE_BASE to (QTI_DEVICE_BASE + QTI_DEVICE_SIZE) is
	 * statically mapped during platform initialization.
	 */
	if (pa < QTI_DEVICE_BASE || (pa + len) > (QTI_DEVICE_BASE + QTI_DEVICE_SIZE)) {
		ERROR("SMEM: region 0x%lx-0x%lx outside QTI_DEVICE range (0x%lx-0x%lx)\n",
		      pa, pa + len, (uintptr_t)QTI_DEVICE_BASE,
		      (uintptr_t)(QTI_DEVICE_BASE + QTI_DEVICE_SIZE));
		return false;
	}

	VERBOSE("SMEM: region 0x%lx-0x%lx within QTI_DEVICE static mapping\n",
		pa, pa + len);

	return true;
}

bool smem_early_boot_check(void)
{
	struct smem_static_allocs *static_allocs;
	uint32_t *version_array;
	uint32_t idx;

	static_allocs = (struct smem_static_allocs *)smem_info.smem_base_addr;
	version_array = static_allocs->version;

	if ((version_array[SMEM_VERSION_BOOT_OFFSET] == 0) ||
	    (version_array[SMEM_VERSION_TZ_HYP_OFFSET] == 0)) {
		return false;
	}

	for (idx = 0; idx < SMEM_VERSION_INFO_SIZE; idx++) {
		/* Ignore below subsystems */
		if ((idx == SMEM_VERSION_BOOT_OFFSET) ||
		    (idx == SMEM_VERSION_TZ_HYP_OFFSET) ||
		    (idx == SMEM_VERSION_RPM_OFFSET) ||
		    (idx == SMEM_VERSION_TME_OFFSET) ||
		    (idx == SMEM_VERSION_SOCCP_OFFSET) ||
		    (idx == SMEM_VERSION_DCP_OFFSET) ||
		    (idx == SMEM_VERSION_OOB_NS_OFFSET) ||
		    (idx == SMEM_VERSION_OOB_TEE_OFFSET) ||
		    (idx == SMEM_VERSION_QECP_OFFSET)) {
			continue;
		}

		if (version_array[idx] != 0) {
			return false;
		}
	}
	return true;
}
