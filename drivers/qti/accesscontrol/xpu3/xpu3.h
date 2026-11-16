/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XPU3_H
#define XPU3_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <xpu_common.h>

#define XPU_RG_ALL 0x00ffffff
#define XPU_UMR_RG (XPU_RG_ALL - 1)

/* Bits in struct xpu_instance::flag. */
#define XPU_PROTECTION_STATIC 0x1
#define XPU_INITIALIZED 0x2

/*
 * Unit type, as reported by IDR0.XPUTYPE and returned by
 * xpu3_hal_get_xpu_type(). Address-range programming is MPU-only.
 * Unrelated to the instance identifiers in enum xpu below, despite the shared
 * XPU_TYPE_ prefix.
 */
#define XPU_TYPE_RPU 0x0U
#define XPU_TYPE_APU 0x1U
#define XPU_TYPE_MPU 0x2U

enum xpu {
	XPU_TYPE_NONE,
	XPU_TYPE_IPA_0_GSI_TOP,
	XPU_TYPE_SEC_CTRL_APU,
	XPU_TYPE_ANOC1_MPU,
	XPU_TYPE_ANOC2_MPU,
	XPU_TYPE_BIMC_MPU0,
	XPU_TYPE_MSS_NAV_MPU,
	XPU_TYPE_LLCC_BROADCAST_MPU,
	XPU_TYPE_AOSS_MPU,
	XPU_TYPE_GEMNOC_MS_MPU,
	XPU_TYPE_BOOT_ROM,
	XPU_TYPE_IMEM_MPU,
	XPU_TYPE_CNOC_SNOC_MPU,
	XPU_TYPE_MSS_Q6_MPU,
	XPU_TYPE_MSS_MPU,
	XPU_TYPE_BIMC_MPU1,
	XPU_TYPE_DC_NOC_NON_BROADCAST_MPU,
	XPU_TYPE_DC_NOC_SHRM_MPU,
	XPU_TYPE_PKA_APU,
	XPU_TYPE_IPC_MPU,
	XPU_TYPE_CNOC_GEMNOC_MPU,
	XPU_TYPE_CNOC2_SS_MPU,
	XPU_TYPE_WPSS_MPU,
	XPU_TYPE_AOSS_CNOC_MPU,
	XPU_TYPE_CNOC_SS_MPU,
	XPU_TYPE_MDSS_0_MDSS_APU,
	XPU_TYPE_MDSS_1_MDSS_APU,
	XPU_TYPE_RAMBLUR_PIMEM_MPU,
	XPU_TYPE_PMIC_ARB,
	XPU_TYPE_BAM_BLSP1_DMA,
	XPU_TYPE_TCSR_REGS,
	XPU_TYPE_BAM_BLSP2_DMA,
	XPU_TYPE_TCSR_MUTEX,
	XPU_TYPE_CRYPTO0_BAM,
	XPU_TYPE_IMEM_APU,
	XPU_TYPE_DCC,
	XPU_TYPE_LPASS_XPU_AUD,
	XPU_TYPE_LPASS_WSA2_DMA_MPU,
	XPU_TYPE_LPASS_XPU_AUD_SB,
	XPU_TYPE_RAMBLUR_PIMEM_APU,
	XPU_TYPE_QUP_AXI,
	XPU_TYPE_CNOC_AOSS_MPU,
	XPU_TYPE_CNOC_SNOC_MS_MPU,
	XPU_TYPE_QM_MPU_CFG,
	XPU_TYPE_AOP_MS_MPU,
	XPU_TYPE_GCC_RPU,
	XPU_TYPE_TITAN_SS_APU2,
	XPU_TYPE_BOOT_IMEM_SS_MPU,
	XPU_TYPE_UFS_ICE,
	XPU_TYPE_BIMC_MPU2,
	XPU_TYPE_BIMC_MPU3,
	XPU_TYPE_NOC_BROADCAST_MPU,
	XPU_TYPE_CRYPTO1_BAM,
	XPU_TYPE_SPDM_APU,
	XPU_TYPE_TITAN_SS_APU1,
	XPU_TYPE_MDSS0,
	XPU_TYPE_TLMM,
	XPU_TYPE_CRYPTO1_CRYPTO_KM_APU,
	XPU_TYPE_BAM_BLSP3_DMA,
	XPU_TYPE_PKE_APU,
	XPU_TYPE_MDSS1,
	XPU_TYPE_ETH_MS_MPU,
	XPU_TYPE_LPASS_Q6SS_MPU,
	XPU_TYPE_LPASS_DEBUG_MPU,
	XPU_TYPE_LPASS_VA_MPU,
	XPU_TYPE_LPASS_WSA_MPU,
	XPU_TYPE_LPASS_CFG_MPU,
	XPU_TYPE_LPASS_RXTX_MPU,
	XPU_TYPE_CRYPTO0_CRYPTO_KM_APU,
	XPU_TYPE_HWKM_CFG_KM_APU,
	XPU_TYPE_UFS_MEM_ICE_KM_APU,
	XPU_TYPE_UFS_CARD_ICE_KM_APU,
	XPU_TYPE_TSCSS_MPU,
	XPU_TYPE_GIC_MS_MPU,
	XPU_TYPE_BIMC_MPU4,
	XPU_TYPE_BIMC_MPU5,
	XPU_TYPE_R52_MS_MPU,
	XPU_TYPE_SDC1_SDCC_ICE,
	XPU_TYPE_SDC1_SDCC_ICE_KM_APU,
	XPU_TYPE_APSS_GIC_WRAPPER_MS_MPU,
	XPU_TYPE_LPASS_SSC_Q6_MPU,
	XPU_TYPE_LPASS_WSA_2CH_MPU,
	XPU_TYPE_UFS_G4_ICE,
	/*
	 * Shikra (Bruin) instances with no counterpart on the targets above.
	 * Appended here so the existing identifiers keep their values.
	 */
	XPU_TYPE_RPM_MSTR_MPU,
	XPU_TYPE_MAPSS_APU,
	XPU_TYPE_CAMSS,
	XPU_TYPE_QPIC_APU,
	XPU_TYPE_MSS_CRYPTO_KM_APU,
	XPU_TYPE_SNOC_MS_MPU,
	XPU_TYPE_MDSS_XPU,
	XPU_TYPE_LMCU_MPU,
	XPU_TYPE_DC_NOC_BROADCAST_MPU,
	XPU_TYPE_MDSP_MPU,
	XPU_TYPE_MSS_APU,
	XPU_TYPE_COUNT,
};

