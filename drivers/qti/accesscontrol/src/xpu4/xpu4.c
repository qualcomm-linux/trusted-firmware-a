/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ac_cfg.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/utils_def.h>
#include <xpu4.h>
#include <xpu4_hal.h>
#include <xpu_common.h>

/* Value of ac_xpu_cfg::status that marks an entry as enabled. */
#define TZBSP_XPU_ENABLE	0x01U

/* Pairing of a QAD vector with the permission-domain mask it corresponds to. */
struct qad_domain_mask_pair {
	const ac_xpu4_qad_vector q;
	const uint32_t d;
};

/*
 * The XPU that protects DDR. Discovered once from the config image by matching
 * against the known DDR/BIMC/LLCC instances.
 */
static enum xpu4_id ddr_xpu_id;
static bool ddr_xpu_id_valid;

/*
 * Policy profiles enabled for this device.
 *
 * Downstream derives this from the silicon revision, debug-policy and fuse
 * state via ChipInfo and dbginfolib, none of which exist upstream. Only the
 * default profile is enabled here, so configuration entries tagged for
 * debug, ramdump or platform-specific profiles are skipped.
 */
static ac_xpu_profile supported_profiles = AC_PROFILE_DEFAULT;

/*
 * Permission translation.
 */

ac_xpu4_qad_vector xpu4_domain_mask_to_qad_vector(uint32_t domain_mask)
{
	static const struct qad_domain_mask_pair domain2qad[] = {
		{ .d = TZBSP_DOMAIN_TZ_BIT, .q = QAD_VECTOR_AP_SEC },
		{ .d = TZBSP_DOMAIN_HYP_BIT, .q = QAD_VECTOR_AP_NS },
		{ .d = TZBSP_VMID_VMID_0_BIT, .q = QAD_VECTOR_AP_NS },
		{ .d = TZBSP_DOMAIN_TME_ROM_BIT, .q = QAD_VECTOR_TME_ROM },
		{ .d = TZBSP_DOMAIN_TME_FW_BIT, .q = QAD_VECTOR_TME_FW },
		{ .d = TZBSP_DOMAIN_DEBUG_BIT, .q = QAD_VECTOR_DEBUG },
		{ .d = TZBSP_DOMAIN_AOP_BIT, .q = QAD_VECTOR_AOP },
		{ .d = TZBSP_DOMAIN_MSA_BIT, .q = QAD_VECTOR_MODEM },
		{ .d = TZBSP_DOMAIN_SP_BIT, .q = QAD_VECTOR_SP },
#if AC_VM_BITMAP_SIZE == 128
		{ .d = TZBSP_DOMAIN_QECP_DEBUG_BIT,
		  .q = QAD_VECTOR_QECP_DEBUG },
		{ .d = TZBSP_DOMAIN_OOBNS_BIT, .q = QAD_VECTOR_OOBNS },
		{ .d = TZBSP_DOMAIN_OOBS_BIT, .q = QAD_VECTOR_OOBS },
#endif
	};

	ac_xpu4_qad_vector qad_vector = 0U;
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(domain2qad); i++) {
		if ((domain_mask & domain2qad[i].d) == domain2qad[i].d) {
			qad_vector |= domain2qad[i].q;
		}
	}

	return qad_vector;
}

uint32_t xpu4_qad_vector_to_domain_mask(ac_xpu4_qad_vector qad_vector)
{
	static const struct qad_domain_mask_pair qad2domain[] = {
		{ .q = QAD_VECTOR_AP_SEC, .d = TZBSP_DOMAIN_TZ_BIT },
		{ .q = QAD_VECTOR_AP_NS, .d = TZBSP_VMID_VMID_0_BIT },
		{ .q = QAD_VECTOR_TME_ROM, .d = TZBSP_DOMAIN_TME_ROM_BIT },
		{ .q = QAD_VECTOR_TME_FW, .d = TZBSP_DOMAIN_TME_FW_BIT },
		{ .q = QAD_VECTOR_AOP, .d = TZBSP_DOMAIN_AOP_BIT },
		{ .q = QAD_VECTOR_MODEM, .d = TZBSP_DOMAIN_MSA_BIT },
		{ .q = QAD_VECTOR_SP, .d = TZBSP_DOMAIN_SP_BIT },
#if AC_VM_BITMAP_SIZE == 128
		{ .q = QAD_VECTOR_QECP_DEBUG,
		  .d = TZBSP_DOMAIN_QECP_DEBUG_BIT },
		{ .q = QAD_VECTOR_OOBNS, .d = TZBSP_DOMAIN_OOBNS_BIT },
		{ .q = QAD_VECTOR_OOBS, .d = TZBSP_DOMAIN_OOBS_BIT },
#endif
	};

	uint32_t domain_mask = 0U;
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(qad2domain); i++) {
		if ((qad_vector & qad2domain[i].q) == qad2domain[i].q) {
			domain_mask |= qad2domain[i].d;
		}
	}

	return domain_mask;
}

