/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XPU_COMMON_H
#define XPU_COMMON_H

#include <stdint.h>

#include <drivers/qti/accesscontrol/accesscontrol.h>
#include <lib/utils_def.h>

/*
 * Interface shared by the XPU hardware revisions. Exactly one revision is
 * linked into a build, selected by XPU_VERSION in access_control.mk, so the
 * symbols below are defined by either src/xpu3/xpu3.c or src/xpu4/xpu4.c.
 *
 * The entry points deliberately take no configuration arguments. XPU3 reads
 * compiled-in tables from cfg/${CHIPSET}, whereas XPU4 has no build-time
 * configuration at all and fetches every structure from a config image
 * supplied at runtime. Keeping the tables out of the interface lets
 * access_control.c stay revision-agnostic.
 */

/* Interrupt context identifiers passed to xpu_print_log(). */
#define XPU_ERR_SEC_CTX		0
#define XPU_ERR_NON_SEC_CTX	1

/*
 * Bit position marking a region as secure. The permission values below match
 * the XPU3 RGn_CR1/CR3 register encoding directly; the XPU4 driver translates
 * them into QAD vectors internally.
 */
#define XPU_DOMAIN_SEC_SHIFT	8

enum domain_type {
	NO_DOMAIN = 0,
	APPS_NS_DOMAIN = BIT(0),
	APPS_S_DOMAIN = BIT(0) | BIT(XPU_DOMAIN_SEC_SHIFT),
	QAD0_NS_DOMAIN = BIT(1),
	QAD1_NS_DOMAIN = BIT(2),
};

#define MSA_DOMAIN	QAD0_NS_DOMAIN
#define SPU_DOMAIN	QAD1_NS_DOMAIN

enum device_type {
	DEVICE_MODEM = 30,
	DEVICE_MSS_NAV = 35,
};

/* One XPU error interrupt status or enable register. */
struct xpu_intr_reg_dtls {
	uintptr_t xpu_intr_reg_addr;
	uintptr_t xpu_intr_reg_mask;
};

/* Maps a bit in an error status register back to the XPU that raised it. */
struct xpu_err_pos_to_hal_map {
	uint32_t bit_mask;
	uint8_t xpu;
};

/*
 * Prepare the configuration source, before anything reads a configuration
 * structure from it. Returns 0 on success.
 *
 * XPU3 has nothing to do: its tables are compiled in. XPU4 parses and validates
 * the access-control config image here, which both the XPU and the VMIDMT
 * driver then read from, so this must run before either is configured.
 */
int acc_cfg_init(void);

/*
 * Report whether this execution environment may write the given address under
 * the XPU protection policy. Returns 0 on success, with *can_write set.
 *
 * Needed because some registers share a 4 KB page with assets another
 * environment owns, leaving TZ read-only on them; writing anyway would fault or
 * be silently dropped.
 */
int xpu_can_write(uintptr_t addr, bool *can_write);

/*
 * Apply the complete static protection policy: prepare the master-side MPU
 * regions that xpu_mem_assign() later hands out, program every XPU instance,
 * cover TZ's own assets, then enable the error interrupts in the aggregator.
 * The ordering between those steps is revision-specific and kept internal.
 */
void xpu_do_static_config(void);

/*
 * Register the XPU error interrupt handlers. Returns 0 on success. The
 * interrupt numbers are revision-specific: XPU3 uses compile-time ids while
 * XPU4 reads them from the config image.
 */
int xpu_register_interrupts(void);

/*
 * Decode and report a pending XPU violation. Called from interrupt context;
 * ctx is one of the XPU_ERR_*_CTX identifiers above.
 */
void xpu_print_log(void *ctx);

/*
 * Grant the given device read and write access to [start, end], expressed as
 * domain_type permission masks. Returns 0 on success.
 */
int xpu_mem_assign(enum device_type device, enum domain_type domain,
		   uintptr_t start, uintptr_t end,
		   uint32_t perm_r, uint32_t perm_w);

/*
 * Unified VMID/domain permission bitmask constants. These match the
 * TZBSP_DOMAIN_* / TZBSP_VMID_* values used by the QTI access control
 * subsystem. XPU3 writes perm_r/perm_w verbatim to CR1/CR3; XPU4 converts
 * them via xpu4_domain_mask_to_qad_vector() to QAD vectors before
 * programming hardware.
 */
#define AC_VMID_VMID_0_BIT    (1U << 0)   /* default NS VMID               */
#define AC_VMID_RPM_BIT       (1U << 1)   /* RPM VMID number   (XPU3 only) */
#define AC_VMID_AP_BIT        (1U << 3)   /* AP/HLOS VMID number(XPU3 only)*/
#define AC_DOMAIN_AOP_BIT     (1U << 27)  /* AOP/RPM domain    (XPU4 only) */
#define AC_DOMAIN_TZ_BIT      (1U << 28)  /* TZ/EL3 domain     (both)      */
#define AC_DOMAIN_MSA_BIT     (1U << 31)  /* MSA/modem domain  (both)      */

/*
 * Maps AC_VM_* to the perm_r/perm_w bitmask passed to xpu_mem_assign().
 * Returns 0 for unrecognised VMs callers skip those entries.
 *
 * Version-specific:
 *   XPU3: AC_VM_HLOS/RPM/TZ return 0 no XPU programming needed; slave-side
 *         DDR protection on XPU3 targets is handled via UMR Perms.
 *   XPU4: AC_VM_HLOS/RPM/TZ return their domain/VMID bits which
 *         xpu4_domain_mask_to_qad_vector() converts to QAD_VECTOR_AP_NS,
 *         QAD_VECTOR_AOP and QAD_VECTOR_AP_SEC respectively.
 */
static inline uint32_t vm_to_perm_mask(uint32_t vm_id)
{
	switch (vm_id) {
	case AC_VM_MSS_MSA:
	case AC_VM_MSS_NAV:
		return AC_DOMAIN_MSA_BIT;
#if XPU_VERSION == 4
	case AC_VM_TZ:
		return AC_DOMAIN_TZ_BIT;
	case AC_VM_RPM:
		return AC_DOMAIN_AOP_BIT;
	case AC_VM_HLOS:
		return AC_VMID_VMID_0_BIT;
#endif
	default:
		return 0U;
	}
}

#endif /* XPU_COMMON_H */
