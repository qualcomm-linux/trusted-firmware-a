/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <lib/utils_def.h>
#include <vmidmt.h>
#include <vmidmt_cfg.h>
#include <vmidmt_internal.h>
#include <vmidmt_target_hwio.h>

#include <shikra_def.h>

/*
 * VMIDMT instance base addresses (SCR0). Derived from the Shikra HWIO bases:
 *   CRYPTO0_CRYPTO_TOP     0x01b00000 + 0x00000 (BAM VMIDMT)
 *   RPM                    0x04600000 + 0x88000
 *   QPIC_QPIC              0x04840000 + 0x00000
 *   QUPV3_0_QUPV3_ID_3     0x04a00000 + 0xc6000
 *   QDSS_SOC_DBG           0x08000000 + 0x49000 (VMIDETR)
 *   QDSS_SOC_DBG           0x08000000 + 0x108000 (VMIDDAP)
 */
#define CRYPTO0_CRYPTO_BAM_VMIDMT_SCR0_ADDR 0x01b00000
#define RPM_VMIDMT_SCR0_ADDR 0x04688000
#define QPIC_QPIC_VMIDMT_SCR0_ADDR 0x04840000
#define QUPV3_0_VMIDMT_SCR0_ADDR 0x04ac6000
#define QDSS_VMIDETR_VMIDMT_SCR0_ADDR 0x08049000
#define QDSS_VMIDDAP_VMIDMT_SCR0_ADDR 0x08108000

/*
 * VMID mappings for HAL_VMIDMT_CRYPTO0_CRYPTO.
 *
 * SIDs 0-1 and 12-15 belong to the secure crypto pipes and stay with TZ; the
 * remaining SIDs are the non-secure pipes owned by the HLOS. Note the master
 * index and the SID diverge from index 10 onwards because SIDs 10 and 11 are
 * not assigned.
 */
static struct vmidmt_map g_vmid_map_crypto0_crypto[] = {
	{ { 0 }, 1, 0, ACC_VMID_TZ,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 1 }, 1, 1, ACC_VMID_TZ,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 2 }, 1, 2, ACC_VMID_AP,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 3 }, 1, 3, ACC_VMID_AP,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 4 }, 1, 4, ACC_VMID_AP,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 5 }, 1, 5, ACC_VMID_AP,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 6 }, 1, 6, ACC_VMID_AP,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 7 }, 1, 7, ACC_VMID_AP,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 8 }, 1, 8, ACC_VMID_AP,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 9 }, 1, 9, ACC_VMID_AP,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 12 }, 1, 10, ACC_VMID_TZ,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 13 }, 1, 11, ACC_VMID_TZ,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 14 }, 1, 12, ACC_VMID_TZ,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 15 }, 1, 13, ACC_VMID_TZ,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
};

/*
 * VMID mappings for HAL_VMIDMT_QPIC.
 *
 * SIDs 0-3 are owned by the HLOS; SIDs 4-7 are denied outright rather than
 * left at the reset value.
 */
static struct vmidmt_map g_vmid_map_qpic[] = {
	{ { 0 }, 1, 0, ACC_VMID_AP,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 1 }, 1, 1, ACC_VMID_AP,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 2 }, 1, 2, ACC_VMID_AP,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 3 }, 1, 3, ACC_VMID_AP,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 4 }, 1, 4, ACC_VMID_NOACCESS,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 5 }, 1, 5, ACC_VMID_NOACCESS,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 6 }, 1, 6, ACC_VMID_NOACCESS,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
	{ { 7 }, 1, 7, ACC_VMID_NOACCESS,
	  ACC_VMIDMT_FLAG_STATIC_CONFIG, ACC_VMIDMT_MEMTYPE_DEFAULT },
};

/*
 * The six VMIDMT instances Shikra configures from EL3, in the order the
 * downstream policy lists them. RPM, QUPV3_0 and the two QDSS instances carry
 * no static VMID table - they are only initialised and have error reporting
 * enabled.
 */
