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
	{
		.sid_list = { 0 },
		.num_sids = 1,
		.index = 0,
		.vmid = ACC_VMID_TZ,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 1 },
		.num_sids = 1,
		.index = 1,
		.vmid = ACC_VMID_TZ,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 2 },
		.num_sids = 1,
		.index = 2,
		.vmid = ACC_VMID_AP,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 3 },
		.num_sids = 1,
		.index = 3,
		.vmid = ACC_VMID_AP,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 4 },
		.num_sids = 1,
		.index = 4,
		.vmid = ACC_VMID_AP,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 5 },
		.num_sids = 1,
		.index = 5,
		.vmid = ACC_VMID_AP,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 6 },
		.num_sids = 1,
		.index = 6,
		.vmid = ACC_VMID_AP,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 7 },
		.num_sids = 1,
		.index = 7,
		.vmid = ACC_VMID_AP,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 8 },
		.num_sids = 1,
		.index = 8,
		.vmid = ACC_VMID_AP,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 9 },
		.num_sids = 1,
		.index = 9,
		.vmid = ACC_VMID_AP,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 12 },
		.num_sids = 1,
		.index = 10,
		.vmid = ACC_VMID_TZ,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 13 },
		.num_sids = 1,
		.index = 11,
		.vmid = ACC_VMID_TZ,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 14 },
		.num_sids = 1,
		.index = 12,
		.vmid = ACC_VMID_TZ,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 15 },
		.num_sids = 1,
		.index = 13,
		.vmid = ACC_VMID_TZ,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
};

/*
 * VMID mappings for HAL_VMIDMT_QPIC.
 *
 * SIDs 0-3 are owned by the HLOS; SIDs 4-7 are denied outright rather than
 * left at the reset value.
 */
static struct vmidmt_map g_vmid_map_qpic[] = {
	{
		.sid_list = { 0 },
		.num_sids = 1,
		.index = 0,
		.vmid = ACC_VMID_AP,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 1 },
		.num_sids = 1,
		.index = 1,
		.vmid = ACC_VMID_AP,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 2 },
		.num_sids = 1,
		.index = 2,
		.vmid = ACC_VMID_AP,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 3 },
		.num_sids = 1,
		.index = 3,
		.vmid = ACC_VMID_AP,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 4 },
		.num_sids = 1,
		.index = 4,
		.vmid = ACC_VMID_NOACCESS,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 5 },
		.num_sids = 1,
		.index = 5,
		.vmid = ACC_VMID_NOACCESS,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 6 },
		.num_sids = 1,
		.index = 6,
		.vmid = ACC_VMID_NOACCESS,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
	{
		.sid_list = { 7 },
		.num_sids = 1,
		.index = 7,
		.vmid = ACC_VMID_NOACCESS,
		.flags = ACC_VMIDMT_FLAG_STATIC_CONFIG,
		.memattr = ACC_VMIDMT_MEMTYPE_DEFAULT,
	},
};

/*
 * The six VMIDMT instances Shikra configures from EL3, in the order the
 * downstream policy lists them. RPM, QUPV3_0 and the two QDSS instances carry
 * no static VMID table - they are only initialised and have error reporting
 * enabled.
 */
const struct vmidmt_cfg g_vmidmt_cfg[] = {
	{
		.ssd_table = NULL,
		.ssd_table_num_elements = 0,
		.table = HAL_VMIDMT_CRYPTO0_CRYPTO,
		.err_opt = VMIDMT_ERR_OPT,
		.static_cfg = ACC_VMIDMT_STATIC_CONFIG_TZ,
		.secure_bypass_vmid = ACC_VMID_NOACCESS,
		.vmid_map = g_vmid_map_crypto0_crypto,
		.vmid_map_count = ARRAY_SIZE(g_vmid_map_crypto0_crypto),
	},
	{
		.ssd_table = NULL,
		.ssd_table_num_elements = 0,
		.table = HAL_VMIDMT_RPM,
		.err_opt = VMIDMT_ERR_OPT,
		.static_cfg = ACC_VMIDMT_STATIC_CONFIG_TZ,
		.secure_bypass_vmid = ACC_VMID_NOACCESS,
		.vmid_map = NULL,
		.vmid_map_count = 0,
	},
	{
		.ssd_table = NULL,
		.ssd_table_num_elements = 0,
		.table = HAL_VMIDMT_QPIC,
		.err_opt = VMIDMT_ERR_OPT,
		.static_cfg = ACC_VMIDMT_STATIC_CONFIG_TZ,
		.secure_bypass_vmid = ACC_VMID_NOACCESS,
		.vmid_map = g_vmid_map_qpic,
		.vmid_map_count = ARRAY_SIZE(g_vmid_map_qpic),
	},
	{
		.ssd_table = NULL,
		.ssd_table_num_elements = 0,
		.table = HAL_VMIDMT_QUPV3_0,
		.err_opt = VMIDMT_ERR_OPT,
		.static_cfg = ACC_VMIDMT_STATIC_CONFIG_TZ,
		.secure_bypass_vmid = ACC_VMID_NOACCESS,
		.vmid_map = NULL,
		.vmid_map_count = 0,
	},
	{
		.ssd_table = NULL,
		.ssd_table_num_elements = 0,
		.table = HAL_VMIDMT_QDSS_VMIDETR,
		.err_opt = VMIDMT_ERR_OPT,
		.static_cfg = ACC_VMIDMT_STATIC_CONFIG_TZ,
		.secure_bypass_vmid = ACC_VMID_NOACCESS,
		.vmid_map = NULL,
		.vmid_map_count = 0,
	},
	{
		.ssd_table = NULL,
		.ssd_table_num_elements = 0,
		.table = HAL_VMIDMT_QDSS_VMIDDAP,
		.err_opt = VMIDMT_ERR_OPT,
		.static_cfg = ACC_VMIDMT_STATIC_CONFIG_TZ,
		.secure_bypass_vmid = ACC_VMID_NOACCESS,
		.vmid_map = NULL,
		.vmid_map_count = 0,
	},
};

