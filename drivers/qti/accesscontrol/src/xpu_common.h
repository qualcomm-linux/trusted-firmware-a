/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XPU_COMMON_H
#define XPU_COMMON_H

#include <stdint.h>

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

#endif /* XPU_COMMON_H */
