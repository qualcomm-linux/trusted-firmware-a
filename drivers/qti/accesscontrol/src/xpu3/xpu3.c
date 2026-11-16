/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <xpu3.h>
#include <xpu3_hal.h>
#include <xpu_target_info.h>

/*
 * XPU3 driver.
 *
 * Decides which instances and resource groups to program from the compile-time
 * tables under cfg/${CHIPSET}, and implements the revision-agnostic interface in
 * xpu_common.h. All register access is delegated to xpu3_hal.c, and violation
 * handling lives in xpu3_isr.c.
 */

/* Address stored in a resource group that is free for dynamic assignment. */
#define XPU_INVALID_ADDR 0xffffffffUL

static void xpu_lock_down_assets(void)
{
	struct xpu_instance *xpus = msm_xpu_cfg;

	for (size_t i = 0; i < msm_xpu_cfg_count; i++, xpus++) {
		if (!(xpus->flag & XPU_PROTECTION_STATIC))
			continue;

		xpu3_hal_enable_domain(xpus);

		xpu3_hal_program_mpu_partitions(xpus, XPU_RG_ALL);
		xpu3_hal_set_region_ownership(xpus, XPU_RG_ALL);
		xpu3_hal_set_mpu_permissions(xpus, XPU_RG_ALL);
	}
}

static int xpu_lock_down_assets_dynamic(struct xpu_instance *xpus,
					uint8_t xpu_count, uint32_t xpu_id,
					uint32_t rg_num, uint32_t perm_r,
					uint32_t perm_w)
{
	uint32_t i;

	for (i = 0; i < xpu_count; i++, xpus++) {
		if (xpus->xpu_id == xpu_id)
			break;
	}

	if (i >= xpu_count)
		return -1;

	xpu3_hal_enable_domain(xpus);

	xpu3_hal_program_mpu_partitions(xpus, rg_num);
	xpu3_hal_set_region_ownership(xpus, rg_num);
	xpu3_hal_set_mpu_dynamic_permissions(xpus, rg_num, perm_r, perm_w);

	return 0;
}

static void xpu_init_unused_regions(struct xpu_instance *mpu, uint32_t dev_cnt)
{
	uint32_t part_len = (uint32_t)mpu->part_range_arr_size;
	uint32_t owner_len = (uint32_t)mpu->owner_arr_size;
	uint32_t num_regions = xpu3_hal_get_idr0_nrg(mpu);

	if ((dev_cnt == 0U) || (dev_cnt > num_regions) ||
	    (dev_cnt > owner_len) || (dev_cnt > part_len)) {
		ERROR("XPU invalid board configuration\n");
		return;
	}

	uint32_t rg_start = num_regions - dev_cnt;

	struct rg_partition_range *part =
		mpu->partition_range + (part_len - dev_cnt);

	struct rg_domain_ownership *owner =
		mpu->rg_owner + (owner_len - dev_cnt);

	/* Prepare un-initialized regions for dynamic configuration */
	for (uint32_t k = 0U; k < dev_cnt; k++) {
		uint32_t rg = rg_start + k;

		owner->owner_domain = NO_DOMAIN;
		owner->rg_num = rg;

		part->start_addr = XPU_INVALID_ADDR;
		part->end_addr = XPU_INVALID_ADDR;
		part->rg_num = rg;

		owner++;
		part++;
	}
}

static void xpu_init_mpu_instances(struct mpu_ranges *range)
{
	for (uint32_t i = 0; i < range->mpus_count; i++) {
		struct xpu_instance *mpu = &range->mpus[i];

		if (xpu3_hal_get_xpu_type(mpu) != XPU_TYPE_MPU)
			continue;

		uint32_t dev_cnt = range->device_prtn_cnt;

		xpu_init_unused_regions(mpu, dev_cnt);
		xpu3_hal_enable_domain(mpu);
	}
}

static void xpu_master_mpu_init(void)
{
	struct mpu_ranges *range = msm_mpu_ranges;

	for (size_t i = 0; i < msm_mpu_ranges_count; i++, range++) {
		xpu_init_mpu_instances(range);
	}
}

