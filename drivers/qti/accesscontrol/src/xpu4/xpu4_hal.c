/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <ac_cfg.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <xpu4.h>
#include <xpu4_hal.h>
#include <xpu4_hwio.h>

#define APNSPE_ENABLED		1U
#define RGE_ENABLED		1U
#define WOWP_ENABLED		1U

/* Bit written to GRP{0,1}_RG_PRG to trigger an XPRESSCFG profile. */
#define XPRESSCFG_TRIGGER_BIT	0x1U

/*
 * On an XPRESSCFG-capable XPU a set of registers forms a bitmap with one bit
 * per resource group. This gives the number of 32-bit words needed to cover
 * nrg groups.
 */
#define NRG_TO_IGNORED_REG_COUNT(nrg) \
	(((nrg) + XPU4_HW_POLICY_IGNORED_RG_PER_REG - 1U) / \
	 XPU4_HW_POLICY_IGNORED_RG_PER_REG)

/* Increment without wrapping. */
#define UINT32_INC_SATURATE(c)			\
	do {					\
		if ((c) < UINT32_MAX) {		\
			(c)++;			\
		}				\
	} while (false)

/* Static configuration of one resource group, flattened from the image. */
struct xpu4_rg_static_config {
	uint64_t start;
	uint64_t end;
	uint32_t rg_num;
	uint32_t rg_profile_flags;
	uint32_t read_qads;
	uint32_t write_qads;
};

static struct xpu4_violation *silent_log;
static uint32_t silent_log_capacity;
static uint32_t silent_log_count;
static uint32_t distinct_violations;

/*
 * Register accessors.
 */

static inline uint32_t xpu4_read(const struct ac_xpu4_priv_info *xpu,
				 uint32_t offset)
{
	return mmio_read_32((uintptr_t)xpu->addr + offset);
}

static inline void xpu4_write(const struct ac_xpu4_priv_info *xpu,
			      uint32_t offset, uint32_t value)
{
	mmio_write_32((uintptr_t)xpu->addr + offset, value);
}

static inline bool xpu4_read_apps1_rg_prg(const struct ac_xpu4_priv_info *xpu)
{
	uint32_t val = xpu4_read(xpu, XPU4_APPS_GRP1_RG_PRG_OFFSET);

	return ((val & XPU4_APPS_GRP1_RG_PRG_BMSK) >>
		XPU4_APPS_GRP1_RG_PRG_SHFT) == XPRESSCFG_TRIGGER_BIT;
}

static inline void xpu4_write_apps1_rg_prg(const struct ac_xpu4_priv_info *xpu,
					   uint32_t value)
{
	mmio_clrsetbits_32((uintptr_t)xpu->addr + XPU4_APPS_GRP1_RG_PRG_OFFSET,
			   XPU4_APPS_GRP1_RG_PRG_BMSK,
			   value << XPU4_APPS_GRP1_RG_PRG_SHFT);
}

static inline ac_xpu4_qad_vector
xpu4_read_umr_perms(const struct ac_xpu4_priv_info *xpu)
{
	return xpu4_read(xpu, XPU4_UMRPERMREG_OFFSET);
}

static inline void xpu4_write_umr_perms(const struct ac_xpu4_priv_info *xpu,
					ac_xpu4_qad_vector umr_perm)
{
	xpu4_write(xpu, XPU4_UMRPERMREG_OFFSET, umr_perm);
}

static inline ac_xpu4_qad_vector
xpu4_read_cfg_owner(const struct ac_xpu4_priv_info *xpu)
{
	return xpu4_read(xpu, XPU4_CFGOWNER_OFFSET);
}

static inline void xpu4_write_cfg_owner(const struct ac_xpu4_priv_info *xpu,
					ac_xpu4_qad_vector cfg_owner)
{
	xpu4_write(xpu, XPU4_CFGOWNER_OFFSET, cfg_owner);
}

static inline uint32_t
xpu4_read_hw_policy_ignored(const struct ac_xpu4_priv_info *xpu, uint16_t index)
{
	return xpu4_read(xpu, XPU4_HW_POLICY_IGNORED_OFFSET(index));
}

static inline void
xpu4_write_hw_policy_ignored(const struct ac_xpu4_priv_info *xpu,
			     uint32_t value, uint16_t index)
{
	xpu4_write(xpu, XPU4_HW_POLICY_IGNORED_OFFSET(index), value);
}

static inline bool xpu4_read_apnspe(const struct ac_xpu4_priv_info *xpu)
{
	uint32_t val = xpu4_read(xpu, XPU4_GCR_OFFSET);

	return XPU4_FIELD_GET(GCR, APNSPE, val) == APNSPE_ENABLED;
}

static inline void xpu4_write_apnspe(const struct ac_xpu4_priv_info *xpu,
				     bool enable)
{
	uint32_t value = enable ? APNSPE_ENABLED : 0U;

	mmio_clrsetbits_32((uintptr_t)xpu->addr + XPU4_GCR_OFFSET,
			   XPU4_GCR_APNSPE_BMSK,
			   value << XPU4_GCR_APNSPE_SHFT);
}

static inline uint32_t
xpu4_read_boot_cfg_status(const struct ac_xpu4_priv_info *xpu)
{
	return xpu4_read(xpu, XPU4_BOOT_CFG_STS_OFFSET);
}

static void xpu4_read_syndrome(const struct ac_xpu4_priv_info *xpu,
			       struct xpu4_error_syndrome *syndrome)
{
	syndrome->esr = xpu4_read(xpu, XPU4_ESR_OFFSET);
	syndrome->synar0 = xpu4_read(xpu, XPU4_SYNAR0_OFFSET);
	syndrome->synar1 = xpu4_read(xpu, XPU4_SYNAR1_OFFSET);
	syndrome->synr0 = xpu4_read(xpu, XPU4_SYNR0_OFFSET);
	syndrome->synr1 = xpu4_read(xpu, XPU4_SYNR1_OFFSET);
	syndrome->synr2 = xpu4_read(xpu, XPU4_SYNR2_OFFSET);
}

static void xpu4_read_rg_addresses(const struct ac_xpu4_priv_info *xpu,
				   uint32_t rg_num, uint64_t *start,
				   uint64_t *end)
{
	uint64_t hw_start = 0U;
	uint64_t hw_end = 0U;

	/* Usable client port address width is IDR.CLIENT_ADDR_WIDTH + 1. */
	if (xpu4_idr_client_addr_width(xpu) > 31U) {
		hw_start = xpu4_read(xpu, XPU4_RGCSAR1_OFFSET(rg_num));
		hw_start <<= 32;
		hw_end = xpu4_read(xpu, XPU4_RGCEAR1_OFFSET(rg_num));
		hw_end <<= 32;
	}

	hw_start |= xpu4_read(xpu, XPU4_RGCSAR0_OFFSET(rg_num));
	hw_end |= xpu4_read(xpu, XPU4_RGCEAR0_OFFSET(rg_num));

	*start = hw_start;
	*end = hw_end;
}

static inline void xpu4_read_rg_perm(const struct ac_xpu4_priv_info *xpu,
				     uint32_t rg_num,
				     ac_xpu4_qad_vector *read_perm,
				     ac_xpu4_qad_vector *write_perm)
{
	*read_perm = xpu4_read(xpu, XPU4_RGRDR_OFFSET(rg_num));
	*write_perm = xpu4_read(xpu, XPU4_RGWRR_OFFSET(rg_num));
}

static inline bool xpu4_read_rg_enable(const struct ac_xpu4_priv_info *xpu,
				       uint32_t rg_num)
{
	uint32_t val = xpu4_read(xpu, XPU4_RGCR1_OFFSET(rg_num));

	return XPU4_FIELD_GET(RGCR1, RGE, val) == RGE_ENABLED;
}

static inline void xpu4_write_rg_enable(const struct ac_xpu4_priv_info *xpu,
					uint32_t rg_num, bool enable)
{
	uint32_t value = enable ? RGE_ENABLED : 0U;

	mmio_clrsetbits_32((uintptr_t)xpu->addr + XPU4_RGCR1_OFFSET(rg_num),
			   XPU4_RGCR1_RGE_BMSK,
			   value << XPU4_RGCR1_RGE_SHFT);
}

static inline bool
xpu4_read_rg_write_protected(const struct ac_xpu4_priv_info *xpu,
			     uint32_t rg_num)
{
	uint32_t val = xpu4_read(xpu, XPU4_RGCR0_OFFSET(rg_num));

	return XPU4_FIELD_GET(RGCR0, RGWOWP, val) == WOWP_ENABLED;
}