ac_xpu4_qad_vector xpu4_get_apps_qad_vector(void)
{
	return QAD_VECTOR_AP_SEC | QAD_VECTOR_AP_NS;
}

/*
 * A permission vector naming APPS is only meaningful if it names secure,
 * non-secure, or both. Anything else is a malformed request.
 */
static bool xpu4_is_valid_apps_qad_vector(ac_xpu4_qad_vector vector)
{
	if (vector == (QAD_VECTOR_AP_SEC | QAD_VECTOR_AP_NS) ||
	    vector == QAD_VECTOR_AP_SEC ||
	    vector == QAD_VECTOR_AP_NS) {
		return true;
	}

	return false;
}

bool xpu4_is_profile_enabled(ac_xpu_profile profile)
{
	return (profile & supported_profiles) != 0U;
}

/*
 * Instance classification and discovery.
 */

bool xpu4_is_ra_xpu(uint32_t xpu_id)
{
	const struct ac_ra_xpu_info *info = NULL;
	uint32_t count = 0U;
	uint32_t i;

	/*
	 * A lookup failure means the RA list is unavailable, which is not the
	 * same as the instance being an RA XPU. Downstream returned a non-zero
	 * error code from this bool function here, which read as "true" and
	 * pushed callers onto the RA path on any failure.
	 */
	if (ac_cfg_get_ra_xpu_info(&info, &count) != 0) {
		return false;
	}

	if (info == NULL) {
		return false;
	}

	for (i = 0U; i < count; i++) {
		if (xpu_id == info[i].xpu_id) {
			return true;
		}
	}

	return false;
}

/*
 * Identify the XPU that protects DDR by matching the configured instances
 * against the known DDR/BIMC/LLCC ids. The first enabled match wins.
 */
static void xpu4_find_ddr_xpu(const struct ac_xpu_cfg *cfg, uint32_t count)
{
	static const enum xpu4_id ddr_ids[] = {
		XPU4_ID_LLCC_AND_BROADCAST_MPU,
		XPU4_ID_LLCC_OR_BROADCAST_MPU,
		XPU4_ID_LLCC_BROADCAST_MPU,
		XPU4_ID_BIMC_MPU0,
		XPU4_ID_BIMC_AND_GLB_MPU,
		XPU4_ID_MACHX_AND_BCAST_LLCC_MPU
	};

	uint32_t i;
	size_t k;

	for (i = 0U; i < count; i++) {
		if (cfg[i].status != TZBSP_XPU_ENABLE) {
			continue;
		}

		if (!xpu4_is_profile_enabled(cfg[i].profile_flags)) {
			continue;
		}

		for (k = 0U; k < ARRAY_SIZE(ddr_ids); k++) {
			if ((uint32_t)ddr_ids[k] != cfg[i].xpu_id) {
				continue;
			}

			ddr_xpu_id = (enum xpu4_id)cfg[i].xpu_id;
			ddr_xpu_id_valid = true;

			return;
		}
	}
}

static enum xpu4_status xpu4_get_ddr_xpu(enum xpu4_id *xpu_id)
{
	if (!ddr_xpu_id_valid) {
		ERROR("xpu4: no DDR XPU in config image\n");
		return XPU4_ERR_APU_INFO_NOT_FOUND;
	}

	*xpu_id = ddr_xpu_id;

	return XPU4_OK;
}

/*
 * Request validation.
 */

