/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VMIDMT_INTERNAL_H
#define VMIDMT_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include <lib/cassert.h>
#include <lib/utils_def.h>
#include <vmidmt_hal.h>

#define ACC_VMIDMT_ERR_INT_STATUS_REG_NUM 1
#define ACC_VMIDMT_ERR_NUM_PER_REG 32

#define VMIDMT_ERR_OPT                                                  \
	(HAL_VMIDMT_ERROR_O_SMCFCFG_EN | HAL_VMIDMT_ERROR_O_USFCFG_EN | \
	 HAL_VMIDMT_ERROR_O_GCFGFIE | HAL_VMIDMT_ERROR_O_GCFGFRE |      \
	 HAL_VMIDMT_ERROR_O_GFIE)

/* Default MemType when not overridden (implementation defined). */
#define ACC_VMIDMT_MEMTYPE_DEFAULT 0xFF
#define ACC_VMIDMT_MEMTYPE_STRONGLY_ORDERED 0

/*
 * Values of struct vmidmt_cfg::static_cfg. This is a tri-state, not a boolean:
 * an instance configured by XBL must not have its tables reprogrammed here, or
 * the VMID, SMR and SSD state XBL already established is lost.
 */
#define ACC_VMIDMT_STATIC_CONFIG_NONE	0U
#define ACC_VMIDMT_STATIC_CONFIG_TZ	1U
#define ACC_VMIDMT_STATIC_CONFIG_XBL	2U

/*
 * Bits in struct vmidmt_map::flags. Note this is a bitmask while
 * vmidmt_cfg::static_cfg above is a tri-state; the two are encoded differently.
 */
#define ACC_VMIDMT_FLAG_STATIC_CONFIG	BIT(0)
#define ACC_VMIDMT_FLAG_SCFG_FOR_SID	BIT(1)

/*
 * The structures below are a wire format shared with the access-control config
 * image producer, so field order, widths and the resulting padding must match
 * it exactly. The CASSERTs at the end of this file pin the layout.
 */

struct vmidmt_err_pos_to_hal_map {
	uint8_t bit_pos; /* Bit position in status register */
	uint8_t vmidmt; /* Corresponding HAL VMIDMT index */
};

/* Stream-ID to VMID mapping for one master index. */
struct vmidmt_map {
	uint32_t sid_list[5]; /* SID list */
	uint8_t num_sids; /* Number of SIDs */
	uint8_t index; /* Master index before VMID mapping */
	uint8_t vmid; /* VMID to assign */
	uint8_t flags; /* ACC_VMIDMT_FLAG_* bits */
	uint8_t memattr; /* Non-default memory attribute override */
};

/* Options for one VMIDMT instance, and the VMID mappings that belong to it. */
struct vmidmt_cfg {
	hal_vmidmt_secure_status_det *ssd_table; /* Pointer to SSD table */
	uint8_t ssd_table_num_elements; /* Number of SSD elements */
	uint8_t table; /* VMIDMT master port ID */
	uint8_t err_opt; /* Error reporting options */
	uint8_t static_cfg; /* ACC_VMIDMT_STATIC_CONFIG_* */
	uint8_t secure_bypass_vmid; /* VMID ejected for secure bypass SIDs */
	struct vmidmt_map *vmid_map; /* Mappings for this instance */
	uint16_t vmid_map_count; /* Number of mappings */
};

/*
 * Layout assertions against the downstream definitions in ACVmidmtInternal.h
 * (ACVmidMTMap, ACVmidMTCfg) and HALvmidmt.h (ACVmidMTInfo). If one of these
 * fires, the driver and the config-image producer no longer agree.
 */
CASSERT(sizeof(struct vmidmt_err_pos_to_hal_map) == 2,
	assert_vmidmt_err_pos_to_hal_map_size);

CASSERT(sizeof(struct vmidmt_map) == 28, assert_vmidmt_map_size);
CASSERT(offsetof(struct vmidmt_map, sid_list) == 0, assert_vmidmt_map_sid_list);
CASSERT(offsetof(struct vmidmt_map, num_sids) == 20, assert_vmidmt_map_num_sids);
CASSERT(offsetof(struct vmidmt_map, index) == 21, assert_vmidmt_map_index);
CASSERT(offsetof(struct vmidmt_map, vmid) == 22, assert_vmidmt_map_vmid);
CASSERT(offsetof(struct vmidmt_map, flags) == 23, assert_vmidmt_map_flags);
CASSERT(offsetof(struct vmidmt_map, memattr) == 24, assert_vmidmt_map_memattr);

CASSERT(sizeof(struct vmidmt_cfg) == 32, assert_vmidmt_cfg_size);
CASSERT(offsetof(struct vmidmt_cfg, ssd_table) == 0, assert_vmidmt_cfg_ssd);
CASSERT(offsetof(struct vmidmt_cfg, ssd_table_num_elements) == 8,
	assert_vmidmt_cfg_ssd_count);
CASSERT(offsetof(struct vmidmt_cfg, table) == 9, assert_vmidmt_cfg_table);
CASSERT(offsetof(struct vmidmt_cfg, err_opt) == 10, assert_vmidmt_cfg_err_opt);
CASSERT(offsetof(struct vmidmt_cfg, static_cfg) == 11,
	assert_vmidmt_cfg_static);
CASSERT(offsetof(struct vmidmt_cfg, secure_bypass_vmid) == 12,
	assert_vmidmt_cfg_bypass);
CASSERT(offsetof(struct vmidmt_cfg, vmid_map) == 16, assert_vmidmt_cfg_map);
CASSERT(offsetof(struct vmidmt_cfg, vmid_map_count) == 24,
	assert_vmidmt_cfg_map_count);

CASSERT(sizeof(struct hal_vmidmt_int_vmidmt_dev_params) == 8,
	assert_vmidmt_dev_params_size);

CASSERT(sizeof(struct hal_vmidmt_info) == 32, assert_vmidmt_info_size);
CASSERT(offsetof(struct hal_vmidmt_info, vmidmt) == 0,
	assert_vmidmt_info_vmidmt);
CASSERT(offsetof(struct hal_vmidmt_info, base_addr) == 8,
	assert_vmidmt_info_base_addr);
CASSERT(offsetof(struct hal_vmidmt_info, dev_params) == 16,
	assert_vmidmt_info_dev_params);
CASSERT(offsetof(struct hal_vmidmt_info, is_initialised) == 24,
	assert_vmidmt_info_is_initialised);

#endif /* VMIDMT_INTERNAL_H */