static inline ac_xpu4_qad_vector
xpu4_read_rg_lock(const struct ac_xpu4_priv_info *xpu, uint32_t rg_num)
{
	return xpu4_read(xpu, XPU4_QADRGL_OFFSET(rg_num));
}

/* Set or clear lock bits. lock_vec must not be zero. */
static void xpu4_write_rg_lock(const struct ac_xpu4_priv_info *xpu,
			       uint32_t rg_num, ac_xpu4_qad_vector lock_vec,
			       bool lock)
{
	ac_xpu4_qad_vector apps_qad = xpu4_get_apps_qad_vector();
	uint32_t new_value = lock ? lock_vec : (apps_qad & ~lock_vec);

	/*
	 * Hardware erratum: locking AP-SEC or AP-NS but not both loses the
	 * other, so fold whatever is already locked back in.
	 */
	if (lock && new_value != 0U && new_value != apps_qad) {
		uint32_t old_value = xpu4_read_rg_lock(xpu, rg_num);

		new_value |= old_value & apps_qad;
	}

	xpu4_write(xpu, XPU4_QADRGL_OFFSET(rg_num), new_value);
}

/*
 * Lock vector calculation.
 */

/* Restricted lock-delegate mask for an XPU, or all-ones if unrestricted. */
static void xpu4_get_lock_delegate_mask(const struct ac_xpu4_priv_info *xpu,
					uint32_t *mask)
{
	static const struct ac_xpu_id_to_qad_mask *masks;
	static uint32_t masks_count;
	static bool initialised;
	uint32_t i;

	if (!initialised) {
		if (ac_cfg_get_lock_delegate_masks(&masks, &masks_count) != 0) {
			masks = NULL;
			masks_count = 0U;
		}
		initialised = true;
	}

	for (i = 0U; i < masks_count; i++) {
		if ((uint32_t)masks[i].xpu_id == xpu->xpu_id) {
			*mask = masks[i].mask;
			return;
		}
	}

	*mask = ~0U;
}

/*
 * Decide which QADs to lock for a resource group.
 *
 * Below revision 4.3.0 only the executing environment is locked. From 4.3.0 the
 * LOCK_DELEGATE feature allows locking every QAD holding a permission, subject
 * to the per-XPU delegate mask.
 *
 * RA XPUs are excluded because only APPS configures them; locking other QADs
 * would close the group to reprogramming. APUs and RPUs are excluded because
 * clients may reconfigure them at runtime.
 */
static ac_xpu4_qad_vector
xpu4_calculate_lock_vector(const struct ac_xpu4_priv_info *xpu,
			   ac_xpu4_qad_vector read_qads,
			   ac_xpu4_qad_vector write_qads)
{
	ac_xpu4_qad_vector env_qad = xpu4_get_apps_qad_vector();
	ac_xpu4_qad_vector calculated = read_qads | write_qads;
	uint32_t delegate_mask = ~0U;

	if (xpu->rev < XPU4_REV(4, 3, 0) ||
	    xpu4_idr_xpu_type(xpu) != (uint32_t)AC_MPU ||
	    xpu4_is_ra_xpu(xpu->xpu_id)) {
		calculated &= env_qad;
	} else {
		xpu4_get_lock_delegate_mask(xpu, &delegate_mask);
		calculated &= delegate_mask | env_qad;
	}

	return calculated;
}

/*
 * Resource group search. MPU only.
 */

/*
 * Find the enabled dynamic RG whose range matches exactly, and count how many
 * dynamic RGs overlap the range at all. The overlap count is valid on both
 * XPU4_OK and XPU4_ERR_RG_NOT_FOUND, and may exceed one even on a match.
 */
static enum xpu4_status
xpu4_locate_enabled_dyn_rg(const struct ac_xpu4_priv_info *xpu, uint64_t start,
			   uint64_t end, uint32_t *rg_num, uint32_t *overlaps)
{
	uint32_t rg_start;
	uint32_t rg_end;
	uint32_t matched;
	uint32_t count = 0U;
	uint32_t i;

	if (xpu->dyn_rgs == NULL) {
		ERROR("xpu4: no dynamic RG info soc=0x%x\n", xpu->soc_addr);
		return XPU4_ERR_NO_DYNAMIC_RG_INFO;
	}

	rg_start = xpu->dyn_rgs->rg_start;
	rg_end = xpu->dyn_rgs->rg_start + xpu->dyn_rgs->rg_count;

	matched = rg_end;	/* an invalid value */

	for (i = rg_start; i < rg_end; i++) {
		uint64_t hw_start;
		uint64_t hw_end;

		if (!xpu4_read_rg_enable(xpu, i)) {
			ac_xpu4_qad_vector read_vec;
			ac_xpu4_qad_vector write_vec;

			xpu4_hal_get_shadow_perm(xpu, i, &read_vec, &write_vec);
			if ((read_vec | write_vec) == 0U) {
				/* Free only if the shadow is empty too. */
				continue;
			}
		}

		xpu4_read_rg_addresses(xpu, i, &hw_start, &hw_end);
		if (hw_end <= start || end <= hw_start) {
			continue;
		}

		count++;
		if (hw_start == start && hw_end == end && matched == rg_end) {
			matched = i;
		}
	}

	*overlaps = count;
	if (matched == rg_end) {
		return XPU4_ERR_RG_NOT_FOUND;
	}

	*rg_num = matched;

	return XPU4_OK;
}

/* Find the enabled static RG whose range matches exactly. MPU only. */
static enum xpu4_status
xpu4_locate_enabled_static_rg(const struct ac_xpu4_priv_info *xpu,
			      uint64_t start, uint64_t end, uint32_t *rg_num)
{
	uint32_t nrg;
	uint32_t dyn_start;
	uint32_t dyn_end;
	uint32_t i;

	nrg = xpu4_idr_nrg(xpu) + 1U;	/* Usable count is IDR.NRG + 1 */

	if (xpu->dyn_rgs != NULL) {
		dyn_start = xpu->dyn_rgs->rg_start;
		dyn_end = xpu->dyn_rgs->rg_start + xpu->dyn_rgs->rg_count;
	} else {
		/* An empty, never-matching window. */
		dyn_start = nrg;
		dyn_end = nrg;
	}

	for (i = 0U; i < nrg; i++) {
		uint64_t hw_start;
		uint64_t hw_end;

		if (dyn_start <= i && i < dyn_end) {
			continue;
		}

		if (!xpu4_read_rg_enable(xpu, i)) {
			continue;
		}

		xpu4_read_rg_addresses(xpu, i, &hw_start, &hw_end);
		if (hw_start == start && hw_end == end) {
			*rg_num = i;
			return XPU4_OK;
		}
	}

	return XPU4_ERR_RG_NOT_FOUND;
}

/* Find an unused dynamic RG. MPU only. */
static enum xpu4_status
xpu4_locate_free_dyn_rg(const struct ac_xpu4_priv_info *xpu, uint32_t *rg_num)
{
	uint32_t rg_start;
	uint32_t rg_end;
	uint32_t i;

	if (xpu->dyn_rgs == NULL) {
		ERROR("xpu4: no dynamic RG info soc=0x%x\n", xpu->soc_addr);
		return XPU4_ERR_NO_DYNAMIC_RG_INFO;
	}

	rg_start = xpu->dyn_rgs->rg_start;
	rg_end = xpu->dyn_rgs->rg_start + xpu->dyn_rgs->rg_count;

	/* The RSR register cannot be used because of shadow permissions. */
	for (i = rg_start; i < rg_end; i++) {
		ac_xpu4_qad_vector read_vec;
		ac_xpu4_qad_vector write_vec;

		if (xpu4_read_rg_enable(xpu, i)) {
			continue;
		}

		xpu4_hal_get_shadow_perm(xpu, i, &read_vec, &write_vec);
		if ((read_vec | write_vec) != 0U) {
			/* Still in use according to the shadow. */
			continue;
		}

		*rg_num = i;

		return XPU4_OK;
	}

	ERROR("xpu4: no free RG soc=0x%x\n", xpu->soc_addr);

	return XPU4_ERR_NO_FREE_RG;
}

/*
 * Resource group programming.
 */