static enum xpu4_status xpu4_validate_address_region(uint64_t start,
						     uint64_t size)
{
	if (size == 0U) {
		ERROR("xpu4: region size is zero\n");
		return XPU4_ERR_WRONG_RG;
	}

	if (start > (UINT64_MAX - size)) {
		ERROR("xpu4: region overflows start=0x%llx size=0x%llx\n",
		      (unsigned long long)start, (unsigned long long)size);
		return XPU4_ERR_WRONG_RG;
	}

	return XPU4_OK;
}

static enum xpu4_status
xpu4_validate_address_alignment(const struct ac_xpu4_priv_info *xpu,
				uint64_t start, uint64_t size)
{
	uint64_t mask = ((uint64_t)1U << xpu4_idr_addr_lsb(xpu)) - 1U;

	if ((start & mask) != 0U) {
		ERROR("xpu4: start 0x%llx not aligned to 0x%llx\n",
		      (unsigned long long)start, (unsigned long long)mask + 1U);
		return XPU4_ERR_RPU_ADDR_SIZE_NOT_ALIGNED;
	}

	if ((size & mask) != 0U) {
		ERROR("xpu4: size 0x%llx not aligned to 0x%llx\n",
		      (unsigned long long)size, (unsigned long long)mask + 1U);
		return XPU4_ERR_RPU_ADDR_SIZE_NOT_ALIGNED;
	}

	return XPU4_OK;
}

/*
 * Reject permission and lock vectors that name an unsupported QAD, that name
 * APPS in a malformed way, or that lock a QAD without also granting it a
 * permission (which would close the group for no benefit).
 */
static enum xpu4_status xpu4_validate_perm(ac_xpu4_qad_vector read_perm,
					   ac_xpu4_qad_vector write_perm,
					   ac_xpu4_qad_vector lock)
{
	ac_xpu4_qad_vector valid_vec = xpu4_hal_get_valid_qad_vector();
	ac_xpu4_qad_vector apps_vec = xpu4_get_apps_qad_vector();
	ac_xpu4_qad_vector read_apps = read_perm & apps_vec;
	ac_xpu4_qad_vector write_apps = write_perm & apps_vec;
	ac_xpu4_qad_vector lock_apps = lock & apps_vec;

	if ((~valid_vec & (read_perm | write_perm)) != 0U ||
	    (read_apps != 0U && !xpu4_is_valid_apps_qad_vector(read_apps)) ||
	    (write_apps != 0U && !xpu4_is_valid_apps_qad_vector(write_apps))) {
		ERROR("xpu4: invalid perm read=0x%x write=0x%x\n", read_perm,
		      write_perm);
		return XPU4_ERR_RG_PERM_MISMATCH;
	}

	if ((lock & ~apps_vec) != 0U ||
	    (lock & ~(read_perm | write_perm)) != 0U ||
	    (lock_apps != 0U && !xpu4_is_valid_apps_qad_vector(lock_apps))) {
		ERROR("xpu4: invalid lock read=0x%x write=0x%x lock=0x%x\n",
		      read_perm, write_perm, lock);
		return XPU4_ERR_RG_PERM_MISMATCH;
	}

	return XPU4_OK;
}

/* Resolve an XPU id to its cached identity, initialising it on first use. */
static enum xpu4_status
xpu4_init_and_get_info(enum xpu4_id xpu_id, struct ac_xpu4_priv_info **xpu)
{
	struct ac_xpu4_priv_info *info;
	enum xpu4_status rc;

	info = xpu4_hal_get_xpu_info(xpu_id);
	if (info == NULL) {
		ERROR("xpu4: no info for xpu_id=%u\n", (uint32_t)xpu_id);
		return XPU4_ERR_APU_INFO_NOT_FOUND;
	}

	if (info->rev == 0U) {
		rc = xpu4_hal_init_xpu(info);
		if (rc != XPU4_OK) {
			return rc;
		}
	}

	*xpu = info;

	return XPU4_OK;
}

/*
 * DDR region locking. These operate on the single DDR XPU discovered above.
 */

