/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <xpu3.h>
#include <xpu3_hal.h>
#include <xpu3_hwio.h>
#include <xpu_target_info.h>

static inline bool vmid_supported(const struct xpu_instance *xpu);

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
	if ((xpu->flags & XPU_INITIALIZED) != 0U) {
		return;
	}

	xpu->flags |= XPU_INITIALIZED;

	/* If GCR0 is already programmed */
	gcr0_val = mmio_read_32(xpu->xpu_base_addr + XPU3_GCR0_OFFSET);
	if (gcr0_val != 0U) {
		return;
	}

	/* HW revision check: enable log-mode disable on newer revisions */
	rev = mmio_read_32(xpu->xpu_base_addr + XPU3_REV_OFFSET);
	rev >>= XPU3_REV_SHFT;

	if (rev > 0x3002U) {
		mmio_write_32(xpu->xpu_base_addr + XPU3_LOG_MODE_DIS_OFFSET, 1);
		gcr0_cfg |= XPU3_GCR0_LOG_MODE_DISABLE;
	}

	/* Main XPU instance */
	mmio_write_32(xpu->xpu_base_addr + XPU3_GCR0_OFFSET, gcr0_cfg);
	mmio_write_32(xpu->xpu_base_addr + XPU3_CR0_OFFSET,  XPU3_CR0_INTR_ENABLE);
	mmio_write_32(xpu->xpu_base_addr + XPU3_SCR0_OFFSET, XPU3_CR0_INTR_ENABLE);

	/* QAD0 */
	mmio_write_32(xpu->xpu_base_addr + XPU3_QAD0_CR0_OFFSET,
		      XPU3_CR0_INTR_ENABLE);
	mmio_write_32(xpu->xpu_base_addr + XPU3_QAD0_GCR0_OFFSET, gcr0_cfg);

	/* QAD1 */
	mmio_write_32(xpu->xpu_base_addr + XPU3_QAD1_CR0_OFFSET,
		      XPU3_CR0_INTR_ENABLE);
	mmio_write_32(xpu->xpu_base_addr + XPU3_QAD1_GCR0_OFFSET, gcr0_cfg);

	/*
	 * Program unmapped-region permissions from the instance-level fields.
	 * Mirrors downstream tzbsp_xpu_cfg_t::unmapped_rvmid/unmapped_wvmid.
	 * UMR_GCR0 owner is programmed by set_region_ownership via XPU_UMR_RG
	 * entries; here we only set the client permission registers.
	 */
	if (xpu->unmapped_rvmid || xpu->unmapped_wvmid) {
		uintptr_t base = xpu->xpu_base_addr;

		mmio_write_32(base + XPU3_UMR_CR1_OFFSET, xpu->unmapped_rvmid);
		mmio_write_32(base + XPU3_UMR_CR3_OFFSET, xpu->unmapped_wvmid);

		if (xpu->unmapped_rvmid & APPS_S_DOMAIN)
			mmio_write_32(base + XPU3_UMR_CR0_OFFSET, 1);
		if (xpu->unmapped_wvmid & APPS_S_DOMAIN)
			mmio_write_32(base + XPU3_UMR_CR2_OFFSET, 1);

		if (vmid_supported(xpu)) {
			uint32_t tmp;

			tmp = (xpu->unmapped_rvmid & APPS_NS_DOMAIN) ?
				DEFAULT_VMID_0 : 0;
			mmio_write_32(base + XPU3_UMR_RACR_OFFSET, tmp);

			tmp = (xpu->unmapped_wvmid & APPS_NS_DOMAIN) ?
				DEFAULT_VMID_0 : 0;
			mmio_write_32(base + XPU3_UMR_WACR_OFFSET, tmp);
		}
	}

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
	struct rg_mpu_cfg *rg = xpu->rg.mpu;
	struct rg_mpu_cfg *umr = NULL;
	uintptr_t base = xpu->xpu_base_addr;
	uintptr_t cr0, cr1, cr2, cr3;
	uintptr_t racr, racw;
	bool no_perms;
	uint32_t tmp;

	if (xpu3_hal_get_xpu_type(xpu) != XPU_TYPE_MPU) {
		return;
	}

	for (uint32_t i = 0; i < xpu->nrg; i++, rg++) {
		if (rg->index == XPU_UMR_RG) {
			umr = rg;
			continue;
		}

		if (rg_num != XPU_RG_ALL && rg->index != rg_num) {
			continue;
		}

		no_perms = (rg->read_vmid == 0 && rg->write_vmid == 0);
		if (no_perms) {
			continue;
		}

		cr1 = base + XPU3_RGN_CR1_OFFSET(rg->index);
		cr3 = base + XPU3_RGN_CR3_OFFSET(rg->index);
		cr0 = base + XPU3_RGN_CR0_OFFSET(rg->index);
		cr2 = base + XPU3_RGN_CR2_OFFSET(rg->index);

		mmio_write_32(cr1, rg->read_vmid);
		mmio_write_32(cr3, rg->write_vmid);

		if (rg->read_vmid & APPS_S_DOMAIN) {
			mmio_write_32(cr0, 1);
		}

		if (rg->write_vmid & APPS_S_DOMAIN) {
			mmio_write_32(cr2, 1);
		}

		if (vmid_supported(xpu)) {
			racr = base + XPU3_RGN_RACR_OFFSET(rg->index);
			tmp = rg->read_vmid & APPS_NS_DOMAIN ? DEFAULT_VMID_0 : 0;
			mmio_write_32(racr, tmp);

			racw = base + XPU3_RGN_WACR_OFFSET(rg->index);
			tmp = rg->write_vmid & APPS_NS_DOMAIN ? DEFAULT_VMID_0 : 0;
			mmio_write_32(racw, tmp);
		}
	}

	/* Handle unmapped region if present. */
	if (umr != NULL) {
		no_perms = (umr->read_vmid == 0 && umr->write_vmid == 0);
		if (!no_perms) {
			cr1 = base + XPU3_UMR_CR1_OFFSET;
			cr3 = base + XPU3_UMR_CR3_OFFSET;
			cr0 = base + XPU3_UMR_CR0_OFFSET;
			cr2 = base + XPU3_UMR_CR2_OFFSET;

			mmio_write_32(cr1, umr->read_vmid);
			mmio_write_32(cr3, umr->write_vmid);

			if (umr->read_vmid & APPS_S_DOMAIN) {
				mmio_write_32(cr0, 1);
			}

			if (umr->write_vmid & APPS_S_DOMAIN) {
				mmio_write_32(cr2, 1);
			}

			if (vmid_supported(xpu)) {
				racr = base + XPU3_UMR_RACR_OFFSET;
				tmp = umr->read_vmid & APPS_NS_DOMAIN ?
					DEFAULT_VMID_0 : 0;
				mmio_write_32(racr, tmp);

				racw = base + XPU3_UMR_WACR_OFFSET;
				tmp = umr->write_vmid & APPS_NS_DOMAIN ?
					DEFAULT_VMID_0 : 0;
				mmio_write_32(racw, tmp);
			}
		}
	}

	dmbsy();
	isb();
}