/* Read back a resource group and check it against the intended values. */
static enum xpu4_status
xpu4_read_and_check_rg_cfg(const struct ac_xpu4_priv_info *xpu, uint32_t rg_num,
			   uint64_t start, uint64_t end, const bool *wowp,
			   ac_xpu4_qad_vector read_perm,
			   ac_xpu4_qad_vector write_perm)
{
	ac_xpu4_qad_vector hw_read;
	ac_xpu4_qad_vector hw_write;

	if (wowp != NULL &&
	    xpu4_read_rg_write_protected(xpu, rg_num) != *wowp) {
		ERROR("xpu4: WOWP mismatch soc=0x%x rg=%u\n", xpu->soc_addr,
		      rg_num);
		return XPU4_ERR_RG_WOWP_MISMATCH;
	}

	if (!xpu4_read_rg_enable(xpu, rg_num)) {
		ERROR("xpu4: RG not enabled soc=0x%x rg=%u\n", xpu->soc_addr,
		      rg_num);
		return XPU4_ERR_RG_NOT_ENABLED;
	}

	xpu4_read_rg_perm(xpu, rg_num, &hw_read, &hw_write);
	if (hw_read != read_perm || hw_write != write_perm) {
		ERROR("xpu4: perm mismatch soc=0x%x rg=%u\n", xpu->soc_addr,
		      rg_num);
		ERROR("  read want=0x%x got=0x%x write want=0x%x got=0x%x\n",
		      read_perm, hw_read, write_perm, hw_write);
		return XPU4_ERR_RG_PERM_MISMATCH;
	}

	if (xpu4_idr_xpu_type(xpu) == (uint32_t)AC_MPU) {
		uint64_t hw_start;
		uint64_t hw_end;

		xpu4_read_rg_addresses(xpu, rg_num, &hw_start, &hw_end);
		if (hw_start != start || hw_end != end) {
			ERROR("xpu4: addr mismatch soc=0x%x rg=%u\n",
			      xpu->soc_addr, rg_num);
			return XPU4_ERR_RG_ADDR_MISMATCH;
		}
	}

	return XPU4_OK;
}

/*
 * Write a resource group. Addresses are programmed for MPUs only; permissions,
 * enable and write-protect apply to every unit type.
 */
static void xpu4_write_rg_cfg(const struct ac_xpu4_priv_info *xpu,
			      uint32_t rg_num, uint64_t start, uint64_t end,
			      bool wowp, ac_xpu4_qad_vector read_perm,
			      ac_xpu4_qad_vector write_perm)
{
	if (xpu4_idr_xpu_type(xpu) == (uint32_t)AC_MPU) {
		xpu4_write(xpu, XPU4_RGCSAR0_OFFSET(rg_num),
			   (uint32_t)(start & UINT32_MAX));
		xpu4_write(xpu, XPU4_RGCEAR0_OFFSET(rg_num),
			   (uint32_t)(end & UINT32_MAX));

		/* Usable client address width is IDR.CLIENT_ADDR_WIDTH + 1. */
		if (xpu4_idr_client_addr_width(xpu) > 31U) {
			xpu4_write(xpu, XPU4_RGCSAR1_OFFSET(rg_num),
				   (uint32_t)(start >> 32));
			xpu4_write(xpu, XPU4_RGCEAR1_OFFSET(rg_num),
				   (uint32_t)(end >> 32));
		}
	}

	xpu4_write(xpu, XPU4_RGRDR_OFFSET(rg_num), read_perm);
	xpu4_write(xpu, XPU4_RGWRR_OFFSET(rg_num), write_perm);

	xpu4_write_rg_enable(xpu, rg_num, true);

	if (wowp) {
		mmio_clrsetbits_32((uintptr_t)xpu->addr +
					   XPU4_RGCR0_OFFSET(rg_num),
				   XPU4_RGCR0_RGWOWP_BMSK,
				   WOWP_ENABLED << XPU4_RGCR0_RGWOWP_SHFT);
	}
}

/* Program a resource group from scratch. Does not update shadow permissions. */
static enum xpu4_status
xpu4_protect_by_rg(const struct ac_xpu4_priv_info *xpu, uint32_t rg_num,
		   uint64_t start, uint64_t end, bool wowp,
		   ac_xpu4_qad_vector read_perm,
		   ac_xpu4_qad_vector write_perm, ac_xpu4_qad_vector lock)
{
	xpu4_write_rg_cfg(xpu, rg_num, start, end, wowp, read_perm, write_perm);

	if (lock != 0U) {
		xpu4_write_rg_lock(xpu, rg_num, lock, true);
		if ((xpu4_read_rg_lock(xpu, rg_num) & lock) != lock) {
			ERROR("xpu4: lock mismatch soc=0x%x rg=%u\n",
			      xpu->soc_addr, rg_num);
			return XPU4_ERR_RG_WRITE_MISMATCH;
		}
	}

	return xpu4_read_and_check_rg_cfg(xpu, rg_num, start, end, &wowp,
					  read_perm, write_perm);
}

/*
 * Adjust the permissions or locks of a group that is already enabled or locked,
 * by setting and clearing lock bits. Returns XPU4_ERR_BEYOND_FIX_UP when the
 * change cannot be made in place, which tells the caller to reallocate.
 *
 * Does not update shadow permissions.
 */
static enum xpu4_status
xpu4_fix_up_by_rg(const struct ac_xpu4_priv_info *xpu, uint32_t rg_num,
		  uint64_t start, uint64_t end, bool wowp,
		  ac_xpu4_qad_vector read_perm, ac_xpu4_qad_vector write_perm,
		  ac_xpu4_qad_vector lock)
{
	ac_xpu4_qad_vector apps_qad = xpu4_get_apps_qad_vector();
	ac_xpu4_qad_vector hw_read;
	ac_xpu4_qad_vector hw_write;
	ac_xpu4_qad_vector hw_lock;

	if (xpu4_idr_xpu_type(xpu) == (uint32_t)AC_MPU) {
		uint64_t hw_start;
		uint64_t hw_end;

		xpu4_read_rg_addresses(xpu, rg_num, &hw_start, &hw_end);
		if (hw_start != start || hw_end != end) {
			/* A different range cannot be fixed up in place. */
			return XPU4_ERR_BEYOND_FIX_UP;
		}
	}

	xpu4_read_rg_perm(xpu, rg_num, &hw_read, &hw_write);
	hw_lock = xpu4_read_rg_lock(xpu, rg_num);

	if (hw_read == read_perm && hw_write == write_perm) {
		/*
		 * Permissions already correct, so the only possible fix up is
		 * setting further lock bits.
		 */
		if (lock != 0U) {
			ac_xpu4_qad_vector hw_apps_lock = hw_lock & apps_qad;

			if ((hw_apps_lock & lock) != hw_apps_lock) {
				/* Would drop a lock and its permission. */
				return XPU4_ERR_BEYOND_FIX_UP;
			}

			xpu4_write_rg_lock(xpu, rg_num, lock, true);
			if ((xpu4_read_rg_lock(xpu, rg_num) & lock) != lock) {
				ERROR("xpu4: lock mismatch soc=0x%x rg=%u\n",
				      xpu->soc_addr, rg_num);
				return XPU4_ERR_RG_WRITE_MISMATCH;
			}
		}
	} else {
		ac_xpu4_qad_vector mask;
		ac_xpu4_qad_vector unlock_vec;

		mask = (xpu4_hal_get_valid_qad_vector() & ~apps_qad) | lock;
		if ((mask & hw_read) != read_perm ||
		    (mask & hw_write) != write_perm ||
		    (mask & (hw_lock | lock)) == 0U) {
			/*
			 * Unlocking APPS_SEC/APPS_NS cannot get there: either
			 * the resulting permissions would be wrong, or the
			 * unlock would disable the group entirely.
			 */
			return XPU4_ERR_BEYOND_FIX_UP;
		}

		unlock_vec = 0U;
		if ((QAD_VECTOR_AP_SEC & lock) != QAD_VECTOR_AP_SEC) {
			unlock_vec |= QAD_VECTOR_AP_SEC;
		}
		if ((QAD_VECTOR_AP_NS & lock) != QAD_VECTOR_AP_NS) {
			unlock_vec |= QAD_VECTOR_AP_NS;
		}

		/*
		 * Set the wanted locks, then clear the ones whose permissions
		 * must go. unlock_vec is guaranteed non-zero here.
		 */
		xpu4_write_rg_lock(xpu, rg_num, apps_qad, true);
		xpu4_write_rg_lock(xpu, rg_num, unlock_vec, false);
		if ((xpu4_read_rg_lock(xpu, rg_num) & lock) != lock) {
			ERROR("xpu4: lock mismatch soc=0x%x rg=%u\n",
			      xpu->soc_addr, rg_num);
			return XPU4_ERR_RG_WRITE_MISMATCH;
		}
	}

	return xpu4_read_and_check_rg_cfg(xpu, rg_num, start, end, &wowp,
					  read_perm, write_perm);
}

/*
 * Release a resource group. With must_free set the group has to end up fully
 * unlocked, otherwise leaving another QAD's lock in place is tolerated.
 *
 * Does not update shadow permissions.
 */