/*
 * Merged MPU resource-group config (ownership + address range in one entry).
 * Mirrors the downstream tzbsp_mpu_rg_t shape. profile_flags is declared for
 * layout parity but is unused in TF-A and must always be initialised to 0.
 * index uses uint32_t (not uint16 as downstream) to accommodate XPU_UMR_RG.
 */
struct rg_mpu_cfg {
	uint32_t index;		/* RG number; XPU_UMR_RG for unmapped region */
	uint32_t profile_flags;	/* unused in TF-A; always 0 */
	uint32_t flags;		/* owner domain (APPS_S_DOMAIN, MSA_DOMAIN …) */
	uint32_t read_vmid;		/* perm_r */
	uint32_t write_vmid;		/* perm_w */
	uintptr_t start;
	uintptr_t end;
};

/*
 * APU/RPU resource-group config. Mirrors downstream tzbsp_rpu_rg_t — same
 * field names so downstream config data can be used with minimal changes.
 * profile_flags is unused in TF-A and must always be 0.
 */
struct rg_rpu_cfg {
	uint32_t index;		/* RG number */
	uint32_t profile_flags;	/* unused in TF-A; always 0 */
	uint32_t flags;		/* owner domain */
	uint32_t read_vmid;		/* perm_r */
	uint32_t write_vmid;		/* perm_w */
};

/*
 * Per-XPU address masking descriptor.
 * Some MPUs expose only base-relative addresses in their START/END registers.
 * Before programming, subtract base_addr and AND with mask.
 */
struct xpu_addr_mask {
	enum xpu xpu_id;
	uint32_t mask;
	uint32_t base_addr;
};

/*
 * Per-XPU instance descriptor. Mirrors the downstream tzbsp_xpu_cfg_t shape.
 * profile_flags, unmapped_rvmid, unmapped_wvmid and superuser_vmid are
 * declared for layout parity but are unused in TF-A and must be 0.
 */
struct xpu_instance {
	uintptr_t xpu_base_addr;
	uint32_t flags;		/* XPU_PROTECTION_STATIC / XPU_INITIALIZED */
	uint32_t profile_flags;	/* unused in TF-A; always 0 */
	uint32_t unmapped_rvmid;	/* unused in TF-A; always 0 */
	uint32_t unmapped_wvmid;	/* unused in TF-A; always 0 */
	uint32_t superuser_vmid;	/* unused in TF-A; always 0 */
	uint32_t nrg;			/* element count of the rg array */
	union {
		const void *any;	/* first member — set via .rg = { array } */
		struct rg_mpu_cfg *mpu;	/* HAL MPU functions */
		struct rg_rpu_cfg *rpu;	/* HAL APU/RPU functions */
	} rg;
	enum xpu xpu_id;
};

struct mpu_ranges {
	const enum device_type device;
	uint8_t mpus_count;
	uint16_t device_prtn_cnt;
	struct xpu_instance *mpus;
};

struct xpu_base_addr_info {
	enum xpu e_xpu;
	uintptr_t base_addr;
	char *name;
};

/*
 * Unmask the XPU error interrupts in the aggregator. Called once the static
 * policy is in place. Either register list may be NULL to leave that bank
 * masked. Implemented in xpu3_isr.c.
 */
void xpu3_enable_interrupts(const struct xpu_intr_reg_dtls *nsec,
			    const struct xpu_intr_reg_dtls *sec);

#endif /* XPU3_H */
