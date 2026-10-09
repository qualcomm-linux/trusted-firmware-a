/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <lib/utils_def.h>
#include <xpu3.h>
#include <xpu_target_info.h>

#include <platform_def.h>

/*
 * LLCC MPU protection for the EL3 carve-out.
 *
 * Keep the reference port's BL31-only protection in broadcast RG0. The other
 * DDR partitions belong to OP-TEE and images this port does not load.
 */
struct rg_domain_ownership llcc_mpu_rgs[] = {
	{ 0, APPS_S_DOMAIN, APPS_S_DOMAIN, APPS_S_DOMAIN },
};

struct rg_partition_range llcc_mpu_rg_addr[] = {
	{ 0, BL31_BASE & 0xffffffffUL,
	  (BL31_BASE + BL31_SIZE) & 0xffffffffUL },
};

/* DC_NOC_BROADCAST_MPU_MPU32Q2N7S1V0_40_CL36M27L12_AHB: enabled, static,
 * explicit default profile. */
struct rg_domain_ownership dc_noc_broadcast_mpu_rgs[] = {
	{  1, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN }, /* HYP */
	{  2, APPS_S_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN },
	{  3, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{  4, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{  5, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{  6, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{  7, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{  8, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{  9, APPS_S_DOMAIN, APPS_S_DOMAIN, APPS_S_DOMAIN },
	{ 10, APPS_S_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN },
	{ 12, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 13, APPS_S_DOMAIN, APPS_S_DOMAIN, APPS_S_DOMAIN },
	{ 15, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 17, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 18, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 19, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 20, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 22, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 23, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 25, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 26, APPS_S_DOMAIN, APPS_S_DOMAIN, APPS_S_DOMAIN },
	{ 28, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 29, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 30, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 31, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 32, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 33, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 34, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 35, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 36, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 37, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 38, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ XPU_UMR_RG, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
};

struct rg_partition_range dc_noc_broadcast_mpu_rg_addr[] = {
	{  1, 0x00e20000, 0x00e30000 },
	{  2, 0x00e34000, 0x00e41000 },
	{  3, 0x00e3d000, 0x00e40000 },
	{  4, 0x00e40000, 0x00e46000 },
	{  5, 0x00e3d000, 0x00e3e000 },
	{  6, 0x00e3f000, 0x00e40000 },
	{  7, 0x00e58000, 0x00e60000 },
	{  8, 0x00e40000, 0x00e45000 },
	{  9, 0x00e25000, 0x00e27000 },
	{ 10, 0x00e27000, 0x00e28000 },
	{ 12, 0x00e49000, 0x00e4c000 },
	{ 13, 0x00e28000, 0x00e2a000 },
	{ 15, 0x00e6b000, 0x00e6c000 },
	{ 17, 0x01020000, 0x01028000 },
	{ 18, 0x01034000, 0x0103c000 },
	{ 19, 0x0103c000, 0x01040000 },
	{ 20, 0x01048000, 0x0104c000 },
	{ 22, 0x0104d000, 0x0104e000 },
	{ 23, 0x01050000, 0x01055000 },
	{ 25, 0x01061000, 0x01064000 },
	{ 26, 0x01030000, 0x01032000 },
	{ 28, 0x0108b000, 0x0108c000 },
	{ 29, 0x00e50000, 0x00e56000 },
	{ 30, 0x00f00000, 0x00f30000 },
	{ 31, 0x00f4c000, 0x00f50000 },
	{ 32, 0x00f36000, 0x00f38000 },
	{ 33, 0x00f58000, 0x00f6a000 },
	{ 34, 0x01040000, 0x01046000 },
	{ 35, 0x01028000, 0x01030000 },
	{ 36, 0x01055000, 0x01056000 },
	{ 37, 0x01064000, 0x0106a000 },
	{ 38, 0x01058000, 0x01060000 },
};

/* DC_NOC_NON_BROADCAST_MPU_MPU32Q2N7S1V0_16_CL36M27L12_AHB: enabled, static,
 * explicit default profile. */
struct rg_domain_ownership dc_noc_qhs_non_broadcast_mpu_rgs[] = {
	{  0, APPS_S_DOMAIN, APPS_S_DOMAIN, APPS_S_DOMAIN },
	{  2, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{  3, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{  4, APPS_S_DOMAIN, NO_DOMAIN, NO_DOMAIN },
	{  5, APPS_S_DOMAIN, APPS_NS_DOMAIN, NO_DOMAIN },
	{  7, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{  9, APPS_S_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN },
	{ 10, APPS_S_DOMAIN, APPS_S_DOMAIN, APPS_S_DOMAIN },
	{ XPU_UMR_RG, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
};

struct rg_partition_range dc_noc_qhs_non_broadcast_mpu_rg_addr[] = {
	{  0, 0x00d00000, 0x00d28000 },
	{  2, 0x00d2a000, 0x00d40000 },
	{  3, 0x00c35000, 0x00c36000 },
	{  4, 0x00c36000, 0x00c37000 },
	{  5, 0x00c10000, 0x00c20000 },
	{  7, 0x01b8e000, 0x01b91000 },
	{  9, 0x00c90000, 0x00c94000 },
	{ 10, 0x00d40000, 0x00d44000 },
};

/* QM_MPU_CFG_QM_MPU_CFG_QM_MPU_CFG_MPU32Q2N7S1V0_4_CL36M23L12_AHB:
 * enabled, static, explicit default profile. */
struct rg_domain_ownership qm_mpu_cfg_rgs[] = {
	{  0, APPS_S_DOMAIN, APPS_S_DOMAIN, APPS_S_DOMAIN },
	{  1, APPS_NS_DOMAIN, NO_DOMAIN, NO_DOMAIN },
	{  2, APPS_NS_DOMAIN, NO_DOMAIN, NO_DOMAIN },
	{  3, APPS_S_DOMAIN, APPS_S_DOMAIN, APPS_S_DOMAIN },
	{ XPU_UMR_RG, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
};

struct rg_partition_range qm_mpu_cfg_rg_addr[] = {
	{  0, 0x01b80000, 0x01b81000 },
	{  1, 0x01b81000, 0x01b82000 },
	{  2, 0x01b82000, 0x01b83000 },
	{  3, 0x01b83000, 0x01b84000 },
};

/* CNOC_SNOC_MS_MPU_CFG: enabled, static, explicit default profile. */
struct rg_domain_ownership cnoc_snoc_ms_mpu_rgs[] = {
	{  7, APPS_S_DOMAIN, APPS_S_DOMAIN, APPS_S_DOMAIN },
	{ 18, APPS_S_DOMAIN, APPS_S_DOMAIN, APPS_S_DOMAIN },
	{ 19, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 25, APPS_S_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN },
	{ 26, APPS_S_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN },
	{ 27, APPS_S_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN },
	{ 30, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ 31, APPS_S_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN },
	{ 32, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
	{ XPU_UMR_RG, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
};

struct rg_partition_range cnoc_snoc_ms_mpu_rg_addr[] = {
	{  7, 0x01900000, 0x01902000 },
	{ 18, 0x045f4000, 0x045f5000 },
	{ 19, 0x0a66e000, 0x0a670000 },
	{ 25, 0x04b00000, 0x04b01000 },
	{ 26, 0x04b01000, 0x04b02000 },
	{ 27, 0x04b10000, 0x04b20000 },
	{ 30, 0x045f5000, 0x045f6000 },
	{ 31, 0x0b384000, 0x0b385000 },
	{ 32, 0x00d44000, 0x00d73000 },
};

/* CNOC_SNOC_QDSS_MPU_CFG_MPU32Q2N7S1V0_120_CL36M35L12_AHB: enabled, static,
 * explicit default profile. */
struct rg_domain_ownership cnoc_snoc_qdss_mpu_rgs[] = {
	{  6, APPS_S_DOMAIN, NO_DOMAIN, NO_DOMAIN },
	{ 10, APPS_S_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN },
	{ XPU_UMR_RG, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
};

struct rg_partition_range cnoc_snoc_qdss_mpu_rg_addr[] = {
	{  6, 0x018b3000, 0x018b4000 },
	{ 10, 0x0b925000, 0x0b928000 },
};

/* OCIMEM_MPU: enabled, static, explicit default profile. */
struct rg_domain_ownership ocimem_mpu_rgs[] = {
	{  0, APPS_S_DOMAIN, APPS_S_DOMAIN, APPS_S_DOMAIN },
	{  1, APPS_S_DOMAIN, APPS_S_DOMAIN, APPS_S_DOMAIN },
	{ XPU_UMR_RG, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
};

struct rg_partition_range ocimem_mpu_rg_addr[] = {
	{  0, 0x0c111000, 0x0c114000 },
	{  1, 0x0c100000, 0x0c111000 },
};

/* RPM_MSTR_MPU: enabled, static, explicit default profile. */
struct rg_domain_ownership rpm_mstr_mpu_rgs[] = {
	{  0, APPS_S_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN },
	{  1, APPS_S_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN },
	{  2, APPS_S_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN,
	   APPS_S_DOMAIN | APPS_NS_DOMAIN },
	{ XPU_UMR_RG, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
};

struct rg_partition_range rpm_mstr_mpu_rg_addr[] = {
	{  0, 0x04b00000, 0x04b01000 }, 
	{  1, 0x04b01000, 0x04b02000 }, 
	{  2, 0x04b10000, 0x04b20000 }, 
};

/* MCU_RVCP_SLV_RVCP_SLV_MPU32Q2N7S1V1_8_CL36M21L12_AHB: enabled, static,
 * explicit default profile. */
struct rg_domain_ownership mcu_rvcp_slv_rgs[] = {
	{  2, APPS_S_DOMAIN, NO_DOMAIN, NO_DOMAIN },
	{  3, APPS_S_DOMAIN, NO_DOMAIN, NO_DOMAIN },
	{  6, APPS_S_DOMAIN, NO_DOMAIN, NO_DOMAIN },
	{ XPU_UMR_RG, APPS_S_DOMAIN, APPS_NS_DOMAIN, APPS_NS_DOMAIN },
};

struct rg_partition_range mcu_rvcp_slv_rg_addr[] = {
	{  2, 0x0b840000, 0x0b88f00c }, /* CSR space */
	{  3, 0x0b900000, 0x0b93fffc }, /* RISC-V Core CSR space */
	{  6, 0x0b940000, 0x0b9ffffc }, /* DLS space */
};

/* Fixed LLCC plus all eligible XML XPU instances. */
struct xpu_instance msm_xpu_cfg[] = {
	{ HWIO_LLC_BROADCAST_LLCC_MPU_XPU3_GCR0_ADDR,
	  ARRAY_SIZE(llcc_mpu_rgs), llcc_mpu_rgs,
	  ARRAY_SIZE(llcc_mpu_rg_addr), llcc_mpu_rg_addr,
	  XPU_TYPE_LLCC_BROADCAST_MPU, XPU_PROTECTION_STATIC },
	{ HWIO_DC_NOC_BROADCAST_MPU_XPU3_GCR0_ADDR,
	  ARRAY_SIZE(dc_noc_broadcast_mpu_rgs), dc_noc_broadcast_mpu_rgs,
	  ARRAY_SIZE(dc_noc_broadcast_mpu_rg_addr),
	  dc_noc_broadcast_mpu_rg_addr,
	  XPU_TYPE_DC_NOC_BROADCAST_MPU, XPU_PROTECTION_STATIC },
	{ HWIO_DC_NOC_QHS_NON_BROADCAST_MPU_XPU3_GCR0_ADDR,
	  ARRAY_SIZE(dc_noc_qhs_non_broadcast_mpu_rgs),
	  dc_noc_qhs_non_broadcast_mpu_rgs,
	  ARRAY_SIZE(dc_noc_qhs_non_broadcast_mpu_rg_addr),
	  dc_noc_qhs_non_broadcast_mpu_rg_addr,
	  XPU_TYPE_DC_NOC_NON_BROADCAST_MPU, XPU_PROTECTION_STATIC },
	{ HWIO_QM_MPU_CFG_XPU3_GCR0_ADDR,
	  ARRAY_SIZE(qm_mpu_cfg_rgs), qm_mpu_cfg_rgs,
	  ARRAY_SIZE(qm_mpu_cfg_rg_addr), qm_mpu_cfg_rg_addr,
	  XPU_TYPE_QM_MPU_CFG, XPU_PROTECTION_STATIC },
	{ HWIO_CNOC_SNOC_MS_MPU_XPU3_GCR0_ADDR,
	  ARRAY_SIZE(cnoc_snoc_ms_mpu_rgs), cnoc_snoc_ms_mpu_rgs,
	  ARRAY_SIZE(cnoc_snoc_ms_mpu_rg_addr), cnoc_snoc_ms_mpu_rg_addr,
	  XPU_TYPE_CNOC_SNOC_MS_MPU, XPU_PROTECTION_STATIC },
	{ HWIO_CNOC_SNOC_QDSS_MPU_XPU3_GCR0_ADDR,
	  ARRAY_SIZE(cnoc_snoc_qdss_mpu_rgs), cnoc_snoc_qdss_mpu_rgs,
	  ARRAY_SIZE(cnoc_snoc_qdss_mpu_rg_addr), cnoc_snoc_qdss_mpu_rg_addr,
	  XPU_TYPE_CNOC_SNOC_MPU, XPU_PROTECTION_STATIC },
	{ HWIO_OCIMEM_MPU_XPU3_GCR0_ADDR,
	  ARRAY_SIZE(ocimem_mpu_rgs), ocimem_mpu_rgs,
	  ARRAY_SIZE(ocimem_mpu_rg_addr), ocimem_mpu_rg_addr,
	  XPU_TYPE_IMEM_MPU, XPU_PROTECTION_STATIC },
	{ HWIO_RPM_MSTR_MPU_XPU3_GCR0_ADDR,
	  ARRAY_SIZE(rpm_mstr_mpu_rgs), rpm_mstr_mpu_rgs,
	  ARRAY_SIZE(rpm_mstr_mpu_rg_addr), rpm_mstr_mpu_rg_addr,
	  XPU_TYPE_RPM_MSTR_MPU, XPU_PROTECTION_STATIC },
	{ HWIO_MCU_RVCP_SLV_XPU3_GCR0_ADDR,
	  ARRAY_SIZE(mcu_rvcp_slv_rgs), mcu_rvcp_slv_rgs,
	  ARRAY_SIZE(mcu_rvcp_slv_rg_addr), mcu_rvcp_slv_rg_addr,
	  XPU_TYPE_LMCU_MPU, XPU_PROTECTION_STATIC },
};

const uint32_t msm_xpu_cfg_count = ARRAY_SIZE(msm_xpu_cfg);

/* No runtime modem/subsystem-restart MPU ranges are managed by this port. */
struct mpu_ranges msm_mpu_ranges[] = {
};

const uint32_t msm_mpu_ranges_count = ARRAY_SIZE(msm_mpu_ranges);