static enum xpu4_status
xpu4_unprotect_by_rg(const struct ac_xpu4_priv_info *xpu, uint32_t rg_num,
		     bool must_free)
{
	ac_xpu4_qad_vector apps_qad = xpu4_get_apps_qad_vector();
	ac_xpu4_qad_vector hw_lock = xpu4_read_rg_lock(xpu, rg_num);

	if (must_free && (hw_lock & ~apps_qad) != 0U) {
		ERROR("xpu4: RG still locked soc=0x%x rg=%u\n", xpu->soc_addr,
		      rg_num);
		return XPU4_ERR_RG_STILL_LOCKED;
	}

	if (hw_lock != 0U) {
		xpu4_write_rg_lock(xpu, rg_num, apps_qad, false);
		hw_lock = xpu4_read_rg_lock(xpu, rg_num);

		/* Still held by an APPS QAD. */
		if ((hw_lock & apps_qad) != 0U) {
			ERROR("xpu4: lock mismatch soc=0x%x rg=%u\n",
			      xpu->soc_addr, rg_num);
			return XPU4_ERR_RG_WRITE_MISMATCH;
		}

		/* APPS released but another QAD still holds it. */
		if (must_free && hw_lock != 0U) {
			ERROR("xpu4: RG still locked soc=0x%x rg=%u\n",
			      xpu->soc_addr, rg_num);
			return XPU4_ERR_RG_STILL_LOCKED;
		}
	} else {
		xpu4_write_rg_enable(xpu, rg_num, false);
		if (xpu4_read_rg_enable(xpu, rg_num)) {
			hw_lock = xpu4_read_rg_lock(xpu, rg_num);
			if (hw_lock == 0U) {
				ERROR("xpu4: RG write mismatch soc=0x%x rg=%u\n",
				      xpu->soc_addr, rg_num);
				return XPU4_ERR_RG_WRITE_MISMATCH;
			}

			if (must_free) {
				ERROR("xpu4: RG still locked soc=0x%x rg=%u\n",
				      xpu->soc_addr, rg_num);
				return XPU4_ERR_RG_STILL_LOCKED;
			}
		}
	}

	return XPU4_OK;
}

/*
 * MPU region helpers.
 */

static enum xpu4_status
xpu4_protect_dyn_mpu_region(const struct ac_xpu4_priv_info *xpu, uint64_t start,
			    uint64_t end, bool wowp,
			    ac_xpu4_qad_vector read_perm,
			    ac_xpu4_qad_vector write_perm,
			    ac_xpu4_qad_vector lock)
{
	enum xpu4_status rc;
	uint32_t rg_num;

	rc = xpu4_locate_free_dyn_rg(xpu, &rg_num);
	if (rc != XPU4_OK) {
		return rc;
	}

	rc = xpu4_protect_by_rg(xpu, rg_num, start, end, wowp, read_perm,
				write_perm, lock);
	if (rc != XPU4_OK) {
		return rc;
	}

	xpu4_hal_cache_shadow_perm(xpu, rg_num, read_perm, write_perm);

	return XPU4_OK;
}

/*
 * Reprogram a dynamic group in place if possible, otherwise move the new
 * configuration into a free group and release the old one.
 */
static enum xpu4_status
xpu4_update_dyn_mpu_region(const struct ac_xpu4_priv_info *xpu, uint32_t rg_num,
			   uint64_t start, uint64_t end, bool wowp,
			   ac_xpu4_qad_vector read_perm,
			   ac_xpu4_qad_vector write_perm,
			   ac_xpu4_qad_vector lock)
{
	enum xpu4_status rc;
	uint32_t new_rg_num;

	rc = xpu4_fix_up_by_rg(xpu, rg_num, start, end, wowp, read_perm,
			       write_perm, lock);
	if (rc == XPU4_OK) {
		xpu4_hal_cache_shadow_perm(xpu, rg_num, read_perm, write_perm);
		return XPU4_OK;
	}
	if (rc != XPU4_ERR_BEYOND_FIX_UP) {
		return rc;
	}

	rc = xpu4_locate_free_dyn_rg(xpu, &new_rg_num);
	if (rc != XPU4_OK) {
		return rc;
	}

	rc = xpu4_protect_by_rg(xpu, new_rg_num, start, end, wowp, read_perm,
				write_perm, lock);
	if (rc != XPU4_OK) {
		return rc;
	}
	xpu4_hal_cache_shadow_perm(xpu, new_rg_num, read_perm, write_perm);

	rc = xpu4_unprotect_by_rg(xpu, rg_num, true);
	if (rc != XPU4_OK) {
		return rc;
	}
	xpu4_hal_cache_shadow_perm(xpu, rg_num, 0U, 0U);

	return XPU4_OK;
}

static enum xpu4_status
xpu4_unprotect_dyn_mpu_region(const struct ac_xpu4_priv_info *xpu,
			      uint32_t rg_num)
{
	enum xpu4_status rc;

	rc = xpu4_unprotect_by_rg(xpu, rg_num, false);
	if (rc != XPU4_OK) {
		return rc;
	}

	xpu4_hal_cache_shadow_perm(xpu, rg_num, 0U, 0U);

	return XPU4_OK;
}

/*
 * Change the permissions of a statically configured MPU region.
 *
 * When the group cannot be fixed up in place the new configuration is staged in
 * a spare dynamic group so the region is never left unprotected, then moved
 * back into the original static group and the spare released.
 */
static enum xpu4_status
xpu4_update_static_mpu_region(const struct ac_xpu4_priv_info *xpu,
			      uint64_t start, uint64_t end, bool wowp,
			      ac_xpu4_qad_vector read_perm,
			      ac_xpu4_qad_vector write_perm,
			      ac_xpu4_qad_vector lock)
{
	enum xpu4_status rc;
	uint32_t old_rg_num;
	uint32_t new_rg_num;

	rc = xpu4_locate_enabled_static_rg(xpu, start, end, &old_rg_num);
	if (rc == XPU4_ERR_RG_NOT_FOUND) {
		return XPU4_ERR_STATIC_MPU_RG_NOT_FOUND;
	}
	if (rc != XPU4_OK) {
		return rc;
	}

	rc = xpu4_fix_up_by_rg(xpu, old_rg_num, start, end, wowp, read_perm,
			       write_perm, lock);
	if (rc == XPU4_OK) {
		return XPU4_OK;
	}
	if (rc != XPU4_ERR_BEYOND_FIX_UP) {
		return rc;
	}

	rc = xpu4_locate_free_dyn_rg(xpu, &new_rg_num);
	if (rc != XPU4_OK) {
		return rc;
	}

	/* Stage the new permissions in the spare dynamic group. */
	rc = xpu4_protect_by_rg(xpu, new_rg_num, start, end, false, read_perm,
				write_perm, lock);
	if (rc != XPU4_OK) {
		return rc;
	}
	xpu4_hal_cache_shadow_perm(xpu, new_rg_num, read_perm, write_perm);

	/* Release the static group. */
	rc = xpu4_unprotect_by_rg(xpu, old_rg_num, true);
	if (rc != XPU4_OK) {
		return rc;
	}

	/* Reprogram the static group with the new permissions. */
	rc = xpu4_protect_by_rg(xpu, old_rg_num, start, end, wowp, read_perm,
				write_perm, lock);
	if (rc != XPU4_OK) {
		return rc;
	}

	/* Release the spare. */
	rc = xpu4_unprotect_by_rg(xpu, new_rg_num, true);
	if (rc != XPU4_OK) {
		return rc;
	}
	xpu4_hal_cache_shadow_perm(xpu, new_rg_num, 0U, 0U);

	return XPU4_OK;
}

/* The caller already knows which XPU and which resource group. */
static enum xpu4_status
xpu4_update_static_apu_rpu_region(const struct ac_xpu4_priv_info *xpu,
				  uint32_t rg_num, bool wowp,
				  ac_xpu4_qad_vector read_perm,
				  ac_xpu4_qad_vector write_perm,
				  ac_xpu4_qad_vector lock)
{
	enum xpu4_status rc;

	if (xpu4_read_rg_lock(xpu, rg_num) != 0U) {
		rc = xpu4_fix_up_by_rg(xpu, rg_num, UINT64_MAX, UINT64_MAX,
				       wowp, read_perm, write_perm, lock);
		if (rc == XPU4_OK) {
			return XPU4_OK;
		}
		if (rc != XPU4_ERR_BEYOND_FIX_UP) {
			return rc;
		}

		rc = xpu4_unprotect_by_rg(xpu, rg_num, true);
		if (rc != XPU4_OK) {
			return rc;
		}
	}

	return xpu4_protect_by_rg(xpu, rg_num, UINT64_MAX, UINT64_MAX, wowp,
				  read_perm, write_perm, lock);
}

