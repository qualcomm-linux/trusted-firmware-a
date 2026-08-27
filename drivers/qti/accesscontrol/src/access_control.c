/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/qti/accesscontrol/accesscontrol.h>
#include <lib/spinlock.h>
#include <vmidmt.h>
#include <xpu_common.h>

#define AC_PERM_X 0x1
#define AC_PERM_W 0x2
#define AC_PERM_R 0x4

static spinlock_t mem_assign_lock;

enum virtual_machine_id {
	AC_VM_NONE = 0,
	AC_VM_HLOS = 3,
	AC_VM_MSS_MSA = 15,
	AC_VM_MSS_NAV = 43,
	AC_VM_LAST = 44,
	AC_VM_MAX = 0x7FFFFFFF,
};

enum ac_error {
	AC_SUCCESS = 0,
	AC_FAILURE = 1,
	AC_ERR_VM_CREATE_FAIL = 2,
	AC_ERR_VM_MAP_FAIL1 = 3,
	AC_ERR_RAM_PARTITION_TABLE = 4,
	AC_ERR_VM_MAP_FAIL2 = 5,
	AC_ERR_VM_UNMAP_FAIL1 = 6,
	AC_ERR_VM_MAP_FAIL3 = 7,
	AC_ERR_VM_UNMAP_FAIL2 = 8,
	AC_ERR_TRANSLATION_SET1 = 9,
	AC_ERR_TRANSLATION_SET2 = 10,
	AC_ERR_TRANSLATION_SET3 = 11,
	AC_ERR_VALIDATION_FAIL1 = 12,
	AC_ERR_INCORRECT_VM = 13,
	AC_ERR_IO_ADDRESS_MISMATCH = 14,
	AC_ERR_SHARED_MEMORY_SINGLE_SOURCE = 15,
	AC_ERR_SHARED_MEMORY_SOURCE_MISMATCH = 16,
	AC_ERR_NOT_SHARED_MEMORY_MULTIPLE_SOURCE_GIVEN = 17,
	AC_ERR_MAPPING_TYPE_NOT_SUPPORTED = 18,
	AC_ERR_MAPPING_NOT_FOUND = 19,
	AC_ERR_REMOVE_MEMORY_FROM_LIST_FAIL = 20,
	AC_ERR_CLEAR_MEMORY_FAIL = 21,
	AC_ERR_IS_DEVICE_MEMORY = 22,
	AC_ERR_VMISMAPPED_FAILED = 23,
	AC_ERR_UNCACHED_ALLOC_FAILED = 24,
	AC_ERR_MEMORY_NOT_OWNED_BY_SOURCE_VM = 25,
	AC_ERR_TZ_ASSIGN_SMC_FAILED = 26,
	AC_ERR_VM_UNMAP_FAIL3 = 27,
	AC_ERR_VM_MAP_FAIL4 = 28,
	AC_ERR_MEMORY_FULL = 29,
	AC_ERR_MEMORY_IN_OWNED_BY_TZ = 30,
	AC_ERR_MEMORY_IN_USE_BY_TZ = 31,
	AC_ERR_XPU_TYPE_NOT_SUPPORTED = 32,
	AC_ERR_XPU_REMOVE_MAPPING_FAILED = 33,
	AC_ERR_XPU_ADD_MAPPING_FAILED = 34,
	AC_ERR_MEMORY_NOT_FOUND_IN_LIST = 35,
	AC_ERR_MEMORY_ALREADY_IN_LIST = 36,
	AC_ERR_ADD_MEMORY_FROM_LIST_FAIL = 37,
	AC_ERR_SIZE_GREATER_THAN_32BITS = 38,
	AC_ERR_INVALID_INDEX = 39,
	AC_ERR_UPDATING_RAM_PARTITION_TABLE = 40,
	AC_ERR_NOT_4K_ALIGNED = 41,
	AC_ERR_MEMORY_NOT_IN_LIST = 42,
	AC_ERR_NOT_DDR_MEMORY = 43,
	AC_ERR_IPA_OVERFLOW = 44,
	AC_ERR_SRC_SIZE_ZERO = 45,
	AC_ERR_SRC_LIST_NULL = 46,
	AC_ERR_DST_SIZE_ZERO = 47,
	AC_ERR_DST_LIST_NULL = 48,
	AC_ERR_SID2VM_SMMU_API_FAILED = 49,
	AC_ERR_SID_VALIDATION_FAIL = 50,
	AC_ERR_UNABLE_TO_XPU_LOCK = 51,
	AC_ERR_STRUCT_SIZE_LESS_THAN_EXPECTED = 52,
	AC_ERR_INVALID_POINTER = 53,
	AC_ERR_IPA_LIST_NULL = 54,
	AC_ERR_IPA_LIST_SIZE_ZER0 = 55,
	AC_ERR_MMU_ADD_MAPPING_FAILED = 56,
	AC_ERR_MMU_REMOVE_MAPPING_FAILED = 57,
	AC_ERR_INVALID_PERM_TYPE = 58,
	AC_ERR_MPU_LOCK_MEMORY_FAILED = 59,
	AC_ERR_MPU_UNLOCK_MEMORY_FAILED = 60,
	AC_ERR_USECASE_NOT_SUPPORTED = 61,
	AC_ERR_SRC_VM_TZ_INVALID = 62,
	AC_ERR_NULL_POINTER = 63,
	AC_ERR_TZ_IO_ASSIGN_SMC_FAILED = 64,
	AC_ERR_NOT_DEVICE_MEMORY = 65,
	AC_ERR_SMMU_CFG_TYPE_INVALID = 66,
	AC_ERR_INCORRECT_PERM = 67,
	AC_ERR_MEMORY_IS_SHARED = 68,
	AC_ERR_CANNOT_CHANGE_HLOS_RO_MEMORY = 69,
	AC_ERR_UNABLE_TO_XPU_UNLOCK = 70,
	AC_ERR_ADD_CLEAR_REGION_FAILED = 71,
	AC_ERR_REMOVE_CLEAR_REGION_FAILED = 72,
	AC_ERR_OVERLAPPING_MEMORY = 73,
	AC_ERR_DEVICE_RANGE_CHECK_OVERFLOW = 74,
	AC_ERR_ITS_A_SECURE_DEVICE = 75,
	AC_ERR_NOT_IN_WHITELIST = 76,
	AC_ERR_API_FAILED = 77,
	AC_ERR_SRC_NUM_INVALID = 78,
	AC_ERR_NUM_MAPPING_OVERFLOW = 79,
	AC_ERR_DST_NUM_INVALID = 80,
	AC_ERR_INCORRECT_DEVICE = 81,
	AC_ERR_DEVICE_NOT_FOUND = 82,
	AC_ERR_XPU_PARTIAL_MAPPING_NOT_ALLOWED = 83,
	AC_ERR_HASH_OVERFLOW = 84,
	AC_ERR_RULE_NOT_FOUND = 85,
	AC_ERR_TZ_SHM_CREATE_SMC_FAILED = 86,
	AC_ERR_VM_SIZE_OVERFLOW = 87,
	AC_ERR_SHM_BRIDGE_NOT_FOUND = 88,
	AC_ERR_GETTING_RANDOM_NUMBER = 89,
	AC_ERR_CALLER_VM_ID_INCORRECT = 90,
	AC_ERR_SHM_RULE_NOT_FOUND = 91,
	AC_ERR_MUTEX_ACQUIRE_FAIL = 92,
	AC_ERR_MUTEX_RELEASE_FAIL = 93,
	AC_ERR_OUTPUT_ADDRESS_MISMATCH = 94,
	AC_ERR_DST_VM_TZ_INVALID = 95,
	AC_ERR_MPU_UPDATE_LOCK_MEMORY_FAILED = 96,
	AC_ERR_DDR_MPU_STATIC_CFG_BLACKLIST_UPDATE = 97,
	AC_ERR_FATAL_ACCESS_CONTROL = 98,
	AC_ERR_LAST,
	AC_ERR_MAX = 0x7FFFFFFF,
};