const struct vmidmt_cfg g_vmidmt_cfg[] = {
	{ NULL, 0, HAL_VMIDMT_CRYPTO0_CRYPTO, VMIDMT_ERR_OPT,
	  ACC_VMIDMT_STATIC_CONFIG_TZ, ACC_VMID_NOACCESS,
	  g_vmid_map_crypto0_crypto, ARRAY_SIZE(g_vmid_map_crypto0_crypto) },
	{ NULL, 0, HAL_VMIDMT_RPM, VMIDMT_ERR_OPT,
	  ACC_VMIDMT_STATIC_CONFIG_TZ, ACC_VMID_NOACCESS, NULL, 0 },
	{ NULL, 0, HAL_VMIDMT_QPIC, VMIDMT_ERR_OPT,
	  ACC_VMIDMT_STATIC_CONFIG_TZ, ACC_VMID_NOACCESS,
	  g_vmid_map_qpic, ARRAY_SIZE(g_vmid_map_qpic) },
	{ NULL, 0, HAL_VMIDMT_QUPV3_0, VMIDMT_ERR_OPT,
	  ACC_VMIDMT_STATIC_CONFIG_TZ, ACC_VMID_NOACCESS, NULL, 0 },
	{ NULL, 0, HAL_VMIDMT_QDSS_VMIDETR, VMIDMT_ERR_OPT,
	  ACC_VMIDMT_STATIC_CONFIG_TZ, ACC_VMID_NOACCESS, NULL, 0 },
	{ NULL, 0, HAL_VMIDMT_QDSS_VMIDDAP, VMIDMT_ERR_OPT,
	  ACC_VMIDMT_STATIC_CONFIG_TZ, ACC_VMID_NOACCESS, NULL, 0 },
};

const uint32_t g_vmidmt_cfg_count = ARRAY_SIZE(g_vmidmt_cfg);

/* Base address and SID count for each instance, from the Shikra HAL data. */
struct hal_vmidmt_info g_vmidmt_info_cfg[] = {
	{ HAL_VMIDMT_CRYPTO0_CRYPTO, CRYPTO0_CRYPTO_BAM_VMIDMT_SCR0_ADDR,
	  { 17, 0, 0, 0, 0, false }, false },
	{ HAL_VMIDMT_RPM, RPM_VMIDMT_SCR0_ADDR,
	  { 1, 0, 0, 0, 0, false }, false },
	{ HAL_VMIDMT_QPIC, QPIC_QPIC_VMIDMT_SCR0_ADDR,
	  { 10, 0, 0, 0, 0, false }, false },
	{ HAL_VMIDMT_QUPV3_0, QUPV3_0_VMIDMT_SCR0_ADDR,
	  { 48, 0, 0, 0, 0, false }, false },
	{ HAL_VMIDMT_QDSS_VMIDETR, QDSS_VMIDETR_VMIDMT_SCR0_ADDR,
	  { 2, 0, 0, 0, 0, false }, false },
	{ HAL_VMIDMT_QDSS_VMIDDAP, QDSS_VMIDDAP_VMIDMT_SCR0_ADDR,
	  { 2, 0, 0, 0, 0, false }, false },
};

const uint32_t g_vmidmt_info_cfg_count = ARRAY_SIZE(g_vmidmt_info_cfg);

/*
 * Bit position in the VMIDMT error status registers to the instance that
 * raised it. Positions with no VMIDMT behind them on this target are mapped to
 * HAL_VMIDMT_COUNT so the ISR skips them.
 */
struct vmidmt_err_pos_to_hal_map vmidmt_err_pos_to_hal_map
	[ACC_VMIDMT_ERR_INT_STATUS_REG_NUM][ACC_VMIDMT_ERR_NUM_PER_REG] = {
		{
			{ 0, HAL_VMIDMT_CRYPTO0_BAM },
			{ 1, HAL_VMIDMT_RPM_MSGRAM },
			{ 2, HAL_VMIDMT_QUPV3_0 },
			{ 3, HAL_VMIDMT_COUNT },
			{ 4, HAL_VMIDMT_QPIC_BAM },
			{ 5, HAL_VMIDMT_COUNT },
			{ 6, HAL_VMIDMT_COUNT },
			{ 7, HAL_VMIDMT_COUNT },
			{ 8, HAL_VMIDMT_COUNT },
			{ 9, HAL_VMIDMT_COUNT },
			{ 10, HAL_VMIDMT_QDSS_VMIDETR },
			{ 11, HAL_VMIDMT_QDSS_VMIDDAP },
			{ 12, HAL_VMIDMT_COUNT },
			{ 13, HAL_VMIDMT_COUNT },
			{ 14, HAL_VMIDMT_COUNT },
			{ 15, HAL_VMIDMT_COUNT },
			{ 16, HAL_VMIDMT_COUNT },
			{ 17, HAL_VMIDMT_COUNT },
		},
	};