/* Flatten one resource group entry from the image. */
static void xpu4_get_rg_static_config(const struct ac_xpu_cfg *cfg,
				      uint32_t index, bool is_mpu,
				      struct xpu4_rg_static_config *rg_cfg)
{
	if (is_mpu) {
		const struct ac_mpu_rg *rg = &cfg->rg.mpu[index];

		rg_cfg->start = rg->start;
		rg_cfg->end = rg->end;
		rg_cfg->rg_num = rg->rg_num;
		rg_cfg->rg_profile_flags = rg->profile_flags;
		rg_cfg->read_qads = rg->read_qads;
		rg_cfg->write_qads = rg->write_qads;
	} else {
		const struct ac_rpu_rg *rg = &cfg->rg.rpu[index];

		rg_cfg->start = UINT64_MAX;
		rg_cfg->end = UINT64_MAX;
		rg_cfg->rg_num = rg->rg_num;
		rg_cfg->rg_profile_flags = rg->profile_flags;
		rg_cfg->read_qads = rg->read_qads;
		rg_cfg->write_qads = rg->write_qads;
	}
}

/*
 * XPRESSCFG.
 */

/*
 * Write the trigger bits for the earlier execution environments if they have
 * not been set already. The supported application order is TME_FW, xBL_SC then
 * TZ, so this only covers for a stage that did not run.
 */
static enum xpu4_status
xpu4_xc_set_for_earlier_ee(const struct ac_xpu4_priv_info *xpu)
{
	uint32_t value = xpu4_read_boot_cfg_status(xpu);
	uint32_t mask;

	mask = XPU4_BOOT_CFG_STS_TME_GRP0_WO_BMSK;
	if ((value & mask) != mask) {
		mmio_clrsetbits_32((uintptr_t)xpu->addr +
					   XPU4_TME_GRP0_RG_PRG_OFFSET,
				   XPU4_TME_GRP0_RG_PRG_BMSK,
				   XPRESSCFG_TRIGGER_BIT <<
					   XPU4_TME_GRP0_RG_PRG_SHFT);
		value = xpu4_read_boot_cfg_status(xpu);
		if ((value & mask) != mask) {
			return XPU4_ERR_XC_STATUS_NOT_SET;
		}
	}

	mask = XPU4_BOOT_CFG_STS_TME_GRP1_WO_BMSK;
	if ((value & mask) != mask) {
		mmio_clrsetbits_32((uintptr_t)xpu->addr +
					   XPU4_TME_GRP1_RG_PRG_OFFSET,
				   XPU4_TME_GRP1_RG_PRG_BMSK,
				   XPRESSCFG_TRIGGER_BIT <<
					   XPU4_TME_GRP1_RG_PRG_SHFT);
		value = xpu4_read_boot_cfg_status(xpu);
		if ((value & mask) != mask) {
			return XPU4_ERR_XC_STATUS_NOT_SET;
		}
	}

	mask = XPU4_BOOT_CFG_STS_APPS_GRP0_WO_BMSK;
	if ((value & mask) != mask) {
		mmio_clrsetbits_32((uintptr_t)xpu->addr +
					   XPU4_APPS_GRP0_RG_PRG_OFFSET,
				   XPU4_APPS_GRP0_RG_PRG_BMSK,
				   XPRESSCFG_TRIGGER_BIT <<
					   XPU4_APPS_GRP0_RG_PRG_SHFT);
		value = xpu4_read_boot_cfg_status(xpu);
		if ((value & mask) != mask) {
			return XPU4_ERR_XC_STATUS_NOT_SET;
		}
	}

	return XPU4_OK;
}

/* Trigger this environment's XPRESSCFG profile. XPRESSCFG must be supported. */
static enum xpu4_status xpu4_set_1_rg_prg(const struct ac_xpu4_priv_info *xpu)
{
	xpu4_write_apps1_rg_prg(xpu, XPRESSCFG_TRIGGER_BIT);

	if (!xpu4_read_apps1_rg_prg(xpu)) {
		ERROR("xpu4: XC1 trigger mismatch soc=0x%x\n", xpu->soc_addr);
		return XPU4_ERR_XC1_TRIGGER_MISMATCH;
	}

	return XPU4_OK;
}

/*
 * Public interface.
 */

ac_xpu4_qad_vector xpu4_hal_get_valid_qad_vector(void)
{
	return QAD_VECTOR_AP_SEC |
	       QAD_VECTOR_AP_NS |
	       QAD_VECTOR_TME_ROM |
	       QAD_VECTOR_TME_FW |
	       QAD_VECTOR_DEBUG |
	       QAD_VECTOR_AOP |
	       QAD_VECTOR_MODEM |
	       QAD_VECTOR_SP |
#if AC_VM_BITMAP_SIZE == 128
	       QAD_VECTOR_OOBNS |
	       QAD_VECTOR_OOBS |
	       QAD_VECTOR_QECP_DEBUG |
#endif
	       0U;
}

enum xpu4_status xpu4_hal_init_xpu(struct ac_xpu4_priv_info *xpu)
{
	if (xpu == NULL) {
		return XPU4_ERR_NULL_POINTER;
	}

	/* Cache the revision and identity registers. */
	xpu->rev = xpu4_read(xpu, XPU4_REV_OFFSET);
	xpu->idr[0] = xpu4_read(xpu, XPU4_IDR0_OFFSET);
	xpu->idr[1] = xpu4_read(xpu, XPU4_IDR1_OFFSET);
	xpu->idr[2] = xpu4_read(xpu, XPU4_IDR2_OFFSET);

	return XPU4_OK;
}

enum xpu4_status xpu4_hal_set_umr_perms(const struct ac_xpu4_priv_info *xpu,
					const struct ac_xpu_cfg *cfg)
{
	ac_xpu4_qad_vector umr_perm = cfg->umr_perm;
	ac_xpu4_qad_vector hw_umr_perm;

	if (xpu->rev < XPU4_REV(4, 2, 0) ||
	    xpu4_idr_xpu_type(xpu) != (uint32_t)AC_MPU) {
		return XPU4_OK;
	}

	xpu4_write_umr_perms(xpu, umr_perm);
	hw_umr_perm = xpu4_read_umr_perms(xpu);
	if (umr_perm != hw_umr_perm) {
		ERROR("xpu4: UMR perm mismatch soc=0x%x want=0x%x got=0x%x\n",
		      xpu->soc_addr, umr_perm, hw_umr_perm);
		return XPU4_ERR_UMRPERM_MISMATCH;
	}

	return XPU4_OK;
}

enum xpu4_status xpu4_hal_set_cfg_owner(const struct ac_xpu4_priv_info *xpu,
					const struct ac_xpu_cfg *cfg)
{
	ac_xpu4_qad_vector cfg_owner = cfg->cfg_owner;
	ac_xpu4_qad_vector hw_cfg_owner;

	if (xpu->rev < XPU4_REV(4, 2, 0)) {
		return XPU4_OK;
	}

	xpu4_write_cfg_owner(xpu, cfg_owner);
	hw_cfg_owner = xpu4_read_cfg_owner(xpu);
	if (cfg_owner != hw_cfg_owner) {
		ERROR("xpu4: cfgowner mismatch soc=0x%x want=0x%x got=0x%x\n",
		      xpu->soc_addr, cfg_owner, hw_cfg_owner);
		return XPU4_ERR_CFGOWNER_MISMATCH;
	}

	return XPU4_OK;
}

ac_xpu4_qad_vector xpu4_hal_get_cfg_owner(const struct ac_xpu4_priv_info *xpu)
{
	ac_xpu4_qad_vector cfg_owner = QAD_VECTOR_INVALID;

	if (xpu->rev >= XPU4_REV(4, 2, 0)) {
		cfg_owner = xpu4_read_cfg_owner(xpu);
	}

	/*
	 * Before 4.2.0, or when no owner is programmed, APNSPE decides whether
	 * AP-secure owns the configuration space.
	 */
	if (cfg_owner == QAD_VECTOR_INVALID && xpu4_read_apnspe(xpu)) {
		cfg_owner = QAD_VECTOR_AP_SEC;
	}

	return cfg_owner;
}