static enum ac_error
process_sources(const uint32_t *src_vm_list, uint32_t src_vm_count,
		const qti_accesscontrol_perm_t *dst_vm_list,
		uint32_t dst_vm_count, enum device_type *device,
		enum domain_type *domain)
{
	const uint32_t *src_vm;
	const qti_accesscontrol_perm_t *dst_vm;
	const qti_accesscontrol_perm_t *dst_vm_limit;
	uint32_t src_index;
	uint32_t dst_index;
	uint32_t vm_also_in_other_list = 0U;

	src_vm = src_vm_list;

	for (src_index = 0U; src_index < src_vm_count; src_index++, src_vm++) {
		uint32_t src_vm_id = *src_vm;

		if (src_vm_id != AC_VM_MSS_MSA && src_vm_id != AC_VM_MSS_NAV)
			continue;

		/* Check if same VM appears as destination */
		dst_vm = dst_vm_list;
		dst_vm_limit =
			dst_vm_list + ((dst_vm_count < (uint32_t)AC_VM_LAST) ?
					       dst_vm_count :
					       (uint32_t)AC_VM_LAST);

		for (dst_index = 0U; dst_vm < dst_vm_limit;
		     dst_index++, dst_vm++) {
			if (dst_vm->dst_vm != src_vm_id)
				continue;

			vm_also_in_other_list = 1U;
			break;
		}

		if (vm_also_in_other_list != 0U) {
			vm_also_in_other_list = 0U;
			continue;
		}

		if (src_vm_id == AC_VM_MSS_MSA)
			*device = DEVICE_MODEM;
		else if (src_vm_id == AC_VM_MSS_NAV)
			*device = DEVICE_MSS_NAV;
		else
			return AC_ERR_XPU_TYPE_NOT_SUPPORTED;

		*domain = APPS_NS_DOMAIN;
	}