void xpu3_hal_set_apu_permissions(struct xpu_instance *xpu, uint32_t rg_num)
{
	struct rg_rpu_cfg *owner = xpu->rg.rpu;
	uintptr_t base = xpu->xpu_base_addr;
	uintptr_t cr0, cr1, cr2, cr3;
	uintptr_t racr, racw;
	bool no_perms;
	uint32_t tmp;
	uint8_t type = xpu3_hal_get_xpu_type(xpu);

	if (type != XPU_TYPE_APU && type != XPU_TYPE_RPU) {
		return;
	}

	for (uint32_t i = 0; i < xpu->nrg; i++, owner++) {
		if (rg_num != XPU_RG_ALL && owner->index != rg_num) {
			continue;
		}

		no_perms = (owner->read_vmid == 0 && owner->write_vmid == 0);

		if (owner->index == XPU_UMR_RG || no_perms) {
			continue;
		}

		cr1 = base + XPU3_RGN_CR1_OFFSET(owner->index);
		cr3 = base + XPU3_RGN_CR3_OFFSET(owner->index);
		cr0 = base + XPU3_RGN_CR0_OFFSET(owner->index);
		cr2 = base + XPU3_RGN_CR2_OFFSET(owner->index);

		mmio_write_32(cr1, owner->read_vmid);
		mmio_write_32(cr3, owner->write_vmid);

		if (owner->read_vmid & APPS_S_DOMAIN) {
			mmio_write_32(cr0, 1);
		}

		if (owner->write_vmid & APPS_S_DOMAIN) {
			mmio_write_32(cr2, 1);
		}

		if (vmid_supported(xpu)) {
			racr = base + XPU3_RGN_RACR_OFFSET(owner->index);
			tmp = owner->read_vmid & APPS_NS_DOMAIN ? DEFAULT_VMID_0 : 0;
			mmio_write_32(racr, tmp);

			racw = base + XPU3_RGN_WACR_OFFSET(owner->index);
			tmp = owner->write_vmid & APPS_NS_DOMAIN ? DEFAULT_VMID_0 : 0;
			mmio_write_32(racw, tmp);
		}
	}

	dmbsy();
	isb();
}