enum xpu4_status
xpu4_hal_apply_xpress_config(const struct ac_xpu4_priv_info *xpu,
			     const struct ac_xpu_cfg *cfg)
{
	const ac_rg_ignored *rg_ignored = cfg->rg_ignored;
	enum xpu4_status rc;
	uint16_t i;

	/* Nothing to do when this instance has no XPRESSCFG support. */
	if (xpu->rev < XPU4_REV(4, 2, 0) || xpu4_idr_xpresscfg_en(xpu) == 0U) {
		return XPU4_OK;
	}

	if (rg_ignored != NULL) {
		for (i = 0U; i < NRG_TO_IGNORED_REG_COUNT(cfg->nrg + 1U); i++) {
			uint32_t hw_val;

			xpu4_write_hw_policy_ignored(xpu, rg_ignored[i], i);
			hw_val = xpu4_read_hw_policy_ignored(xpu, i);
			if (rg_ignored[i] != hw_val) {
				ERROR("xpu4: XC ignored mismatch soc=0x%x want=0x%x got=0x%x\n",
				      xpu->soc_addr, rg_ignored[i], hw_val);
				return XPU4_ERR_XC_IGNORED_MISMATCH;
			}
		}
	}

	rc = xpu4_xc_set_for_earlier_ee(xpu);
	if (rc != XPU4_OK) {
		return rc;
	}

	return xpu4_set_1_rg_prg(xpu);
}

enum xpu4_status xpu4_hal_configure_apnspe(const struct ac_xpu4_priv_info *xpu)
{
	xpu4_write_apnspe(xpu, true);

	if (!xpu4_read_apnspe(xpu)) {
		ERROR("xpu4: APNSPE enable failed soc=0x%x\n", xpu->soc_addr);
		return XPU4_ERR_APNSPE_ENABLE;
	}

	return XPU4_OK;
}

void xpu4_hal_restore_shadow_perm(const struct ac_xpu4_priv_info *xpu)
{
	uint32_t rg_start;
	uint32_t rg_end;
	uint32_t i;

	/* Only permissions for dynamic RGs are tracked. */
	if (xpu->dyn_rgs == NULL) {
		return;
	}

	rg_start = xpu->dyn_rgs->rg_start;
	rg_end = xpu->dyn_rgs->rg_start + xpu->dyn_rgs->rg_count;

	for (i = rg_start; i < rg_end; i++) {
		ac_xpu4_qad_vector hw_read;
		ac_xpu4_qad_vector hw_write;

		if (!xpu4_read_rg_enable(xpu, i)) {
			continue;
		}

		xpu4_read_rg_perm(xpu, i, &hw_read, &hw_write);
		xpu4_hal_cache_shadow_perm(xpu, i, hw_read, hw_write);
	}
}

enum xpu4_status
xpu4_hal_apply_static_config(const struct ac_xpu4_priv_info *xpu,
			     const struct ac_xpu_cfg *cfg)
{
	uint32_t rg_num = UINT32_MAX;	/* an invalid RG number */
	bool is_mpu;
	uint32_t i;

	is_mpu = xpu4_idr_xpu_type(xpu) == (uint32_t)AC_MPU;

	/*
	 * Walk the entries backwards, from nrg - 1 down to 0; the loop ends on
	 * unsigned underflow. Later entries take precedence for a given group,
	 * so visiting them first lets earlier duplicates be skipped.
	 */
	for (i = cfg->nrg - 1U; i < cfg->nrg; i--) {
		struct xpu4_rg_static_config rg_cfg;
		ac_xpu4_qad_vector lock_vec;
		bool wowp = false;
		enum xpu4_status rc;

		xpu4_get_rg_static_config(cfg, i, is_mpu, &rg_cfg);

		if (!xpu4_is_profile_enabled(rg_cfg.rg_profile_flags)) {
			continue;
		}

		if (rg_cfg.rg_num == rg_num) {
			continue;
		}
		rg_num = rg_cfg.rg_num;

		if (is_mpu) {
			rc = xpu4_hal_soc_addr_to_match_addr(xpu, &rg_cfg.start,
							     &rg_cfg.end);
			if (rc != XPU4_OK) {
				return rc;
			}
		}

		lock_vec = xpu4_calculate_lock_vector(xpu, rg_cfg.read_qads,
						      rg_cfg.write_qads);

		if (xpu4_read_rg_lock(xpu, rg_cfg.rg_num) != 0U) {
			rc = xpu4_fix_up_by_rg(xpu, rg_cfg.rg_num,
					       rg_cfg.start, rg_cfg.end, wowp,
					       rg_cfg.read_qads,
					       rg_cfg.write_qads, lock_vec);
			if (rc == XPU4_OK) {
				continue;
			}
			if (rc != XPU4_ERR_BEYOND_FIX_UP) {
				return rc;
			}

			/* Unlock so TZ can override the earlier stage. */
			rc = xpu4_unprotect_by_rg(xpu, rg_cfg.rg_num, true);
			if (rc != XPU4_OK) {
				return rc;
			}
		}

		rc = xpu4_protect_by_rg(xpu, rg_cfg.rg_num, rg_cfg.start,
					rg_cfg.end, wowp, rg_cfg.read_qads,
					rg_cfg.write_qads, lock_vec);
		if (rc != XPU4_OK) {
			return rc;
		}
	}

	return XPU4_OK;
}

enum xpu4_status
xpu4_hal_scan_mpu_region(const struct ac_xpu4_priv_info *xpu, uint64_t start,
			 uint64_t size, uint32_t flags,
			 struct xpu4_partition_overlap *overlap)
{
	uint64_t s = start;
	uint64_t e = start + size;
	uint32_t num_overlaps = 0U;
	enum xpu4_status rc;
	uint32_t rg_num;

	if (xpu4_idr_xpu_type(xpu) != (uint32_t)AC_MPU) {
		ERROR("xpu4: wrong XPU type soc=0x%x\n", xpu->soc_addr);
		return XPU4_ERR_WRONG_XPU_TYPE;
	}

	rc = xpu4_hal_soc_addr_to_match_addr(xpu, &s, &e);
	if (rc != XPU4_OK) {
		return rc;
	}

	if ((flags & (uint32_t)AC_XPU_FLAG_STATIC) != 0U) {
		rc = xpu4_locate_enabled_static_rg(xpu, s, e, &rg_num);
		if (rc == XPU4_OK) {
			overlap->has_exact_match = true;
			overlap->num_overlaps = 1U;
			overlap->rg_num = rg_num;
			return XPU4_OK;
		}
		if (rc == XPU4_ERR_RG_NOT_FOUND) {
			overlap->has_exact_match = false;
			overlap->num_overlaps = 0U;
			overlap->rg_num = UINT32_MAX;
			return XPU4_OK;
		}

		return rc;
	}

	rc = xpu4_locate_enabled_dyn_rg(xpu, s, e, &rg_num, &num_overlaps);
	if (rc == XPU4_OK) {
		overlap->has_exact_match = true;
		overlap->num_overlaps = num_overlaps;
		overlap->rg_num = rg_num;
		return XPU4_OK;
	}
	if (rc == XPU4_ERR_RG_NOT_FOUND) {
		overlap->has_exact_match = false;
		overlap->num_overlaps = num_overlaps;
		overlap->rg_num = UINT32_MAX;
		return XPU4_OK;
	}

	return rc;
}

enum xpu4_status
xpu4_hal_protect_dyn_region(const struct ac_xpu4_priv_info *xpu, uint64_t start,
			    uint64_t size, bool wowp,
			    ac_xpu4_qad_vector read_perm,
			    ac_xpu4_qad_vector write_perm,
			    ac_xpu4_qad_vector lock)
{
	uint64_t s = start;
	uint64_t e = start + size;
	enum xpu4_status rc;

	if (xpu4_idr_xpu_type(xpu) != (uint32_t)AC_MPU) {
		ERROR("xpu4: wrong XPU type soc=0x%x\n", xpu->soc_addr);
		return XPU4_ERR_WRONG_XPU_TYPE;
	}

	rc = xpu4_hal_soc_addr_to_match_addr(xpu, &s, &e);
	if (rc != XPU4_OK) {
		return rc;
	}

	return xpu4_protect_dyn_mpu_region(xpu, s, e, wowp, read_perm,
					   write_perm, lock);
}

