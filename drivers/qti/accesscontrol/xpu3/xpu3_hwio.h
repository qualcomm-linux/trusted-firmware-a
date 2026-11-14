/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XPU3_HWIO_H
#define XPU3_HWIO_H

#include <lib/utils_def.h>

/*
 * XPU3 register offsets and field masks.
 *
 * Offsets are relative to an instance's base address, taken from
 * struct xpu_instance::xpu_base_addr. Values were verified against the
 * XPU3 hardware register specification.
 */

/* Extract a field, given its mask. The shift is implied by the mask. */
#define FIELD_GET(mask, reg)		(((reg) & (mask)) >> __builtin_ctz(mask))

/*
 * Global control. GCR0 enables the instance; CR0 carries the four error
 * reporting and interrupt enables in bits 3:0.
 */
#define XPU3_GCR0_OFFSET		0x0U
#define XPU3_CR0_OFFSET			0x10U

#define XPU3_GCR0_DOMAIN_ENABLE		BIT(0)
#define XPU3_GCR0_LOG_MODE_DISABLE	BIT(1)
#define XPU3_CR0_INTR_ENABLE		(GENMASK(3, 0) | BIT(8))

/* Per-QAD control, same layout as the global pair above. */
#define XPU3_QAD0_GCR0_OFFSET		0x80U
#define XPU3_QAD0_CR0_OFFSET		0x90U
#define XPU3_QAD1_GCR0_OFFSET		0x100U
#define XPU3_QAD1_CR0_OFFSET		0x110U

/* Unmapped region: ownership, the four client permission registers, VMID. */
#define XPU3_UMR_RACR_OFFSET		0x40U
#define XPU3_UMR_WACR_OFFSET		0x60U
#define XPU3_UMR_GCR0_OFFSET		0x300U
#define XPU3_UMR_CR0_OFFSET		0x310U
#define XPU3_UMR_CR1_OFFSET		0x314U
#define XPU3_UMR_CR2_OFFSET		0x318U
#define XPU3_UMR_CR3_OFFSET		0x31CU

/* Silent logging. Only meaningful on the revisions that implement it. */
#define XPU3_LOG_MODE_DIS_OFFSET	0x400U

/* Identity and revision. Read-only. */
#define XPU3_IDR2_OFFSET		0x3F0U
#define XPU3_IDR0_OFFSET		0x3F8U
#define XPU3_REV_OFFSET			0x3FCU

/* IDR0.XPUTYPE. Compare the extracted value against XPU_TYPE_MPU in xpu3.h. */
#define XPU_TYPE_BITMASK		GENMASK(1, 0)

/* IDR0.NRG holds the resource group count, less one. */
#define XPU3_IDR0_NRG_BMSK		GENMASK(25, 16)

/* IDR2.VMIDACR_EN: non-zero when the instance implements VMID permissions. */
#define XPU_IDR2_VMID_SUPPORT_BITMASK	0xFF00U
#define XPU_IDR2_VMID_SUPPORT_BITSHIFT	0x8U

/* REV, shifted down to the packed major:minor form the driver compares. */
#define XPU3_REV_SHFT			0x10U

/*
 * Resource group registers. Each group occupies its own XPU3_RG_STRIDE-sized
 * window, so the offsets take the group number.
 */
#define XPU3_RG_STRIDE			0x80U

#define XPU3_RGN_GCR0_OFFSET(n)	(0x1000U + (XPU3_RG_STRIDE * (n)))
/*
 * GCR1.PD (bit 31): set to disable the partition before programming its
 * address range, clear to re-enable it after. Required to prevent an empty
 * RG (start=0, end=0) from blocking the full address range during the update.
 */
#define XPU3_RGN_GCR1_OFFSET(n)	(0x1004U + (XPU3_RG_STRIDE * (n)))
#define XPU3_RGN_GCR1_PD_BIT		BIT(31)
/*
 * GCR2 controls the shadow-register commit mechanism for atomic address
 * range updates. Write ABORT to clear the shadow, write the new addresses
 * to the SSHADOW registers, then write COMMIT to atomically apply them.
 */
#define XPU3_RGN_GCR2_OFFSET(n)	(0x1008U + (XPU3_RG_STRIDE * (n)))
#define XPU3_RGN_GCR2_ABORT		BIT(1)
#define XPU3_RGN_GCR2_COMMIT		BIT(0)
#define XPU3_RGN_CR0_OFFSET(n)		(0x1010U + (XPU3_RG_STRIDE * (n)))
#define XPU3_RGN_CR1_OFFSET(n)		(0x1014U + (XPU3_RG_STRIDE * (n)))
#define XPU3_RGN_CR2_OFFSET(n)		(0x1018U + (XPU3_RG_STRIDE * (n)))
#define XPU3_RGN_CR3_OFFSET(n)		(0x101CU + (XPU3_RG_STRIDE * (n)))
/*
 * Shadow address registers. These are global (not per-RG); the RG to update
 * is selected by which GCR2 the COMMIT is written to.
 */
#define XPU3_RGN_START0_SSHADOW_OFFSET	0x830U
#define XPU3_RGN_START1_SSHADOW_OFFSET	0x834U
#define XPU3_RGN_END0_SSHADOW_OFFSET	0x838U
#define XPU3_RGN_END1_SSHADOW_OFFSET	0x83CU
/* Direct address registers (read path / legacy). */
#define XPU3_RGN_START0_OFFSET(n)	(0x1030U + (XPU3_RG_STRIDE * (n)))
#define XPU3_RGN_START1_OFFSET(n)	(0x1034U + (XPU3_RG_STRIDE * (n)))
#define XPU3_RGN_END0_OFFSET(n)		(0x1038U + (XPU3_RG_STRIDE * (n)))
#define XPU3_RGN_END1_OFFSET(n)		(0x103CU + (XPU3_RG_STRIDE * (n)))
#define XPU3_RGN_RACR_OFFSET(n)		(0x1040U + (XPU3_RG_STRIDE * (n)))
#define XPU3_RGN_WACR_OFFSET(n)		(0x1060U + (XPU3_RG_STRIDE * (n)))

/* Value written to RACR/WACR to grant the default non-secure VMID. */
#define DEFAULT_VMID_0			(1U << 0)

/*
 * Violation reporting. The secure and non-secure banks have identical layouts,
 * so the register offsets below are relative to one of these two block bases.
 */
#define XPU3_ERR_BLK_SEC		0x800U
#define XPU3_ERR_BLK_NONSEC		0x880U

#define XPU3_EAR0_OFFSET		0x0
#define XPU3_EAR1_OFFSET		0x4
#define XPU3_ESR_OFFSET			0x8
#define XPU3_SRRESTORE_OFFSET		0xC
#define XPU3_ESYNR0_OFFSET		0x10
#define XPU3_ESYNR1_OFFSET		0x14
#define XPU3_ESYNR2_OFFSET		0x18
#define XPU3_ESYNR3_OFFSET		0x1C
#define XPU3_ESYNR4_OFFSET		0x20

/*
 * SCR0 is the secure-domain counterpart of CR0. It carries the same four error
 * reporting and interrupt enables and receives the same value as CR0.
 *
 */
#define XPU3_SCR0_OFFSET		0x8U
#define XPU3_GCR0_BASE_CFG		XPU3_GCR0_DOMAIN_ENABLE

#endif /* XPU3_HWIO_H */
