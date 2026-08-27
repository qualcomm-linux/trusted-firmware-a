/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XPU4_HAL_H
#define XPU4_HAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <xpu4.h>
#include <xpu4_hwio.h>

/* Extract a field from a previously read register value. */
#define XPU4_FIELD_GET(reg, field, val) \
	(((val) & XPU4_##reg##_##field##_BMSK) >> XPU4_##reg##_##field##_SHFT)

/* Compose a revision value for comparison against the REV register. */
#define XPU4_REV(major, minor, step)					\
	((((major) << XPU4_REV_MAJOR_SHFT) & XPU4_REV_MAJOR_BMSK) |	\
	 (((minor) << XPU4_REV_MINOR_SHFT) & XPU4_REV_MINOR_BMSK) |	\
	 (((step) << XPU4_REV_STEP_SHFT) & XPU4_REV_STEP_BMSK))

/*
 * Number of distinct violations the silent log can hold. Silent logging records
 * a violation and lets execution continue instead of panicking, so this bounds
 * how many distinct ones are kept; repeats of an already-recorded violation
 * only bump its counter. Override per platform if a larger history is wanted.
 */
#ifndef XPU4_SILENT_LOG_ENTRIES
#define XPU4_SILENT_LOG_ENTRIES		16U
#endif

/*
 * HAL status codes.
 *
 * These are not errno values on purpose: the region-programming paths branch on
 * specific codes, most importantly XPU4_ERR_BEYOND_FIX_UP (fall back to
 * reallocating a resource group) and XPU4_ERR_RG_NOT_FOUND (no match), so the
 * distinctions have to survive.
 */
enum xpu4_status {
	XPU4_OK = 0,
	XPU4_ERR_NULL_POINTER,
	XPU4_ERR_WRONG_XPU_TYPE,
	XPU4_ERR_WRONG_RG,
	XPU4_ERR_RG_NOT_FOUND,
	XPU4_ERR_STATIC_MPU_RG_NOT_FOUND,
	XPU4_ERR_NO_FREE_RG,
	XPU4_ERR_NO_DYNAMIC_RG_INFO,
	XPU4_ERR_BEYOND_FIX_UP,
	XPU4_ERR_RG_NOT_ENABLED,
	XPU4_ERR_RG_STILL_LOCKED,
	XPU4_ERR_RG_WRITE_MISMATCH,
	XPU4_ERR_RG_PERM_MISMATCH,
	XPU4_ERR_RG_ADDR_MISMATCH,
	XPU4_ERR_RG_WOWP_MISMATCH,
	XPU4_ERR_XC_STATUS_NOT_SET,
	XPU4_ERR_XC1_TRIGGER_MISMATCH,
	XPU4_ERR_XC_IGNORED_MISMATCH,
	XPU4_ERR_UMRPERM_MISMATCH,
	XPU4_ERR_CFGOWNER_MISMATCH,
	XPU4_ERR_APNSPE_ENABLE,
	XPU4_ERR_BUS_OFFSET_UNDERFLOW,
	XPU4_ERR_INCONSISTENT_XPU_INFO,
	XPU4_ERR_APU_INFO_NOT_FOUND,
	XPU4_ERR_APU_RG_INFO_NOT_FOUND,
	XPU4_ERR_RPU_INFO_NOT_FOUND,
	XPU4_ERR_RPU_ADDR_SIZE_NOT_ALIGNED
};

/* One recorded violation in the silent log. */
struct xpu4_violation {
	struct xpu4_error_syndrome syn;
	uint32_t xpu;
	uint32_t vio_count;
};

/*
 * Identity decoders. The IDR registers are read once by xpu4_hal_init_xpu()
 * and cached in struct ac_xpu4_priv_info, so these are pure accessors.
 */

static inline uint32_t
xpu4_idr_xpresscfg_en(const struct ac_xpu4_priv_info *xpu)
{
	return XPU4_FIELD_GET(IDR0, XPRESSCFGEN, xpu->idr[0]);
}

/* Returns an enum xpu4_variant value. */
static inline uint32_t xpu4_idr_xpu_type(const struct ac_xpu4_priv_info *xpu)
{
	return XPU4_FIELD_GET(IDR0, XPU_TYPE, xpu->idr[0]);
}

/* NOTE: the usable resource group count is this value plus one. */
static inline uint32_t xpu4_idr_nrg(const struct ac_xpu4_priv_info *xpu)
{
	return XPU4_FIELD_GET(IDR0, NRG, xpu->idr[0]);
}

/* NOTE: the usable client address width is this value plus one. */
static inline uint32_t
xpu4_idr_client_addr_width(const struct ac_xpu4_priv_info *xpu)
{
	return XPU4_FIELD_GET(IDR1, CLIENT_ADDR_WIDTH, xpu->idr[1]);
}

static inline uint32_t xpu4_idr_addr_msb(const struct ac_xpu4_priv_info *xpu)
{
	return XPU4_FIELD_GET(IDR1, ADDR_MSB, xpu->idr[1]);
}

static inline uint32_t xpu4_idr_addr_lsb(const struct ac_xpu4_priv_info *xpu)
{
	return XPU4_FIELD_GET(IDR1, ADDR_LSB, xpu->idr[1]);
}

/* Bitmap of every QAD the driver considers valid on this configuration. */
ac_xpu4_qad_vector xpu4_hal_get_valid_qad_vector(void);

/*
 * Instance lookup, identity and address translation.
 */
struct ac_xpu4_priv_info *xpu4_hal_get_xpu_info(enum xpu4_id xpu_id);
enum xpu4_status xpu4_hal_init_xpu(struct ac_xpu4_priv_info *xpu);

/*
 * Translate a pair of SOC-view addresses into the client-view addresses the
 * XPU compares against, applying the bus offset and mask from the config image
 * followed by the instance's own address alignment.
 */
enum xpu4_status
xpu4_hal_soc_addr_to_match_addr(const struct ac_xpu4_priv_info *xpu,
				uint64_t *addr1, uint64_t *addr2);

/* Resolve the resource group covering a range on an APU or an RPU. */
enum xpu4_status
xpu4_hal_get_apu_rg_from_range(const struct ac_xpu4_priv_info *xpu,
			       uint64_t start, uint64_t size, uint32_t *rg_num);
enum xpu4_status
xpu4_hal_get_rpu_rg_from_range(const struct ac_xpu4_priv_info *xpu,
			       uint64_t start, uint64_t size, uint32_t *rg_num);

/*
 * Shadow permission cache. Only resource groups reserved for runtime
 * assignment are tracked, because only those can be reprogrammed.
 */
void xpu4_hal_get_shadow_perm(const struct ac_xpu4_priv_info *xpu,
			      uint32_t rg_num,
			      ac_xpu4_qad_vector *read_perm,
			      ac_xpu4_qad_vector *write_perm);
void xpu4_hal_cache_shadow_perm(const struct ac_xpu4_priv_info *xpu,
				uint32_t rg_num,
				ac_xpu4_qad_vector read_perm,
				ac_xpu4_qad_vector write_perm);
void xpu4_hal_restore_shadow_perm(const struct ac_xpu4_priv_info *xpu);

/*
 * Static configuration.
 */
enum xpu4_status
xpu4_hal_apply_static_config(const struct ac_xpu4_priv_info *xpu,
			     const struct ac_xpu_cfg *cfg);
enum xpu4_status
xpu4_hal_apply_xpress_config(const struct ac_xpu4_priv_info *xpu,
			     const struct ac_xpu_cfg *cfg);
enum xpu4_status xpu4_hal_set_umr_perms(const struct ac_xpu4_priv_info *xpu,
					const struct ac_xpu_cfg *cfg);
enum xpu4_status xpu4_hal_set_cfg_owner(const struct ac_xpu4_priv_info *xpu,
					const struct ac_xpu_cfg *cfg);
ac_xpu4_qad_vector
xpu4_hal_get_cfg_owner(const struct ac_xpu4_priv_info *xpu);
enum xpu4_status
xpu4_hal_configure_apnspe(const struct ac_xpu4_priv_info *xpu);

/*
 * Runtime region programming. Address ranges are given as SOC-view start plus
 * size and translated internally.
 */
enum xpu4_status
xpu4_hal_scan_mpu_region(const struct ac_xpu4_priv_info *xpu, uint64_t start,
			 uint64_t size, uint32_t flags,
			 struct xpu4_partition_overlap *overlap);
enum xpu4_status
xpu4_hal_protect_dyn_region(const struct ac_xpu4_priv_info *xpu,
			    uint64_t start, uint64_t size, bool wowp,
			    ac_xpu4_qad_vector read_perm,
			    ac_xpu4_qad_vector write_perm,
			    ac_xpu4_qad_vector lock);
enum xpu4_status
xpu4_hal_update_dyn_region(const struct ac_xpu4_priv_info *xpu,
			   uint32_t rg_num, uint64_t start, uint64_t size,
			   bool wowp, ac_xpu4_qad_vector read_perm,
			   ac_xpu4_qad_vector write_perm,
			   ac_xpu4_qad_vector lock);
enum xpu4_status
xpu4_hal_unprotect_dyn_region(const struct ac_xpu4_priv_info *xpu,
			      uint32_t rg_num, uint64_t start, uint64_t size);
enum xpu4_status
xpu4_hal_update_static_region(const struct ac_xpu4_priv_info *xpu,
			      uint64_t start, uint64_t size, bool wowp,
			      ac_xpu4_qad_vector read_perm,
			      ac_xpu4_qad_vector write_perm,
			      ac_xpu4_qad_vector lock);
enum xpu4_status xpu4_hal_fill_rg_info(const struct ac_xpu4_priv_info *xpu,
				       uint32_t rg_num,
				       struct xpu4_rg_info *info);

/*
 * Violation reporting. With silent logging enabled a violation is appended to
 * the log and cleared so execution continues; otherwise the caller panics.
 */
void xpu4_hal_set_silent_log_buffer(void *buffer, size_t size);
void xpu4_hal_append_silent_log(const struct ac_xpu4_priv_info *xpu);
void xpu4_hal_log_violation(const struct ac_xpu4_priv_info *xpu);
void xpu4_hal_clear_violation(const struct ac_xpu4_priv_info *xpu);

/*
 * Provided by xpu4.c: the QAD vector of the executing environment, whether an
 * instance is an RA (resource-access) XPU, and whether a policy profile is
 * enabled for this device.
 */
ac_xpu4_qad_vector xpu4_get_apps_qad_vector(void);
bool xpu4_is_ra_xpu(uint32_t xpu_id);
bool xpu4_is_profile_enabled(ac_xpu_profile profile);

#endif /* XPU4_HAL_H */