const uint32_t g_vmidmt_cfg_count = ARRAY_SIZE(g_vmidmt_cfg);

/* Base address and SID count for each instance, from the Shikra HAL data. */
struct hal_vmidmt_info g_vmidmt_info_cfg[] = {
	{
		.vmidmt = HAL_VMIDMT_CRYPTO0_CRYPTO,
		.base_addr = CRYPTO0_CRYPTO_BAM_VMIDMT_SCR0_ADDR,
		.dev_params = {
			.entry_count = 17,
			.num_vmid = 0,
			.num_ssd_index_bits = 0,
			.num_stream_id_bits = 0,
			.input_addr_size = 0,
			.stream_match_support = 0,
		},
		.is_initialised = false,
	},
	{
		.vmidmt = HAL_VMIDMT_RPM,
		.base_addr = RPM_VMIDMT_SCR0_ADDR,
		.dev_params = {
			.entry_count = 1,
			.num_vmid = 0,
			.num_ssd_index_bits = 0,
			.num_stream_id_bits = 0,
			.input_addr_size = 0,
			.stream_match_support = 0,
		},
		.is_initialised = false,
	},
	{
		.vmidmt = HAL_VMIDMT_QPIC,
		.base_addr = QPIC_QPIC_VMIDMT_SCR0_ADDR,
		.dev_params = {
			.entry_count = 10,
			.num_vmid = 0,
			.num_ssd_index_bits = 0,
			.num_stream_id_bits = 0,
			.input_addr_size = 0,
			.stream_match_support = 0,
		},
		.is_initialised = false,
	},
	{
		.vmidmt = HAL_VMIDMT_QUPV3_0,
		.base_addr = QUPV3_0_VMIDMT_SCR0_ADDR,
		.dev_params = {
			.entry_count = 48,
			.num_vmid = 0,
			.num_ssd_index_bits = 0,
			.num_stream_id_bits = 0,
			.input_addr_size = 0,
			.stream_match_support = 0,
		},
		.is_initialised = false,
	},
	{
		.vmidmt = HAL_VMIDMT_QDSS_VMIDETR,
		.base_addr = QDSS_VMIDETR_VMIDMT_SCR0_ADDR,
		.dev_params = {
			.entry_count = 2,
			.num_vmid = 0,
			.num_ssd_index_bits = 0,
			.num_stream_id_bits = 0,
			.input_addr_size = 0,
			.stream_match_support = 0,
		},
		.is_initialised = false,
	},
	{
		.vmidmt = HAL_VMIDMT_QDSS_VMIDDAP,
		.base_addr = QDSS_VMIDDAP_VMIDMT_SCR0_ADDR,
		.dev_params = {
			.entry_count = 2,
			.num_vmid = 0,
			.num_ssd_index_bits = 0,
			.num_stream_id_bits = 0,
			.input_addr_size = 0,
			.stream_match_support = 0,
		},
		.is_initialised = false,
	},
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
			{ .bit_pos = 0,  .vmidmt = HAL_VMIDMT_CRYPTO0_BAM },
			{ .bit_pos = 1,  .vmidmt = HAL_VMIDMT_RPM_MSGRAM },
			{ .bit_pos = 2,  .vmidmt = HAL_VMIDMT_QUPV3_0 },
			{ .bit_pos = 3,  .vmidmt = HAL_VMIDMT_COUNT },
			{ .bit_pos = 4,  .vmidmt = HAL_VMIDMT_QPIC_BAM },
			{ .bit_pos = 5,  .vmidmt = HAL_VMIDMT_COUNT },
			{ .bit_pos = 6,  .vmidmt = HAL_VMIDMT_COUNT },
			{ .bit_pos = 7,  .vmidmt = HAL_VMIDMT_COUNT },
			{ .bit_pos = 8,  .vmidmt = HAL_VMIDMT_COUNT },
			{ .bit_pos = 9,  .vmidmt = HAL_VMIDMT_COUNT },
			{ .bit_pos = 10, .vmidmt = HAL_VMIDMT_QDSS_VMIDETR },
			{ .bit_pos = 11, .vmidmt = HAL_VMIDMT_QDSS_VMIDDAP },
			{ .bit_pos = 12, .vmidmt = HAL_VMIDMT_COUNT },
			{ .bit_pos = 13, .vmidmt = HAL_VMIDMT_COUNT },
			{ .bit_pos = 14, .vmidmt = HAL_VMIDMT_COUNT },
			{ .bit_pos = 15, .vmidmt = HAL_VMIDMT_COUNT },
			{ .bit_pos = 16, .vmidmt = HAL_VMIDMT_COUNT },
			{ .bit_pos = 17, .vmidmt = HAL_VMIDMT_COUNT },
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