static enum xpu4_status xpu4_lock_memory(uintptr_t start, uintptr_t end,
					 ac_xpu4_qad_vector read_vec,
					 ac_xpu4_qad_vector write_vec,
					 ac_xpu4_qad_vector lock_vec)
{
	struct ac_xpu4_priv_info *xpu;
	enum xpu4_status rc;
	enum xpu4_id xpu_id;

	rc = xpu4_get_ddr_xpu(&xpu_id);
	if (rc != XPU4_OK) {
		return rc;
	}

	rc = xpu4_init_and_get_info(xpu_id, &xpu);
	if (rc != XPU4_OK) {
		return rc;
	}

	rc = xpu4_validate_address_region(start, end - start);
	if (rc != XPU4_OK) {
		return rc;
	}

	rc = xpu4_validate_address_alignment(xpu, start, end - start);
	if (rc != XPU4_OK) {
		return rc;
	}

	rc = xpu4_validate_perm(read_vec, write_vec, lock_vec);
	if (rc != XPU4_OK) {
		return rc;
	}

	return xpu4_hal_protect_dyn_region(xpu, start, end - start, false,
					   read_vec, write_vec, lock_vec);
}

/* Find the resource group already covering a range on the DDR XPU. */
static enum xpu4_status xpu4_get_rg_info(struct ac_xpu4_priv_info *xpu,
					 uintptr_t start, uintptr_t end,
					 uint32_t *rg_num)
{
	struct xpu4_partition_overlap overlap = { 0 };
	enum xpu4_status rc;

	rc = xpu4_hal_scan_mpu_region(xpu, start, end - start, 0U, &overlap);
	if (rc != XPU4_OK) {
		return rc;
	}

	if (!overlap.has_exact_match) {
		return XPU4_ERR_RG_NOT_FOUND;
	}

	*rg_num = overlap.rg_num;

	return XPU4_OK;
}

static enum xpu4_status xpu4_update_lock_memory(uintptr_t start, uintptr_t end,
						ac_xpu4_qad_vector read_vec,
						ac_xpu4_qad_vector write_vec,
						ac_xpu4_qad_vector lock_vec)
{
	struct ac_xpu4_priv_info *xpu;
	enum xpu4_status rc;
	enum xpu4_id xpu_id;
	uint32_t rg_num;

	rc = xpu4_get_ddr_xpu(&xpu_id);
	if (rc != XPU4_OK) {
		return rc;
	}

	rc = xpu4_init_and_get_info(xpu_id, &xpu);
	if (rc != XPU4_OK) {
		return rc;
	}

	rc = xpu4_validate_perm(read_vec, write_vec, lock_vec);
	if (rc != XPU4_OK) {
		return rc;
	}

	rc = xpu4_get_rg_info(xpu, start, end, &rg_num);
	if (rc != XPU4_OK) {
		return rc;
	}

	return xpu4_hal_update_dyn_region(xpu, rg_num, start, end - start,
					  false, read_vec, write_vec, lock_vec);
}

static enum xpu4_status xpu4_unlock_memory(uintptr_t start, uintptr_t end)
{
	struct ac_xpu4_priv_info *xpu;
	enum xpu4_status rc;
	enum xpu4_id xpu_id;
	uint32_t rg_num;

	rc = xpu4_get_ddr_xpu(&xpu_id);
	if (rc != XPU4_OK) {
		return rc;
	}

	rc = xpu4_init_and_get_info(xpu_id, &xpu);
	if (rc != XPU4_OK) {
		return rc;
	}

	rc = xpu4_get_rg_info(xpu, start, end, &rg_num);
	if (rc != XPU4_OK) {
		return rc;
	}

	return xpu4_hal_unprotect_dyn_region(xpu, rg_num, start, end - start);
}

/*
 * Static configuration.
 */

/*
 * Program every enabled XPU instance described by the config image.
 *
 * The image may carry more than one entry for the same instance, and the last
 * one wins. The duplicate check below only remembers the id handled most
 * recently, so the walk has to run from the highest index down for that to hold
 * — visiting forwards would make the *first* entry win instead.
 *
 * Each instance goes through four phases: resource groups, XPRESSCFG,
 * unmapped-region permissions and configuration ownership.
 */
