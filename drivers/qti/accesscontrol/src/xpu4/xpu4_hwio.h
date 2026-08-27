/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XPU4_HWIO_H
#define XPU4_HWIO_H

/*
 * XPU4 register offsets and field definitions.
 *
 * Offsets are relative to an XPU instance base address. Registers whose name
 * takes an index are replicated per resource group (RG) or per word; the
 * stride is folded into the offset macro.
 *
 * Only the registers the driver actually programs are defined here. The
 * downstream generated header additionally described IDR3-IDR10, RSR, CFGERE,
 * CLERE, DBGAR, GCR2, SYNR3, RG_MODIFIED, QADRGL_ALIAS and the duplicate
 * '...s' address variants, none of which are referenced.
 */

/* Spacing between the register blocks of consecutive resource groups. */
#define XPU4_RG_STRIDE				0x40U

/* Identification and revision. */
#define XPU4_IDR0_OFFSET			0x000U
#define XPU4_IDR1_OFFSET			0x004U
#define XPU4_IDR2_OFFSET			0x008U
#define XPU4_REV_OFFSET				0x00cU

#define XPU4_IDR0_NRG_BMSK			0x03ff0000U
#define XPU4_IDR0_NRG_SHFT			16U
#define XPU4_IDR0_XPRESSCFGEN_BMSK		0x00000004U
#define XPU4_IDR0_XPRESSCFGEN_SHFT		2U
#define XPU4_IDR0_XPU_TYPE_BMSK			0x00000003U
#define XPU4_IDR0_XPU_TYPE_SHFT			0U

#define XPU4_IDR1_CLIENT_ADDR_WIDTH_BMSK	0x3f000000U
#define XPU4_IDR1_CLIENT_ADDR_WIDTH_SHFT	24U
#define XPU4_IDR1_ADDR_MSB_BMSK			0x00003f00U
#define XPU4_IDR1_ADDR_MSB_SHFT			8U
#define XPU4_IDR1_ADDR_LSB_BMSK			0x0000003fU
#define XPU4_IDR1_ADDR_LSB_SHFT			0U

#define XPU4_REV_MAJOR_BMSK			0xf0000000U
#define XPU4_REV_MAJOR_SHFT			28U
#define XPU4_REV_MINOR_BMSK			0x0fff0000U
#define XPU4_REV_MINOR_SHFT			16U
#define XPU4_REV_STEP_BMSK			0x0000ffffU
#define XPU4_REV_STEP_SHFT			0U

/* Global configuration. */
#define XPU4_GCR_OFFSET				0x100U
#define XPU4_GCR_APNSPE_BMSK			0x00000001U
#define XPU4_GCR_APNSPE_SHFT			0U

#define XPU4_CFGOWNER_OFFSET			0x404U
#define XPU4_CFGOWNER_CFG_QAD_BMSK		0x0000001fU
#define XPU4_CFGOWNER_CFG_QAD_SHFT		0U

/* Unmapped region permissions (MPU only). */
#define XPU4_UMRPERMREG_OFFSET			0x408U

/* Error status and syndrome. */
#define XPU4_ESR_OFFSET				0x500U
#define XPU4_ESR_CLMULTI_BMSK			0x00000008U
#define XPU4_ESR_CLMULTI_SHFT			3U
#define XPU4_ESR_CFGMULTI_BMSK			0x00000004U
#define XPU4_ESR_CFGMULTI_SHFT			2U
#define XPU4_ESR_CLERR_BMSK			0x00000002U
#define XPU4_ESR_CLERR_SHFT			1U
#define XPU4_ESR_CFGERR_BMSK			0x00000001U
#define XPU4_ESR_CFGERR_SHFT			0U

#define XPU4_SYNAR0_OFFSET			0x504U
#define XPU4_SYNAR1_OFFSET			0x508U
#define XPU4_SYNR0_OFFSET			0x50cU
#define XPU4_SYNR1_OFFSET			0x510U
#define XPU4_SYNR2_OFFSET			0x514U

#define XPU4_SYNR0_QAD_BMSK			0x00001f00U
#define XPU4_SYNR0_QAD_SHFT			8U
#define XPU4_SYNR0_XPROTNS_BMSK			0x00000001U
#define XPU4_SYNR0_XPROTNS_SHFT			0U