enum xpu4_status
xpu4_hal_update_dyn_region(const struct ac_xpu4_priv_info *xpu,
			   uint32_t rg_num, uint64_t start, uint64_t size,
			   bool wowp, ac_xpu4_qad_vector read_perm,
			   ac_xpu4_qad_vector write_perm,
			   ac_xpu4_qad_vector lock)
{
	uint64_t s = start;
	uint64_t e = start + size;
	uint64_t hw_start;
	uint64_t hw_end;
	enum xpu4_status rc;

	if (xpu4_idr_xpu_type(xpu) != (uint32_t)AC_MPU) {
		ERROR("xpu4: wrong XPU type soc=0x%x\n", xpu->soc_addr);
		return XPU4_ERR_WRONG_XPU_TYPE;
	}

	rc = xpu4_hal_soc_addr_to_match_addr(xpu, &s, &e);
	if (rc != XPU4_OK) {
		return rc;
	}

	xpu4_read_rg_addresses(xpu, rg_num, &hw_start, &hw_end);
	if (hw_start != s || hw_end != e) {
		ERROR("xpu4: wrong RG soc=0x%x rg=%u\n", xpu->soc_addr, rg_num);
		return XPU4_ERR_WRONG_RG;
	}

	return xpu4_update_dyn_mpu_region(xpu, rg_num, s, e, wowp, read_perm,
					  write_perm, lock);
}

enum xpu4_status
xpu4_hal_unprotect_dyn_region(const struct ac_xpu4_priv_info *xpu,
			      uint32_t rg_num, uint64_t start, uint64_t size)
{
	uint64_t s = start;
	uint64_t e = start + size;
	uint64_t hw_start;
	uint64_t hw_end;
	enum xpu4_status rc;

	if (xpu4_idr_xpu_type(xpu) != (uint32_t)AC_MPU) {
		ERROR("xpu4: wrong XPU type soc=0x%x\n", xpu->soc_addr);
		return XPU4_ERR_WRONG_XPU_TYPE;
	}

	rc = xpu4_hal_soc_addr_to_match_addr(xpu, &s, &e);
	if (rc != XPU4_OK) {
		return rc;
	}

	xpu4_read_rg_addresses(xpu, rg_num, &hw_start, &hw_end);
	if (hw_start != s || hw_end != e) {
		ERROR("xpu4: wrong RG soc=0x%x rg=%u\n", xpu->soc_addr, rg_num);
		return XPU4_ERR_WRONG_RG;
	}

	return xpu4_unprotect_dyn_mpu_region(xpu, rg_num);
}

enum xpu4_status
xpu4_hal_update_static_region(const struct ac_xpu4_priv_info *xpu,
			      uint64_t start, uint64_t size, bool wowp,
			      ac_xpu4_qad_vector read_perm,
			      ac_xpu4_qad_vector write_perm,
			      ac_xpu4_qad_vector lock)
{
	uint32_t xpu_type = xpu4_idr_xpu_type(xpu);
	enum xpu4_status rc;
	uint32_t rg_num;

	if (xpu_type == (uint32_t)AC_APU) {
		rc = xpu4_hal_get_apu_rg_from_range(xpu, start, size, &rg_num);
		if (rc != XPU4_OK) {
			return rc;
		}

		return xpu4_update_static_apu_rpu_region(xpu, rg_num, wowp,
							read_perm, write_perm,
							lock);
	}

	if (xpu_type == (uint32_t)AC_RPU) {
		rc = xpu4_hal_get_rpu_rg_from_range(xpu, start, size, &rg_num);
		if (rc != XPU4_OK) {
			return rc;
		}

		return xpu4_update_static_apu_rpu_region(xpu, rg_num, wowp,
							read_perm, write_perm,
							lock);
	}

	if (xpu_type == (uint32_t)AC_MPU) {
		uint64_t s = start;
		uint64_t e = start + size;

		rc = xpu4_hal_soc_addr_to_match_addr(xpu, &s, &e);
		if (rc != XPU4_OK) {
			return rc;
		}

		return xpu4_update_static_mpu_region(xpu, s, e, wowp, read_perm,
						     write_perm, lock);
	}

	ERROR("xpu4: wrong XPU type soc=0x%x\n", xpu->soc_addr);

	return XPU4_ERR_WRONG_XPU_TYPE;
}

enum xpu4_status xpu4_hal_fill_rg_info(const struct ac_xpu4_priv_info *xpu,
				       uint32_t rg_num,
				       struct xpu4_rg_info *info)
{
	memset(info, 0, sizeof(*info));

	if (xpu4_idr_xpu_type(xpu) == (uint32_t)AC_MPU) {
		xpu4_read_rg_addresses(xpu, rg_num, &info->start_addr,
				       &info->end_addr);
	} else {
		info->start_addr = UINT64_MAX;
		info->end_addr = UINT64_MAX;
	}

	xpu4_read_rg_perm(xpu, rg_num, &info->read_perm_vector,
			  &info->write_perm_vector);

	info->lock_vector = xpu4_read_rg_lock(xpu, rg_num);
	info->rge = xpu4_read_rg_enable(xpu, rg_num);
	info->wowp = xpu4_read_rg_write_protected(xpu, rg_num);

	xpu4_hal_get_shadow_perm(xpu, rg_num, &info->shadow_read_perm_vector,
				 &info->shadow_write_perm_vector);

	return XPU4_OK;
}

void xpu4_hal_set_silent_log_buffer(void *buffer, size_t size)
{
	silent_log = (struct xpu4_violation *)buffer;
	silent_log_capacity = (uint32_t)(size / sizeof(struct xpu4_violation));
	silent_log_count = 0U;
	distinct_violations = 0U;
}

void xpu4_hal_append_silent_log(const struct ac_xpu4_priv_info *xpu)
{
	struct xpu4_violation curr;
	uint32_t lsb;
	uint32_t i;

	xpu4_read_syndrome(xpu, &curr.syn);
	curr.xpu = xpu->soc_addr;
	curr.vio_count = 1U;

	/*
	 * Two violations are treated as the same if they come from the same XPU
	 * and the faulting addresses differ only below the instance's address
	 * granule, in which case only the repeat counter is bumped.
	 */
	lsb = xpu4_idr_addr_lsb(xpu);
	for (i = 0U; i < silent_log_count; i++) {
		struct xpu4_violation *prev = &silent_log[i];

		if (prev->xpu == curr.xpu &&
		    prev->syn.synar1 == curr.syn.synar1 &&
		    ((prev->syn.synar0 ^ curr.syn.synar0) >> lsb) == 0U) {
			UINT32_INC_SATURATE(prev->vio_count);
			return;
		}
	}

	UINT32_INC_SATURATE(distinct_violations);

	if (silent_log == NULL || silent_log_count >= silent_log_capacity) {
		return;
	}

	memcpy(&silent_log[silent_log_count], &curr, sizeof(curr));
	silent_log_count++;
}

void xpu4_hal_log_violation(const struct ac_xpu4_priv_info *xpu)
{
	struct xpu4_error_syndrome syndrome;

	xpu4_read_syndrome(xpu, &syndrome);

	ERROR("XPU4 violation soc=0x%x esr=0x%x\n", xpu->soc_addr,
	      syndrome.esr);
	ERROR("  synar0=0x%x synar1=0x%x\n", syndrome.synar0, syndrome.synar1);
	ERROR("  synr0=0x%x synr1=0x%x synr2=0x%x\n", syndrome.synr0,
	      syndrome.synr1, syndrome.synr2);
}

void xpu4_hal_clear_violation(const struct ac_xpu4_priv_info *xpu)
{
	xpu4_write(xpu, XPU4_ESR_OFFSET, 0U);
}

/*
 * Instance lookup, address translation and the shadow permission cache.
 */

/*
 * Look up the bus-level address offset and mask for an XPU instance.
 *
 * Some XPUs sit behind a bus that presents a shifted or narrowed view of the
 * address space, so a SOC address has to be adjusted before the XPU will match
 * it. Instances with no entry need no adjustment.
 */
static bool xpu4_get_addr_offset_mask(enum xpu4_id xpu_id, uint64_t *offset,
				      uint64_t *mask)
{
	const struct ac_xpu_hwaddr_mask *entry = NULL;
	uint32_t count = 0U;
	uint32_t i;

	if (ac_cfg_get_addr_offset_mask_info(&entry, &count) != 0) {
		return false;
	}

	if (entry == NULL) {
		return false;
	}

	for (i = 0U; i < count; i++) {
		if ((uint32_t)xpu_id != entry[i].xpu_index) {
			continue;
		}

		*offset = entry[i].base_addr;
		*mask = entry[i].mask_value;

		return true;
	}

	return false;
}

struct ac_xpu4_priv_info *xpu4_hal_get_xpu_info(enum xpu4_id xpu_id)
{
	struct ac_xpu4_priv_info *infos = NULL;
	uint32_t count = 0U;
	uint32_t i;

	if (ac_cfg_get_xpu_infos(&infos, &count) != 0) {
		return NULL;
	}

	if (infos == NULL) {
		return NULL;
	}

	for (i = 0U; i < count; i++) {
		if ((uint32_t)xpu_id == infos[i].xpu_id) {
			return &infos[i];
		}
	}