static enum xpu4_status
xpu4_apply_static_xpu_config(const struct ac_xpu_cfg *cfg, uint32_t count)
{
	uint32_t xpu_id = UINT32_MAX;	/* an invalid instance id */
	uint32_t i;

	if (cfg == NULL) {
		return XPU4_ERR_NULL_POINTER;
	}

	/* From count - 1 down to 0; the loop ends on unsigned underflow. */
	for (i = count - 1U; i < count; i--) {
		struct ac_xpu4_priv_info *xpu;
		enum xpu4_status rc;

		if (cfg[i].status != TZBSP_XPU_ENABLE) {
			continue;
		}

		if (!xpu4_is_profile_enabled(cfg[i].profile_flags)) {
			continue;
		}

		if (cfg[i].xpu_id == xpu_id) {
			continue;
		}
		xpu_id = cfg[i].xpu_id;

		rc = xpu4_init_and_get_info((enum xpu4_id)cfg[i].xpu_id, &xpu);
		if (rc != XPU4_OK) {
			return rc;
		}

		rc = xpu4_hal_apply_static_config(xpu, &cfg[i]);
		if (rc != XPU4_OK) {
			return rc;
		}

		rc = xpu4_hal_apply_xpress_config(xpu, &cfg[i]);
		if (rc != XPU4_OK) {
			return rc;
		}

		rc = xpu4_hal_set_umr_perms(xpu, &cfg[i]);
		if (rc != XPU4_OK) {
			return rc;
		}

		rc = xpu4_hal_set_cfg_owner(xpu, &cfg[i]);
		if (rc != XPU4_OK) {
			return rc;
		}
	}

	return XPU4_OK;
}

/*
 * Report whether APPS-secure is the configuration owner of an instance, which
 * grants it access to unmapped space.
 */
static bool xpu4_is_config_owner(const struct ac_xpu4_priv_info *xpu)
{
	ac_xpu4_qad_vector owner = xpu4_hal_get_cfg_owner(xpu);

	return owner == QAD_VECTOR_AP_SEC;
}

/*
 * Report whether APPS-secure holds both read and write permission for an
 * address, per the XPU covering it.
 *
 * The permission vectors are converted to domain masks before testing, because
 * QAD_VECTOR_AP_SEC shares bit 0 with QAD_VECTOR_AP_NS: a raw vector test would
 * report success for a region only AP-non-secure can reach.
 */
int xpu_can_write(uintptr_t addr, bool *can_write)
{
	struct ac_xpu4_priv_info *xpu;
	struct xpu4_rg_info rg_info;
	enum xpu4_status rc;
	uint32_t xpu_type;
	uint32_t rg_num;
	bool found = false;

	if (can_write == NULL) {
		return -1;
	}

	*can_write = false;

	rc = xpu4_init_and_get_info(XPU4_ID_TCSR_REGS, &xpu);
	if (rc != XPU4_OK) {
		return -1;
	}

	xpu_type = xpu4_idr_xpu_type(xpu);

	if (xpu_type != (uint32_t)AC_MPU) {
		uint64_t start = addr;
		uint64_t size = sizeof(uint32_t);

		/* An RPU only resolves whole partitions, so align to one. */
		if (xpu_type == (uint32_t)AC_RPU) {
			uint64_t ptn_size =
				(uint64_t)1U << xpu4_idr_addr_lsb(xpu);

			size = ptn_size;
			start &= ~(ptn_size - 1U);
			rc = xpu4_hal_get_rpu_rg_from_range(xpu, start, size,
							    &rg_num);
		} else {
			rc = xpu4_hal_get_apu_rg_from_range(xpu, start, size,
							    &rg_num);
		}

		if (rc != XPU4_OK) {
			return -1;
		}

		rc = xpu4_hal_fill_rg_info(xpu, rg_num, &rg_info);
		if (rc != XPU4_OK) {
			return -1;
		}

		found = true;
	} else {
		uint32_t nrg = xpu4_idr_nrg(xpu) + 1U;
		uint32_t i;

		for (i = 0U; i < nrg; i++) {
			rc = xpu4_hal_fill_rg_info(xpu, i, &rg_info);
			if (rc != XPU4_OK) {
				return -1;
			}

			/* Skip groups that are disabled or describe no range. */
			if (!rg_info.rge ||
			    rg_info.start_addr == rg_info.end_addr ||
			    addr < rg_info.start_addr ||
			    addr >= rg_info.end_addr) {
				continue;
			}

			found = true;
			break;
		}
	}

	if (found) {
		uint32_t read_domains =
			xpu4_qad_vector_to_domain_mask(rg_info.read_perm_vector);
		uint32_t write_domains =
			xpu4_qad_vector_to_domain_mask(rg_info.write_perm_vector);

		if (((read_domains & write_domains) & TZBSP_DOMAIN_TZ_BIT) !=
		    0U) {
			*can_write = true;
		}

		return 0;
	}

	/*
	 * No resource group covers the address, so it falls in unmapped space.
	 * That is reachable by the configuration owner (or via the unmapped
	 * region permissions on XPU 4.2 and later), so allow it when
	 * APPS-secure owns the configuration.
	 */
	if (xpu_type == (uint32_t)AC_MPU) {
		*can_write = xpu4_is_config_owner(xpu);
	}

	return 0;
}