#define XPU4_SYNR2_SLE_BMSK			0x01000000U
#define XPU4_SYNR2_SLE_SHFT			24U

/*
 * XPRESSCFG resource-group programming gates. Each is write-once; the matching
 * BOOT_CFG_STS bit reports whether it has already been written.
 */
#define XPU4_TME_GRP0_RG_PRG_OFFSET		0x600U
#define XPU4_TME_GRP1_RG_PRG_OFFSET		0x604U
#define XPU4_APPS_GRP0_RG_PRG_OFFSET		0x608U
#define XPU4_APPS_GRP1_RG_PRG_OFFSET		0x60cU

#define XPU4_TME_GRP0_RG_PRG_BMSK		0x00000001U
#define XPU4_TME_GRP0_RG_PRG_SHFT		0U
#define XPU4_TME_GRP1_RG_PRG_BMSK		0x00000001U
#define XPU4_TME_GRP1_RG_PRG_SHFT		0U
#define XPU4_APPS_GRP0_RG_PRG_BMSK		0x00000001U
#define XPU4_APPS_GRP0_RG_PRG_SHFT		0U
#define XPU4_APPS_GRP1_RG_PRG_BMSK		0x00000001U
#define XPU4_APPS_GRP1_RG_PRG_SHFT		0U

#define XPU4_BOOT_CFG_STS_OFFSET		0x610U
#define XPU4_BOOT_CFG_STS_POLICY_IGNORED_WO_BMSK	0x000000f0U
#define XPU4_BOOT_CFG_STS_POLICY_IGNORED_WO_SHFT	4U
#define XPU4_BOOT_CFG_STS_APPS_GRP1_WO_BMSK	0x00000008U
#define XPU4_BOOT_CFG_STS_APPS_GRP1_WO_SHFT	3U
#define XPU4_BOOT_CFG_STS_APPS_GRP0_WO_BMSK	0x00000004U
#define XPU4_BOOT_CFG_STS_APPS_GRP0_WO_SHFT	2U
#define XPU4_BOOT_CFG_STS_TME_GRP1_WO_BMSK	0x00000002U
#define XPU4_BOOT_CFG_STS_TME_GRP1_WO_SHFT	1U
#define XPU4_BOOT_CFG_STS_TME_GRP0_WO_BMSK	0x00000001U
#define XPU4_BOOT_CFG_STS_TME_GRP0_WO_SHFT	0U

/* Per-word bitmap of resource groups excluded from hardware policy. */
#define XPU4_HW_POLICY_IGNORED_OFFSET(r)	(0x620U + (0x4U * (r)))

/* Number of RGs covered by one HW_POLICY_IGNORED word. */
#define XPU4_HW_POLICY_IGNORED_RG_PER_REG	32U

/* Per-resource-group configuration. */
#define XPU4_RGCR0_OFFSET(n)	(0x1000U + (XPU4_RG_STRIDE * (n)))
#define XPU4_RGCR1_OFFSET(n)	(0x1004U + (XPU4_RG_STRIDE * (n)))
#define XPU4_RGCSAR1_OFFSET(n)	(0x1008U + (XPU4_RG_STRIDE * (n)))
#define XPU4_RGCSAR0_OFFSET(n)	(0x100cU + (XPU4_RG_STRIDE * (n)))
#define XPU4_RGCEAR1_OFFSET(n)	(0x1010U + (XPU4_RG_STRIDE * (n)))
#define XPU4_RGCEAR0_OFFSET(n)	(0x1014U + (XPU4_RG_STRIDE * (n)))
#define XPU4_RGRDR_OFFSET(n)	(0x1018U + (XPU4_RG_STRIDE * (n)))
#define XPU4_RGWRR_OFFSET(n)	(0x101cU + (XPU4_RG_STRIDE * (n)))
#define XPU4_QADRGL_OFFSET(n)	(0x1030U + (XPU4_RG_STRIDE * (n)))

/* RGCR0: write-owner-writes-permitted. */
#define XPU4_RGCR0_RGWOWP_BMSK			0x00000001U
#define XPU4_RGCR0_RGWOWP_SHFT			0U

/* RGCR1: resource group enable. */
#define XPU4_RGCR1_RGE_BMSK			0x00000001U
#define XPU4_RGCR1_RGE_SHFT			0U

#endif /* XPU4_HWIO_H */