	return NULL;
}

enum xpu4_status
xpu4_hal_soc_addr_to_match_addr(const struct ac_xpu4_priv_info *xpu,
				uint64_t *addr1, uint64_t *addr2)
{
	uint64_t a1 = *addr1;
	uint64_t a2 = *addr2;
	uint32_t extra_high_bits;
	uint32_t extra_low_bits;
	uint64_t offset;
	uint64_t mask;

	/* Bus level offset and masking. */
	if (xpu4_get_addr_offset_mask((enum xpu4_id)xpu->xpu_id, &offset,
				      &mask)) {
		if (a1 < offset || a2 < offset) {
			ERROR("xpu4: bus offset underflow soc=0x%x\n",
			      xpu->soc_addr);
			return XPU4_ERR_BUS_OFFSET_UNDERFLOW;
		}

		a1 = (a1 - offset) & mask;
		a2 = (a2 - offset) & mask;
	}

	/* XPU level masking: drop the bits the instance does not compare. */
	extra_high_bits = 63U - xpu4_idr_addr_msb(xpu);
	extra_low_bits = xpu4_idr_addr_lsb(xpu);
	mask = UINT64_MAX >> (extra_high_bits + extra_low_bits)
			  << extra_low_bits;

	*addr1 = a1 & mask;
	*addr2 = a2 & mask;

	return XPU4_OK;
}

void xpu4_hal_get_shadow_perm(const struct ac_xpu4_priv_info *xpu,
			      uint32_t rg_num,
			      ac_xpu4_qad_vector *read_perm,
			      ac_xpu4_qad_vector *write_perm)
{
	ac_xpu4_qad_vector shadow_read = 0U;
	ac_xpu4_qad_vector shadow_write = 0U;

	/* Only permissions for dynamic RGs are tracked. */
	if (xpu->dyn_rgs != NULL) {
		if (rg_num >= xpu->dyn_rgs->rg_start &&
		    rg_num < (xpu->dyn_rgs->rg_start +
			      xpu->dyn_rgs->rg_count)) {
			rg_num -= xpu->dyn_rgs->rg_start;

			if (xpu->dyn_rgs->shadow_read_perm != NULL) {
				shadow_read =
					xpu->dyn_rgs->shadow_read_perm[rg_num];
			}

			if (xpu->dyn_rgs->shadow_write_perm != NULL) {
				shadow_write =
					xpu->dyn_rgs->shadow_write_perm[rg_num];
			}
		}
	}

	*read_perm = shadow_read;
	*write_perm = shadow_write;
}

void xpu4_hal_cache_shadow_perm(const struct ac_xpu4_priv_info *xpu,
				uint32_t rg_num,
				ac_xpu4_qad_vector read_perm,
				ac_xpu4_qad_vector write_perm)
{
	/* Only permissions for dynamic RGs are tracked. */
	if (xpu->dyn_rgs == NULL) {
		return;
	}

	if (rg_num < xpu->dyn_rgs->rg_start ||
	    rg_num >= (xpu->dyn_rgs->rg_start + xpu->dyn_rgs->rg_count)) {
		return;
	}

	rg_num -= xpu->dyn_rgs->rg_start;

	if (xpu->dyn_rgs->shadow_read_perm != NULL) {
		xpu->dyn_rgs->shadow_read_perm[rg_num] = read_perm;
	}

	if (xpu->dyn_rgs->shadow_write_perm != NULL) {
		xpu->dyn_rgs->shadow_write_perm[rg_num] = write_perm;
	}
}

enum xpu4_status
xpu4_hal_get_apu_rg_from_range(const struct ac_xpu4_priv_info *xpu,
			       uint64_t start, uint64_t size, uint32_t *rg_num)
{
	uintptr_t *group = NULL;
	uint32_t count = 0U;
	uint32_t i;

	if (xpu4_idr_xpu_type(xpu) != (uint32_t)AC_APU) {
		ERROR("xpu4: wrong XPU type soc=0x%x\n", xpu->soc_addr);
		return XPU4_ERR_WRONG_XPU_TYPE;
	}

	if (ac_cfg_get_apu_rpu_partitions_info(&group, &count) != 0) {
		return XPU4_ERR_APU_INFO_NOT_FOUND;
	}

	if (group == NULL) {
		return XPU4_ERR_NULL_POINTER;
	}

	for (i = 0U; i < count; i++) {
		const struct ac_xpu_info *apu;
		uint32_t k;

		apu = (const struct ac_xpu_info *)group[i];
		if (xpu->xpu_id != (uint32_t)apu->xpu_id) {
			continue;
		}

		if (apu->xpu_id_type != XPU4_DEVICE_APU) {
			ERROR("xpu4: inconsistent info soc=0x%x type=%u\n",
			      xpu->soc_addr, (uint32_t)apu->xpu_id_type);
			return XPU4_ERR_INCONSISTENT_XPU_INFO;
		}

		for (k = 0U; k < apu->num_partitions; k++) {
			const struct ac_partition *p = &apu->partition_info[k];

			if (start < p->start || (start + size) > p->end) {
				continue;
			}

			/* Usable RG count is IDR.NRG + 1. */
			if (p->rg_num > xpu4_idr_nrg(xpu)) {
				ERROR("xpu4: RG %u exceeds NRG %u soc=0x%x\n",
				      p->rg_num, xpu4_idr_nrg(xpu),
				      xpu->soc_addr);
				return XPU4_ERR_INCONSISTENT_XPU_INFO;
			}

			*rg_num = p->rg_num;

			return XPU4_OK;
		}

		ERROR("xpu4: no APU RG for range soc=0x%x\n", xpu->soc_addr);

		return XPU4_ERR_APU_RG_INFO_NOT_FOUND;
	}

	ERROR("xpu4: no APU info soc=0x%x\n", xpu->soc_addr);

	return XPU4_ERR_APU_INFO_NOT_FOUND;
}

enum xpu4_status
xpu4_hal_get_rpu_rg_from_range(const struct ac_xpu4_priv_info *xpu,
			       uint64_t start, uint64_t size, uint32_t *rg_num)
{
	uintptr_t *group = NULL;
	uint32_t count = 0U;
	uint32_t rg;
	uint32_t i;

	if (ac_cfg_get_apu_rpu_partitions_info(&group, &count) != 0) {
		return XPU4_ERR_RPU_INFO_NOT_FOUND;
	}

	if (group == NULL) {
		return XPU4_ERR_NULL_POINTER;
	}

	if (xpu4_idr_xpu_type(xpu) != (uint32_t)AC_RPU) {
		ERROR("xpu4: wrong XPU type soc=0x%x\n", xpu->soc_addr);
		return XPU4_ERR_WRONG_XPU_TYPE;
	}

	for (i = 0U; i < count; i++) {
		const struct ac_xpu_info *rpu;
		uint64_t ptn_size;

		rpu = (const struct ac_xpu_info *)group[i];
		if (xpu->xpu_id != (uint32_t)rpu->xpu_id) {
			continue;
		}

		if (rpu->xpu_id_type != XPU4_DEVICE_RPU) {
			ERROR("xpu4: inconsistent info soc=0x%x type=%u\n",
			      xpu->soc_addr, (uint32_t)rpu->xpu_id_type);
			return XPU4_ERR_INCONSISTENT_XPU_INFO;
		}

		/*
		 * An RPU divides its range into fixed-size partitions, one per
		 * resource group, so the request has to match that granule.
		 */
		ptn_size = (uint64_t)1U << xpu4_idr_addr_lsb(xpu);
		if ((start & (ptn_size - 1U)) != 0U || size != ptn_size) {
			return XPU4_ERR_RPU_ADDR_SIZE_NOT_ALIGNED;
		}

		if (start < rpu->range_start ||
		    (start + size) > rpu->range_end) {
			ERROR("xpu4: range outside RPU soc=0x%x\n",
			      xpu->soc_addr);
			return XPU4_ERR_INCONSISTENT_XPU_INFO;
		}

		rg = (uint32_t)((start - rpu->range_start) / ptn_size);

		/* Usable RG count is IDR.NRG + 1. */
		if (rg > xpu4_idr_nrg(xpu)) {
			ERROR("xpu4: RG %u exceeds NRG %u soc=0x%x\n", rg,
			      xpu4_idr_nrg(xpu), xpu->soc_addr);
			return XPU4_ERR_INCONSISTENT_XPU_INFO;
		}

		*rg_num = rg;

		return XPU4_OK;
	}

	ERROR("xpu4: no RPU info soc=0x%x\n", xpu->soc_addr);

	return XPU4_ERR_RPU_INFO_NOT_FOUND;
}