/*
 * Enable APPS non-secure permission enforcement on every relevant instance.
 *
 * Two passes, as downstream: the configured instances, then the instances named
 * by the dynamic protected ranges. The second pass matters because an instance
 * can appear only as a protected range and never in the configuration array,
 * and it still needs APNSPE set.
 */
static enum xpu4_status xpu4_set_xpu_apnspe(const struct ac_xpu_cfg *cfg,
					    uint32_t count)
{
	const struct ac_xpu_protected_range *range = NULL;
	uint32_t xpu_id = UINT32_MAX;
	uint32_t range_count = 0U;
	uint32_t i;

	if (cfg == NULL) {
		return XPU4_ERR_NULL_POINTER;
	}

	for (i = 0U; i < count; i++) {
		struct ac_xpu4_priv_info *xpu;
		enum xpu4_status rc;

		if (cfg[i].status != TZBSP_XPU_ENABLE) {
			continue;
		}

		if (!xpu4_is_profile_enabled(cfg[i].profile_flags)) {
			continue;
		}

		if (cfg[i].xpu_id == xpu_id) {
			continue;
		}
		xpu_id = cfg[i].xpu_id;

		rc = xpu4_init_and_get_info((enum xpu4_id)cfg[i].xpu_id, &xpu);
		if (rc != XPU4_OK) {
			return rc;
		}

		rc = xpu4_hal_configure_apnspe(xpu);
		if (rc != XPU4_OK) {
			return rc;
		}
	}

	if (ac_cfg_get_dynamic_protect_ranges(&range, &range_count) != 0) {
		return XPU4_ERR_APU_INFO_NOT_FOUND;
	}

	for (i = 0U; i < range_count; i++) {
		struct ac_xpu4_priv_info *xpu;
		enum xpu4_status rc;

		rc = xpu4_init_and_get_info(range[i].xpu_id, &xpu);
		if (rc != XPU4_OK) {
			return rc;
		}

		rc = xpu4_hal_configure_apnspe(xpu);
		if (rc != XPU4_OK) {
			return rc;
		}
	}

	return XPU4_OK;
}

/*
 * Rebuild the shadow permission cache for the instances holding dynamic
 * resource groups, so a later assignment sees what is already programmed.
 *
 * Each instance is visited once; the dynamic protected ranges may name the same
 * instance more than once.
 */
static enum xpu4_status xpu4_restore_shadow_perm(void)
{
	const struct ac_xpu_protected_range *range = NULL;
	uint32_t count = 0U;
	uint32_t i;
	uint32_t k;

	if (ac_cfg_get_dynamic_protect_ranges(&range, &count) != 0) {
		return XPU4_ERR_APU_INFO_NOT_FOUND;
	}

	for (i = 0U; i < count; i++) {
		struct ac_xpu4_priv_info *xpu;
		enum xpu4_status rc;
		bool restored = false;

		for (k = 0U; k < i; k++) {
			if (range[i].xpu_id == range[k].xpu_id) {
				restored = true;
				break;
			}
		}

		if (restored) {
			continue;
		}

		rc = xpu4_init_and_get_info(range[i].xpu_id, &xpu);
		if (rc != XPU4_OK) {
			return rc;
		}

		xpu4_hal_restore_shadow_perm(xpu);
	}