int xpu_can_write(uintptr_t addr, bool *can_write)
{
	if (can_write == NULL) {
		return -1;
	}

	/*
	 * XPU3 targets do not place TZ-writable control registers in a page
	 * owned by another environment, so no query is needed. The downstream
	 * XPU3 driver had no equivalent check either.
	 */
	(void)addr;
	*can_write = true;

	return 0;
}

int acc_cfg_init(void)
{
	/*
	 * XPU3 configuration is compiled in under cfg/${CHIPSET}, so there is no
	 * configuration source to bring up.
	 */
	return 0;
}

void xpu_do_static_config(void)
{
	xpu_master_mpu_init();
	xpu_lock_down_assets();
	xpu_configure_tz();
	dsbsy();

	xpu3_enable_interrupts(xpu_non_sec_intr_en_reg, xpu_sec_intr_en_reg);
}

/*
 * Find a resource group in the given master-side MPU that already covers
 * [start_addr, end_addr], or else the first free one, and program it. Passing
 * APPS_NS_DOMAIN for a range that is already mapped releases it instead.
 */
static int update_master_side_mpu(struct xpu_instance *instance,
				  uint32_t dynamic_partition_count,
				  enum domain_type domain, uintptr_t start_addr,
				  uintptr_t end_addr, uint32_t perm_r,
				  uint32_t perm_w)
{
	uint64_t last_index = instance->part_range_arr_size;
	uint64_t first_index = last_index - dynamic_partition_count;
	struct rg_partition_range *range_base = instance->partition_range;
	struct rg_domain_ownership *owner_base = instance->rg_owner;
	struct rg_domain_ownership *found_owner = NULL;
	struct rg_partition_range *found_range = NULL;
	struct rg_domain_ownership *owner;
	struct rg_partition_range *range;
	uint64_t idx;

	range = range_base + first_index;
	owner = owner_base + first_index + 1; /* +1 for unmapped entry */

	for (idx = first_index; idx < last_index; idx++, range++, owner++) {
		/* Check if this RG already covers the requested range */
		if (range->start_addr == start_addr &&
		    range->end_addr == end_addr) {
			/* Free region */
			if (domain == APPS_NS_DOMAIN) {
				start_addr = XPU_INVALID_ADDR;
				end_addr = XPU_INVALID_ADDR;
				domain = NO_DOMAIN;

				INFO("freeing RG for xpu 0x%lx idx:%llu\n",
				     (unsigned long)instance->xpu_base_addr,
				     (unsigned long long)idx);
			}

			found_range = range;
			found_owner = owner;
			break;
		}

		/*
		 * Keep the first free RG; might be overridden if we later
		 * find an exact range match.
		 */
		if (owner->owner_domain == NO_DOMAIN &&
		    range->start_addr == XPU_INVALID_ADDR &&
		    range->end_addr == XPU_INVALID_ADDR) {
			if (!found_range) {
				found_range = range;
				found_owner = owner;
			}
		}
	}

	if (!found_range) {
		ERROR("No free RG xpu addr : 0x%lx",
		      (unsigned long)instance->xpu_base_addr);
		return 1;
	}

	found_owner->owner_domain = domain;
	found_range->start_addr = start_addr;
	found_range->end_addr = end_addr;

	return xpu_lock_down_assets_dynamic(instance, 1, instance->xpu_id,
					    found_range->rg_num, perm_r,
					    perm_w);
}

int xpu_mem_assign(enum device_type device, enum domain_type domain,
		   uintptr_t start, uintptr_t end,
		   uint32_t perm_r, uint32_t perm_w)
{
	struct mpu_ranges *range = msm_mpu_ranges;
	uint32_t i, j;
	int ret = 0;

	for (i = 0; i < msm_mpu_ranges_count; i++, range++) {
		if (range->device != device) {
			continue;
		}

		struct xpu_instance *mpu = range->mpus;

		for (j = 0; j < range->mpus_count; j++, mpu++) {
			ret = update_master_side_mpu(mpu,
						     range->device_prtn_cnt,
						     domain, start, end,
						     perm_r, perm_w);
			if (ret) {
				return ret;
			}
		}

		/* We found the device; no need to scan the rest. */
		break;
	}

	return 0;
}