	return AC_SUCCESS;
}

static enum ac_error
process_destinations(const uint32_t *src_vm_list, uint32_t src_vm_count,
		     const qti_accesscontrol_perm_t *dst_vm_list,
		     uint32_t dst_vm_count, enum device_type *device,
		     enum domain_type *domain, uint32_t *read_perm_domain,
		     uint32_t *write_perm_domain)
{
	const qti_accesscontrol_perm_t *dst_vm;
	const uint32_t *src_check;
	uint32_t vm_also_in_other_list = 0U;
	uint32_t dst_index;
	uint32_t src_index;

	dst_vm = dst_vm_list;

	for (dst_index = 0U; dst_index < dst_vm_count; dst_index++, dst_vm++) {
		uint32_t dst_vm_id = dst_vm->dst_vm;
		uint32_t dst_vm_perm = dst_vm->dst_vm_perm;

		if (dst_vm_id != AC_VM_MSS_MSA && dst_vm_id != AC_VM_MSS_NAV)
			continue;

		/* Check if same VM exists in source list */
		src_check = src_vm_list;
		for (src_index = 0U; src_index < src_vm_count;
		     src_index++, src_check++) {
			if (dst_vm_id != *src_check)
				continue;

			vm_also_in_other_list = 1U;
			break;
		}

		if (vm_also_in_other_list != 0U) {
			vm_also_in_other_list = 0U;
			continue;
		}

		if (dst_vm_id == AC_VM_MSS_MSA)
			*device = DEVICE_MODEM;
		else if (dst_vm_id == AC_VM_MSS_NAV)
			*device = DEVICE_MSS_NAV;
		else
			return AC_ERR_XPU_TYPE_NOT_SUPPORTED;

		/* Permissions */
		if ((dst_vm_perm & AC_PERM_W) != 0U)
			*write_perm_domain = MSA_DOMAIN;

		if ((dst_vm_perm & AC_PERM_R) != 0U)
			*read_perm_domain = MSA_DOMAIN;

		/* Modem keeps secure RG ownership permanently */
		*domain = APPS_S_DOMAIN;
	}

	return AC_SUCCESS;
}