	return XPU4_OK;
}

/*
 * Apply the protection policy covering TZ's own assets.
 *
 * Nothing to do on XPU4: TZ's regions are described by the config image and are
 * already applied by xpu4_apply_static_xpu_config(). Kept so the static
 * configuration sequence reads the same for both XPU revisions.
 */
static void xpu_configure_tz(void)
{
}

/*
 * Interface implementation. See src/xpu_common.h.
 */

int acc_cfg_init(void)
{
	/*
	 * The config image is mandatory: without it there is no XPU or VMIDMT
	 * policy at all, so the caller treats failure as fatal rather than
	 * continuing unprotected.
	 */
	return ac_cfg_init();
}

void xpu_do_static_config(void)
{
	const struct ac_xpu_cfg *cfg = NULL;
	uint32_t count = 0U;
	bool enabled = false;
	enum xpu4_status rc;

	/*
	 * acc_cfg_init() must already have run. Every read below goes through
	 * the parser, which refuses to hand out anything until the image has
	 * been validated, so a misordered call fails loudly here.
	 */
	if (ac_cfg_get_xpu_enabled(&enabled) != 0) {
		ERROR("xpu4: cannot read XPU enable state\n");
		panic();
	}

	if (!enabled) {
		WARN("xpu4: XPU disabled by config image\n");
		return;
	}

	if (ac_cfg_get_global_xpu_cfg(&cfg, &count) != 0) {
		ERROR("xpu4: cannot read XPU configuration\n");
		panic();
	}

	/* Locate the DDR XPU before any dynamic assignment can be made. */
	xpu4_find_ddr_xpu(cfg, count);

	/*
	 * Bring up the HAL before the policy is applied, in the same order as
	 * downstream: enable permission enforcement, then rebuild the shadow
	 * permission cache from whatever is already programmed.
	 */
	rc = xpu4_set_xpu_apnspe(cfg, count);
	if (rc != XPU4_OK) {
		ERROR("xpu4: APNSPE configuration failed (%d)\n", (int)rc);
		panic();
	}

	rc = xpu4_restore_shadow_perm();
	if (rc != XPU4_OK) {
		ERROR("xpu4: shadow permission restore failed (%d)\n",
		      (int)rc);
		panic();
	}

	rc = xpu4_apply_static_xpu_config(cfg, count);
	if (rc != XPU4_OK) {
		ERROR("xpu4: static configuration failed (%d)\n", (int)rc);
		panic();
	}

	xpu_configure_tz();

	dsbsy();
}

int xpu_mem_assign(enum device_type device, enum domain_type domain,
		   uintptr_t start, uintptr_t end,
		   uint32_t perm_r, uint32_t perm_w)
{
	ac_xpu4_qad_vector read_vec;
	ac_xpu4_qad_vector write_vec;
	ac_xpu4_qad_vector lock_vec;
	enum xpu4_status rc;

	/*
	 * XPU4 protects DDR with a single instance rather than a per-device
	 * master-side MPU, so the device is not needed to locate it. The
	 * master-side path is not implemented.
	 */
	(void)device;
	(void)domain;

	read_vec = xpu4_domain_mask_to_qad_vector(perm_r);
	write_vec = xpu4_domain_mask_to_qad_vector(perm_w);

	/* No permissions at all means the region is being released. */
	if (read_vec == 0U && write_vec == 0U) {
		rc = xpu4_unlock_memory(start, end);
		return (rc == XPU4_OK) ? 0 : -1;
	}

	/*
	 * Lock only the QADs that were granted a permission, and only those
	 * belonging to this execution environment.
	 */
	lock_vec = (read_vec | write_vec) & xpu4_get_apps_qad_vector();

	/*
	 * Reprogram the resource group if one already covers the range,
	 * otherwise take a free one.
	 */
	rc = xpu4_update_lock_memory(start, end, read_vec, write_vec, lock_vec);
	if (rc == XPU4_ERR_RG_NOT_FOUND) {
		rc = xpu4_lock_memory(start, end, read_vec, write_vec,
				      lock_vec);
	}

	return (rc == XPU4_OK) ? 0 : -1;
}
