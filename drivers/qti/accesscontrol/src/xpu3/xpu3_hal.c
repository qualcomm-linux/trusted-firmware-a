/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <xpu3.h>
#include <xpu3_hal.h>
#include <xpu3_hwio.h>

uint8_t xpu3_hal_get_xpu_type(struct xpu_instance *xpu)
{
	uintptr_t base = xpu->xpu_base_addr;
	uint32_t idr0;

	idr0 = mmio_read_32(base + XPU3_IDR0_OFFSET);

	return (uint8_t)(idr0 & XPU_TYPE_BITMASK);
}

uint32_t xpu3_hal_get_idr0_nrg(struct xpu_instance *xpu)
{
	uint32_t id_r0 = mmio_read_32(xpu->xpu_base_addr + XPU3_IDR0_OFFSET);

	return FIELD_GET(XPU3_IDR0_NRG_BMSK, id_r0) + 1;
}

void xpu3_hal_enable_domain(struct xpu_instance *xpu)
{
	uint32_t gcr0_cfg = XPU3_GCR0_DOMAIN_ENABLE;
	uint32_t gcr0_val;
	uint32_t rev;

	/* Already initialized from this context */
	if ((xpu->flag & XPU_INITIALIZED) != 0U)
		return;

	xpu->flag |= XPU_INITIALIZED;

	/* If GCR0 is already programmed */
	gcr0_val = mmio_read_32(xpu->xpu_base_addr + XPU3_GCR0_OFFSET);
	if (gcr0_val != 0U)
		return;

	/* HW revision check: enable log-mode disable on newer revisions */
	rev = mmio_read_32(xpu->xpu_base_addr + XPU3_REV_OFFSET);
	rev >>= XPU3_REV_SHFT;

	if (rev > 0x3002U) {
		mmio_write_32(xpu->xpu_base_addr + XPU3_LOG_MODE_DIS_OFFSET, 1);
		gcr0_cfg |= XPU3_GCR0_LOG_MODE_DISABLE;
	}

	/* Main XPU instance */
	mmio_write_32(xpu->xpu_base_addr + XPU3_GCR0_OFFSET, gcr0_cfg);
	mmio_write_32(xpu->xpu_base_addr + XPU3_CR0_OFFSET,
		      XPU3_CR0_INTR_ENABLE);

	/* QAD0 */
	mmio_write_32(xpu->xpu_base_addr + XPU3_QAD0_CR0_OFFSET,
		      XPU3_CR0_INTR_ENABLE);
	mmio_write_32(xpu->xpu_base_addr + XPU3_QAD0_GCR0_OFFSET, gcr0_cfg);

	/* QAD1 */
	mmio_write_32(xpu->xpu_base_addr + XPU3_QAD1_CR0_OFFSET,
		      XPU3_CR0_INTR_ENABLE);
	mmio_write_32(xpu->xpu_base_addr + XPU3_QAD1_GCR0_OFFSET, gcr0_cfg);

	dmbsy();
	isb();
}

static inline bool vmid_supported(const struct xpu_instance *xpu)
{
	uint32_t idr2;

	idr2 = mmio_read_32(xpu->xpu_base_addr + XPU3_IDR2_OFFSET);

	return ((idr2 & XPU_IDR2_VMID_SUPPORT_BITMASK) >>
		XPU_IDR2_VMID_SUPPORT_BITSHIFT) != 0U;
}

void xpu3_hal_set_mpu_permissions(struct xpu_instance *xpu, uint32_t rg_num)
{
	struct rg_partition_range *range = xpu->partition_range;
	struct rg_domain_ownership *owner = xpu->rg_owner;
	uintptr_t base = xpu->xpu_base_addr;
	uintptr_t cr0, cr1, cr2, cr3;
	uintptr_t racr, racw;
	bool no_perms;
	uint32_t tmp;

	if (xpu3_hal_get_xpu_type(xpu) != XPU_TYPE_MPU)
		return;

	for (int i = 0; i < xpu->part_range_arr_size; i++, range++, owner++) {
		if (rg_num != XPU_RG_ALL && range->rg_num != rg_num)
			continue;

		if (i >= xpu->owner_arr_size)
			goto out;

		no_perms = (owner->perm_r == 0 && owner->perm_w == 0);

		if (owner->rg_num == XPU_UMR_RG || no_perms)
			continue;

		cr1 = base + XPU3_RGN_CR1_OFFSET(range->rg_num);
		cr3 = base + XPU3_RGN_CR3_OFFSET(range->rg_num);
		cr0 = base + XPU3_RGN_CR0_OFFSET(range->rg_num);
		cr2 = base + XPU3_RGN_CR2_OFFSET(range->rg_num);

		/* Set permissions */
		mmio_write_32(cr1, owner->perm_r);
		mmio_write_32(cr3, owner->perm_w);

		if (owner->perm_r & APPS_S_DOMAIN)
			mmio_write_32(cr0, 1);

		if (owner->perm_w & APPS_S_DOMAIN)
			mmio_write_32(cr2, 1);

		if (vmid_supported(xpu)) {
			/* RACR */
			racr = base + XPU3_RGN_RACR_OFFSET(range->rg_num);
			tmp = owner->perm_r & APPS_NS_DOMAIN ?
				DEFAULT_VMID_0 : 0;
			mmio_write_32(racr, tmp);

			/* RACW */
			racw = base + XPU3_RGN_WACR_OFFSET(range->rg_num);
			tmp = owner->perm_w & APPS_NS_DOMAIN ?
				DEFAULT_VMID_0 : 0;
			mmio_write_32(racw, tmp);
		}
	}

	/* Handle UMR region */
	no_perms = (owner->perm_r == 0 && owner->perm_w == 0);

	if ((owner->rg_num == XPU_UMR_RG) && !no_perms) {
		cr1 = base + XPU3_UMR_CR1_OFFSET;
		cr3 = base + XPU3_UMR_CR3_OFFSET;
		cr0 = base + XPU3_UMR_CR0_OFFSET;
		cr2 = base + XPU3_UMR_CR2_OFFSET;

		mmio_write_32(cr1, owner->perm_r);
		mmio_write_32(cr3, owner->perm_w);

		if (owner->perm_r & APPS_S_DOMAIN)
			mmio_write_32(cr0, 1);

		if (owner->perm_w & APPS_S_DOMAIN)
			mmio_write_32(cr2, 1);

		if (vmid_supported(xpu)) {
			/* RACR */
			racr = base + XPU3_UMR_RACR_OFFSET;
			tmp = owner->perm_r & APPS_NS_DOMAIN ?
				DEFAULT_VMID_0 : 0;
			mmio_write_32(racr, tmp);

			/* RACW */
			racw = base + XPU3_UMR_WACR_OFFSET;
			tmp = owner->perm_w & APPS_NS_DOMAIN ?
				DEFAULT_VMID_0 : 0;
			mmio_write_32(racw, tmp);
		}
	}
out:
	dmbsy();
	isb();
}

