/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "smem.h"
#include "smem_internal.h"
#include "smem_os.h"
#include "smem_partition.h"
#include "smem_target.h"
#include "smem_v.h"
#include "smem_version.h"

struct smem_info smem_info;

static int32_t smem_init_check(struct smem_info *sinfo, enum smem_mem_type smem_type)
{
	if (sinfo->state != SMEM_STATE_INITIALIZED) {
		smem_init();
	}

	if ((sinfo->state != SMEM_STATE_INITIALIZED) ||
	    (sinfo->funcs == NULL)) {
		ERROR("SMEM: not initialized state=%d! smem_type=%d\n",
		      sinfo->state, smem_type);
		return SMEM_STATUS_FAILURE;
	}

	if (smem_type >= sinfo->max_items) {
		ERROR("SMEM: trying to allocate/get smem item=%d which is >= max item=%d\n",
		      smem_type, sinfo->max_items);
		return SMEM_STATUS_INVALID_PARAM;
	}

	return SMEM_STATUS_SUCCESS;
}

void *smem_alloc(enum smem_mem_type smem_type, uint32_t buf_size)
{
	void *smem_buf;
	uint32_t ret_size;

	smem_buf = smem_get_addr(smem_type, &ret_size);
	if (smem_buf == NULL) {
		ERROR("SMEM: %s is not supported only get address is supported. smem_type=%d was not allocated already\n",
			__func__, smem_type);
	} else if (ret_size != buf_size) {
		VERBOSE("SMEM: %s smem_type=%d was allocated with different size=%d already, expected=%d\n",
			__func__, smem_type, ret_size, buf_size);

		smem_buf = NULL;
	}

	return smem_buf;
}

void *smem_get_addr(enum smem_mem_type smem_type, uint32_t *buf_size)
{
	struct smem_info *sinfo = &smem_info;
	void *smem_buf;

	if (smem_init_check(sinfo, smem_type) != SMEM_STATUS_SUCCESS) {
		return NULL;
	}
	if (!sinfo->funcs->get_addr) {
		return NULL;
	}

	smem_buf = sinfo->funcs->get_addr(smem_type, buf_size);

	if (smem_buf == NULL) {
		ERROR("SMEM: get addr failed! smem_type=%d\n",
			smem_type);
	} else {
		VERBOSE("SMEM: get addr success. smem_type=%d, buf_size=%d and offset=0x%08lX\n",
			smem_type, buf_size ? *buf_size : 0xFFFFFFFF,
			(unsigned long)((uint8_t *)smem_buf - sinfo->smem_base_addr));
	}
	return smem_buf;
}

bool smem_version_set(enum smem_mem_type type, uint32_t version, uint32_t mask)
{
	uint32_t idx;
	uint32_t my_version;
	uint32_t other_version;
	bool match = true;
	uint32_t *version_array;

	if ((type != SMEM_VERSION_INFO) &&
	    ((type < SMEM_VERSION_FIRST) || (type > SMEM_VERSION_LAST))) {
		return false;
	}

	if (type == SMEM_VERSION_INFO) {
		struct smem_static_allocs *static_allocs =
			(struct smem_static_allocs *)smem_info.smem_base_addr;

		version_array = static_allocs->version;
	} else {
		version_array = (uint32_t *)smem_alloc(
			type, SMEM_VERSION_INFO_SIZE * sizeof(uint32_t));
		if (version_array == NULL) {
			ERROR("SMEM: %s: unable to allocate version array: type %d\n",
			      __func__, type);
			plat_error_handler(EFAULT);
			return false;
		}
	}

	my_version = (version & mask);
	version_array[smem_info.version_offset] |= my_version;

	smem_os_mem_barrier();

	for (idx = 0; idx < SMEM_VERSION_INFO_SIZE; idx++) {
		other_version = version_array[idx] & mask;

		if ((other_version != 0) && (other_version != my_version)) {
			match = false;
			idx = SMEM_VERSION_INFO_SIZE;
		}
	}

	return match;
}

