/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/utils_def.h>
#include <xpu3.h>
#include <xpu_target_info.h>

#include <platform_def.h>

/* RG 20-24 are not added as qc_sec assigns them to the modem */
struct rg_mpu_cfg llcc_mpu_cfg[] = {
	{
		.index         = 0,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = NO_DOMAIN,
		.write_vmid    = NO_DOMAIN,
		.start         = BL31_BASE & 0xffffffffUL,
		.end           = (BL31_BASE + BL31_SIZE) & 0xffffffffUL,
	},
};

struct rg_mpu_cfg gemnoc_cnoc_mpu_cfg[] = {
	{
		.index         = 6,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x0c200000,
		.end           = 0x0c300000,
	},
	{
		.index         = 19,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x09a36000,
		.end           = 0x09a37000,
	},
	{
		.index         = 24,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x0c263000,
		.end           = 0x0c264000,
	},
	{
		.index         = 25,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x0c265000,
		.end           = 0x0c266000,
	},
	{
		.index         = 26,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x0c251000,
		.end           = 0x0c253000,
	},
	{
		.index         = 27,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x01790000,
		.end           = 0x01792000,
	},
	{
		.index         = 28,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x09a30000,
		.end           = 0x09a32000,
	},
	{
		.index         = 29,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x016b0000,
		.end           = 0x016b2000,
	},
	{
		.index         = 30,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x0acac000,
		.end           = 0x0acad000,
	},
	{
		.index         = XPU_UMR_RG,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x0,
		.end           = 0x0,
	},
};

/*
 * For RGs 24, 25 and 29, start and end addresses are subtracted with the base
 * address 0x0b000000 as per policy:
 *   RG 24, start = 0xc200000, end = 0xc210000
 *   RG 25, start = 0xc310000, end = 0xc3f1000
 *   RG 29, start = 0xc400000, end = 0xec00000
 */
struct rg_mpu_cfg aoss_cnoc_mpu_cfg[] = {
	{
		.index         = 10,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x0c266000,
		.end           = 0x0c270000,
	},
	{
		.index         = 13,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x0c2f7000,
		.end           = 0x0c2f8000,
	},
	{
		.index         = 16,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = NO_DOMAIN,
		.write_vmid    = NO_DOMAIN,
		.start         = 0xffffffff,
		.end           = 0xffffffff,
	},
	{
		.index         = 18,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x0c272000,
		.end           = 0x0c273000,
	},
	{
		.index         = 21,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x0c263000,
		.end           = 0x0c264000,
	},
	{
		.index         = 24,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = APPS_NS_DOMAIN,
		.write_vmid    = APPS_NS_DOMAIN,
		.start         = 0x0c200000,
		.end           = 0x0c210000,
	},
};

static struct rg_mpu_cfg ramblur_pimem_mpu_cfg[] = {
	{
		.index         = 7,
		.flags         = APPS_S_DOMAIN,
		.read_vmid     = NO_DOMAIN,
		.write_vmid    = NO_DOMAIN,
		.start         = 0x0,
		.end           = 0x0, /* full p_imem range */
	},
};

struct xpu_instance msm_xpu_cfg[] = {
	{
		.xpu_base_addr  = 0x00616000,
		.flags          = XPU_PROTECTION_STATIC,
		.nrg            = ARRAY_SIZE(ramblur_pimem_mpu_cfg),
		.rg             = { ramblur_pimem_mpu_cfg },
		.xpu_id         = XPU_TYPE_RAMBLUR_PIMEM_MPU,
	},
	{
		.xpu_base_addr  = 0x09230000,
		.flags          = XPU_PROTECTION_STATIC,
		.nrg            = ARRAY_SIZE(llcc_mpu_cfg),
		.rg             = { llcc_mpu_cfg },
		.xpu_id         = XPU_TYPE_LLCC_BROADCAST_MPU,
	},
	{
		.xpu_base_addr  = 0x0933e000,
		.flags          = XPU_PROTECTION_STATIC,
		.nrg            = ARRAY_SIZE(llcc_mpu_cfg),
		.rg             = { llcc_mpu_cfg },
		.xpu_id         = XPU_TYPE_LLCC_BROADCAST_MPU,
	},
	{
		.xpu_base_addr  = 0x01520000,
		.flags          = XPU_PROTECTION_STATIC,
		.nrg            = ARRAY_SIZE(gemnoc_cnoc_mpu_cfg),
		.rg             = { gemnoc_cnoc_mpu_cfg },
		.xpu_id         = XPU_TYPE_CNOC_GEMNOC_MPU,
	},
	{
		.xpu_base_addr  = 0x01524000,
		.flags          = 0,
		.nrg            = 0,
		.rg             = { NULL },
		/* CNOC2_SS unclocked at EL3 boot: GCR0 access stalls, so skip */
		.xpu_id         = XPU_TYPE_CNOC2_SS_MPU,
	},
	{
		.xpu_base_addr  = 0x0eff0000,
		.flags          = XPU_PROTECTION_STATIC,
		.nrg            = ARRAY_SIZE(aoss_cnoc_mpu_cfg),
		.rg             = { aoss_cnoc_mpu_cfg },
		.xpu_id         = XPU_TYPE_AOSS_MPU,
	},
};

const uint32_t msm_xpu_cfg_count = ARRAY_SIZE(msm_xpu_cfg);

/* QCS9075 does not have a modem subsystem - no modem MPU ranges */
struct mpu_ranges msm_mpu_ranges[] = {
};

const uint32_t msm_mpu_ranges_count = ARRAY_SIZE(msm_mpu_ranges);