static enum ac_error assign_regions(const qti_accesscontrol_mem_t *mem_regions,
				    uint32_t mem_region_count,
				    enum device_type device,
				    enum domain_type domain,
				    uint32_t read_perm_domain,
				    uint32_t write_perm_domain)
{
	const qti_accesscontrol_mem_t *mem_region = mem_regions;
	uint32_t mem_index;

	for (mem_index = 0U; mem_index < mem_region_count;
	     mem_index++, mem_region++) {
		uintptr_t region_base = mem_region->mem_addr;
		uintptr_t region_size = mem_region->mem_size;
		uintptr_t region_end = region_base + region_size;
		int rc;

		rc = xpu_mem_assign(device, domain, region_base, region_end,
				    read_perm_domain, write_perm_domain);
		if (rc != 0) {
			/*
			 * A failed assignment leaves the XPU policy in an
			 * indeterminate state, so this is not recoverable.
			 */
			ERROR("Access control fatal (%x)\n",
			      AC_ERR_MPU_UPDATE_LOCK_MEMORY_FAILED);

			for (;;) {
				wfi();
			}
		}
	}

	return AC_SUCCESS;
}

static uint64_t mem_assign(const qti_accesscontrol_mem_t *mem_regions,
			   uint32_t mem_region_count,
			   const uint32_t *src_vm_list, uint32_t src_vm_count,
			   const qti_accesscontrol_perm_t *dst_vm_list,
			   uint32_t dst_vm_count)
{
	enum ac_error result = AC_SUCCESS;
	uint32_t write_perm_domain = NO_DOMAIN;
	uint32_t read_perm_domain = NO_DOMAIN;
	enum domain_type domain = NO_DOMAIN;
	enum device_type device = 0;

	spin_lock(&mem_assign_lock);

	result = process_sources(src_vm_list, src_vm_count, dst_vm_list,
				 dst_vm_count, &device, &domain);
	if (result != AC_SUCCESS)
		goto out;

	result = process_destinations(src_vm_list, src_vm_count, dst_vm_list,
				      dst_vm_count, &device, &domain,
				      &read_perm_domain, &write_perm_domain);
	if (result != AC_SUCCESS)
		goto out;

	result = assign_regions(mem_regions, mem_region_count, device, domain,
				read_perm_domain, write_perm_domain);
	if (result != AC_SUCCESS)
		goto out;

out:
	spin_unlock(&mem_assign_lock);

	if (result != AC_SUCCESS) {
		ERROR("Access Control: memory assignment failed %x\n", result);
		return (uint64_t)-1;
	}

	return 0;
}

uint64_t qti_accesscontrol_mem_assign(const qti_accesscontrol_mem_t *mem,
				      uint32_t mem_len, const uint32_t *src,
				      uint32_t src_len,
				      const qti_accesscontrol_perm_t *perm,
				      uint32_t perm_len)
{
	return mem_assign(mem, mem_len, src, src_len, perm, perm_len);
}

void qti_accesscontrol_init(void)
{
	int rc;

	/*
	 * Bring up the configuration source first: on XPU4 both the VMIDMT and
	 * the XPU configuration are read out of the access-control config image,
	 * so nothing below can run until it has been parsed and validated.
	 */
	rc = acc_cfg_init();
	if (rc) {
		ERROR("Error reading access control config, fatal (%d)\n", rc);
		goto error;
	}

	rc = vmidmt_configure();
	if (rc) {
		ERROR("Error configuring the VMIDMT, fatal (%d)\n", rc);
		goto error;
	}

	xpu_do_static_config();

	rc = xpu_register_interrupts();
	if (rc) {
		ERROR("Error registering the XPU interrupts, fatal\n");
		goto error;
	}

	return;
error:
	for (;;)
		wfi();
}