static uint32_t smem_boot_version_get(struct smem_info *sinfo)
{
	volatile struct smem_static_allocs *static_allocs =
		(volatile struct smem_static_allocs *)sinfo->smem_base_addr;
	uint32_t version;

	version = static_allocs->version[SMEM_VERSION_BOOT_OFFSET];

	return version;
}

static bool smem_boot_init_check(struct smem_info *sinfo)
{
	volatile struct smem_static_allocs *static_allocs =
		(volatile struct smem_static_allocs *)sinfo->smem_base_addr;

	bool init_status;

	init_status =
		(static_allocs->heap_info.initialized == SMEM_HEAP_INFO_INIT);

	return init_status;
}

void smem_init(void)
{
	struct smem_info *sinfo = &smem_info;
	struct smem_targ_host_info *targ_host_info;

	uint32_t version;

	if (sinfo->state == SMEM_STATE_INITIALIZED) {
		return;
	}

	targ_host_info = smem_targ_init();
	if (!targ_host_info) {
		ERROR("SMEM: target init failed\n");
		plat_error_handler(EFAULT);
		return;
	}

	if (targ_host_info->version_offset >= SMEM_VERSION_INFO_SIZE) {
		ERROR("SMEM: version info offset (%d) is invalid\n",
		      sinfo->version_offset);
		plat_error_handler(EFAULT);
		return;
	}

	sinfo->this_host = targ_host_info->this_host;
	sinfo->smem_phys_base_addr = targ_host_info->phys_addr;
	sinfo->smem_size = targ_host_info->size;
	sinfo->max_items = targ_host_info->max_items;
	sinfo->version_offset = targ_host_info->version_offset;

	/*
	 * Verify SMEM base address is within statically mapped region.
	 */
	if (!smem_os_verify_addr_mapped((uintptr_t)sinfo->smem_phys_base_addr,
					sinfo->smem_size)) {
		ERROR("SMEM: base addr=0x%016lX, size=%d is not mapped\n",
		      sinfo->smem_phys_base_addr, sinfo->smem_size);
		plat_error_handler(EFAULT);
		return;
	}

	/*
	 * SMEM is statically mapped as part of QTI_DEVICE region.
	 * Since it's 1:1 mapping, use physical address directly as virtual address.
	 */
	sinfo->smem_base_addr = (uint8_t *)(uintptr_t)sinfo->smem_phys_base_addr;

	if (!smem_boot_init_check(sinfo)) {
		ERROR("SMEM: not initialized by boot\n");
		plat_error_handler(EFAULT);
		return;
	}

	version = smem_boot_version_get(sinfo);

	if ((version & SMEM_MAJOR_VERSION_MASK) == SMEM_LEGACY_VERSION_ID) {
		ERROR("SMEM: version 0x%X is not supported\n", version);
		plat_error_handler(EFAULT);
	} else {
		sinfo->version = SMEM_VERSION_ID;

		sinfo->funcs = &smem_part_funcs;
	}

	smem_part_init();

	/*
	 * Set SMEM version info and verify that everyone registered so
	 * far is compatible.
	 * Each call to smem_init verifies that all previous calls are
	 * compatible. So, the last call to smem_init will be certain to
	 * detect if anyone is incompatible.
	 */
	smem_os_mem_barrier();

	/*
	 * Moved before the version check/set in order to fulfill that function's
	 * dependency that smem has been initialized (and indicate so).
	 */
	sinfo->state = SMEM_STATE_INITIALIZED;

	if (smem_version_set(SMEM_VERSION_INFO, sinfo->version,
			     SMEM_MAJOR_VERSION_MASK) == false) {
		ERROR("SMEM: %s: major version (%d) does not match all procs\n",
		      __func__, sinfo->version);
		plat_error_handler(EFAULT);
		return;
	}
}