void xpu3_hal_set_mpu_dynamic_permissions(struct xpu_instance *xpu,
					  uint32_t rg_num, uint32_t perm_r,
					  uint32_t perm_w)
{
	struct rg_mpu_cfg *rg = xpu->rg.mpu;
	uintptr_t base = xpu->xpu_base_addr;
	uintptr_t racr, racw;
	uintptr_t cr1, cr3;
	uint32_t tmp;

	if (xpu3_hal_get_xpu_type(xpu) != XPU_TYPE_MPU) {
		return;
	}

	for (uint32_t i = 0; i < xpu->nrg; i++, rg++) {
		if (rg->index == XPU_UMR_RG) {
			continue;
		}

		if (rg_num != XPU_RG_ALL && rg->index != rg_num) {
			continue;
		}

		cr1 = base + XPU3_RGN_CR1_OFFSET(rg->index);
		cr3 = base + XPU3_RGN_CR3_OFFSET(rg->index);

		mmio_write_32(cr1, perm_r);
		mmio_write_32(cr3, perm_w);

		if (!vmid_supported(xpu)) {
			continue;
		}

		racr = base + XPU3_RGN_RACR_OFFSET(rg->index);
		tmp = perm_r & APPS_NS_DOMAIN ? DEFAULT_VMID_0 : 0;
		mmio_write_32(racr, tmp);

		racw = base + XPU3_RGN_WACR_OFFSET(rg->index);
		tmp = perm_w & APPS_NS_DOMAIN ? DEFAULT_VMID_0 : 0;
		mmio_write_32(racw, tmp);
	}
}

void xpu3_hal_program_mpu_partitions(struct xpu_instance *xpu, uint32_t rg_num)
{
	struct rg_mpu_cfg *rg = xpu->rg.mpu;
	uintptr_t base = xpu->xpu_base_addr;

	if (xpu3_hal_get_xpu_type(xpu) != XPU_TYPE_MPU) {
		return;
	}

	for (uint32_t i = 0; i < xpu->nrg; i++, rg++) {
		if (rg->index == XPU_UMR_RG) {
			continue;
		}

		if (rg_num != XPU_RG_ALL && rg->index != rg_num) {
			continue;
		}

		/*
		 * Disable the partition before programming. An active RG with
		 * start=0 end=0 would protect the full address range; disabling
		 * first prevents that window during the update.
		 */
		mmio_write_32(base + XPU3_RGN_GCR1_OFFSET(rg->index),
			      XPU3_RGN_GCR1_PD_BIT);

		/*
		 * Some MPUs use base-relative addresses. Apply per-XPU masking
		 * before programming the shadow registers.
		 */
		uintptr_t start = rg->start;
		uintptr_t end   = rg->end;

		for (uint32_t m = 0; m < xpu_addr_mask_count; m++) {
			if (xpu_addr_mask_table[m].xpu_id == xpu->xpu_id) {
				start -= xpu_addr_mask_table[m].base_addr;
				end   -= xpu_addr_mask_table[m].base_addr;
				start &= xpu_addr_mask_table[m].mask;
				end   &= xpu_addr_mask_table[m].mask;
				break;
			}
		}

		/*
		 * Atomic address update via shadow registers:
		 * ABORT clears the shadow, write new addresses, COMMIT applies
		 * them atomically to this RG.
		 */
		mmio_write_32(base + XPU3_RGN_GCR2_OFFSET(rg->index),
			      XPU3_RGN_GCR2_ABORT);
		mmio_write_32(base + XPU3_RGN_START0_SSHADOW_OFFSET,
			      (uint32_t)start);
		mmio_write_32(base + XPU3_RGN_START1_SSHADOW_OFFSET,
			      (uint32_t)(start >> 32));
		mmio_write_32(base + XPU3_RGN_END0_SSHADOW_OFFSET,
			      (uint32_t)end);
		mmio_write_32(base + XPU3_RGN_END1_SSHADOW_OFFSET,
			      (uint32_t)(end >> 32));
		mmio_write_32(base + XPU3_RGN_GCR2_OFFSET(rg->index),
			      XPU3_RGN_GCR2_COMMIT);

		/* Re-enable the partition. */
		mmio_write_32(base + XPU3_RGN_GCR1_OFFSET(rg->index), 0);
	}

	dmbsy();
	isb();
}

void xpu3_hal_set_region_ownership(struct xpu_instance *xpu, uint32_t rg_num)
{
	uintptr_t base = xpu->xpu_base_addr;
	uintptr_t gcr0;

	if (xpu3_hal_get_xpu_type(xpu) == XPU_TYPE_MPU) {
		struct rg_mpu_cfg *rg = xpu->rg.mpu;

		for (uint32_t i = 0; i < xpu->nrg; i++, rg++) {
			if (rg->index == XPU_UMR_RG) {
				gcr0 = base + XPU3_UMR_GCR0_OFFSET;
				mmio_write_32(gcr0, rg->flags);
				continue;
			}

			if (rg_num != XPU_RG_ALL && rg->index != rg_num) {
				continue;
			}

			gcr0 = base + XPU3_RGN_GCR0_OFFSET(rg->index);
			mmio_write_32(gcr0, rg->flags);
		}
	} else {
		struct rg_rpu_cfg *owner = xpu->rg.rpu;

		for (uint32_t i = 0; i < xpu->nrg; i++, owner++) {
			/* UMR programming is MPU-only — skip for APU/RPU. */
			if (owner->index == XPU_UMR_RG) {
				continue;
			}

			if (rg_num != XPU_RG_ALL && owner->index != rg_num) {
				continue;
			}

			gcr0 = base + XPU3_RGN_GCR0_OFFSET(owner->index);
			mmio_write_32(gcr0, owner->flags);
		}
	}
}