void xpu3_hal_set_mpu_dynamic_permissions(struct xpu_instance *xpu,
					  uint32_t rg_num, uint32_t perm_r,
					  uint32_t perm_w)
{
	struct rg_partition_range *range = xpu->partition_range;
	uintptr_t base = xpu->xpu_base_addr;
	uintptr_t racr, racw;
	uintptr_t cr1, cr3;
	uint32_t tmp;

	if (xpu3_hal_get_xpu_type(xpu) != XPU_TYPE_MPU)
		return;

	for (size_t i = 0; i < xpu->part_range_arr_size; i++, range++) {
		if (rg_num != XPU_RG_ALL && range->rg_num != rg_num)
			continue;

		cr1 = base + XPU3_RGN_CR1_OFFSET(range->rg_num);
		cr3 = base + XPU3_RGN_CR3_OFFSET(range->rg_num);

		mmio_write_32(cr1, perm_r);
		mmio_write_32(cr3, perm_w);

		if (!vmid_supported(xpu))
			continue;

		racr = base + XPU3_RGN_RACR_OFFSET(range->rg_num);
		tmp = perm_r & APPS_NS_DOMAIN ? DEFAULT_VMID_0 : 0;
		mmio_write_32(racr, tmp);

		racw = base + XPU3_RGN_WACR_OFFSET(range->rg_num);
		tmp = perm_w & APPS_NS_DOMAIN ? DEFAULT_VMID_0 : 0;
		mmio_write_32(racw, tmp);
	}
}

void xpu3_hal_program_mpu_partitions(struct xpu_instance *xpu, uint32_t rg_num)
{
	struct rg_partition_range *range = xpu->partition_range;
	uint32_t start_lo, start_hi, end_lo, end_hi;
	uintptr_t start_0, start_1, end_0, end_1;

	if (xpu3_hal_get_xpu_type(xpu) != XPU_TYPE_MPU)
		return;

	for (size_t i = 0; i < xpu->part_range_arr_size; i++, range++) {
		if (rg_num != XPU_RG_ALL && range->rg_num != rg_num)
			continue;

		start_lo = (uint32_t)range->start_addr;
		start_hi = (uint32_t)(range->start_addr >> 32);

		end_lo = (uint32_t)range->end_addr;
		end_hi = (uint32_t)(range->end_addr >> 32);

		start_0 = xpu->xpu_base_addr +
			  XPU3_RGN_START0_OFFSET(range->rg_num);
		start_1 = xpu->xpu_base_addr +
			  XPU3_RGN_START1_OFFSET(range->rg_num);
		end_0 = xpu->xpu_base_addr +
			XPU3_RGN_END0_OFFSET(range->rg_num);
		end_1 = xpu->xpu_base_addr +
			XPU3_RGN_END1_OFFSET(range->rg_num);

		/* Set the specified address range in the partition */
		mmio_write_32(start_0, start_lo);
		mmio_write_32(start_1, start_hi);
		mmio_write_32(end_0, end_lo);
		mmio_write_32(end_1, end_hi);
	}

	dmbsy();
	isb();
}

void xpu3_hal_set_region_ownership(struct xpu_instance *xpu, uint32_t rg_num)
{
	struct rg_domain_ownership *owner = xpu->rg_owner;
	uintptr_t base = xpu->xpu_base_addr;
	uintptr_t gcr0;

	for (size_t i = 0; i < xpu->owner_arr_size; i++, owner++) {
		/* Handle unmapped region ownership */
		if (owner->rg_num == XPU_UMR_RG) {
			gcr0 = base + XPU3_UMR_GCR0_OFFSET;
			mmio_write_32(gcr0, owner->owner_domain);
			continue;
		}

		/* Apply to ALL or specific region */
		if (rg_num != XPU_RG_ALL && owner->rg_num != rg_num)
			continue;

		gcr0 = base + XPU3_RGN_GCR0_OFFSET(owner->rg_num);

		/* Set the specified domain as the owner */
		mmio_write_32(gcr0, owner->owner_domain);
	}
}