static const struct vmidmt_intr_reg g_vmidmt_intr_reg[VMIDMT_INTR_COUNT] = {
	[VMIDMT_INTR_CLT_SEC] = {
		.status_addr = HWIO_TCSR_SS_VMIDMT_CLIENT_SEC_INTR_ADDR,
		.status_mask = HWIO_TCSR_SS_VMIDMT_CLIENT_SEC_INTR_RMSK,
		.enable_addr = HWIO_TCSR_SS_VMIDMT_CLIENT_SEC_INTR_ENABLE_ADDR,
		.intr_num = PLAT_INT_ID_VMIDMT_ERR_CLT_SEC,
	},
	[VMIDMT_INTR_CLT_NONSEC] = {
		.status_addr = HWIO_TCSR_SS_VMIDMT_CLIENT_NON_SEC_INTR_ADDR,
		.status_mask = HWIO_TCSR_SS_VMIDMT_CLIENT_NON_SEC_INTR_RMSK,
		.enable_addr =
			HWIO_TCSR_SS_VMIDMT_CLIENT_NON_SEC_INTR_ENABLE_ADDR,
		.intr_num = PLAT_INT_ID_VMIDMT_ERR_CLT_NONSEC,
	},
	[VMIDMT_INTR_CFG_SEC] = {
		.status_addr = HWIO_TCSR_SS_VMIDMT_CFG_SEC_INTR_ADDR,
		.status_mask = HWIO_TCSR_SS_VMIDMT_CFG_SEC_INTR_RMSK,
		.enable_addr = HWIO_TCSR_SS_VMIDMT_CFG_SEC_INTR_ENABLE_ADDR,
		.intr_num = PLAT_INT_ID_VMIDMT_ERR_CFG_SEC,
	},
	[VMIDMT_INTR_CFG_NONSEC] = {
		.status_addr = HWIO_TCSR_SS_VMIDMT_CFG_NON_SEC_INTR_ADDR,
		.status_mask = HWIO_TCSR_SS_VMIDMT_CFG_NON_SEC_INTR_RMSK,
		.enable_addr = HWIO_TCSR_SS_VMIDMT_CFG_NON_SEC_INTR_ENABLE_ADDR,
		.intr_num = PLAT_INT_ID_VMIDMT_ERR_CFG_NONSEC,
	},
};

int vmidmt_cfg_get_info_array(struct hal_vmidmt_info **info, uint32_t *count)
{
	*info = g_vmidmt_info_cfg;
	*count = g_vmidmt_info_cfg_count;

	return 0;
}

int vmidmt_cfg_get_cfg_array(const struct vmidmt_cfg **cfg, uint32_t *count)
{
	*cfg = g_vmidmt_cfg;
	*count = g_vmidmt_cfg_count;

	return 0;
}

int vmidmt_cfg_get_err_pos_map(const struct vmidmt_err_pos_to_hal_map **map,
			       uint32_t *reg_count, uint32_t *per_reg)
{
	*map = &vmidmt_err_pos_to_hal_map[0][0];
	*reg_count = ACC_VMIDMT_ERR_INT_STATUS_REG_NUM;
	*per_reg = ACC_VMIDMT_ERR_NUM_PER_REG;

	return 0;
}

int vmidmt_cfg_get_intr_reg(enum vmidmt_intr_id id,
			    const struct vmidmt_intr_reg **reg)
{
	if (id >= VMIDMT_INTR_COUNT) {
		return -1;
	}

	*reg = &g_vmidmt_intr_reg[id];

	return 0;
}
