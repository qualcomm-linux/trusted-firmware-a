/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <ac_cfg.h>
#include <common/debug.h>
#include <vmidmt_cfg.h>
#include <vmidmt_hal.h>
#include <vmidmt_internal.h>

/*
 * VMIDMT configuration provider for XPU4 targets.
 *
 * XPU4 targets have no cfg/${CHIPSET} directory, so everything the VMIDMT
 * driver needs is parsed out of the access-control config image instead. The
 * XPU3 equivalent lives in cfg/${CHIPSET}/vmidmt_static_config.c.
 *
 * NOTE: the image-side layouts are assumed to match the driver structures
 * below, which is how the downstream driver consumed them. Confirm against the
 * config-image producer before relying on this on real hardware.
 */

/*
 * Interrupt descriptors, cached on first use.
 *
 * The image supplies the interrupt numbers in the shared descriptor array,
 * where slots 2 to 5 belong to VMIDMT, and the status and enable register
 * addresses in the VMIDMT interrupt tables.
 */
#define AC_INTR_DESC_VMIDMT_FIRST	2U

static struct vmidmt_intr_reg intr_reg[VMIDMT_INTR_COUNT];
static bool intr_reg_valid;

int vmidmt_cfg_get_info_array(struct hal_vmidmt_info **info, uint32_t *count)
{
	const void *data = NULL;
	uint32_t entries = 0U;
	int rc;

	rc = ac_cfg_get_vmidmt_table(AC_VMIDMT_TABLE_INFO,
				     sizeof(struct hal_vmidmt_info), &data,
				     &entries);
	if (rc != 0) {
		return rc;
	}

	/*
	 * Cast away const: the driver caches probed device parameters and the
	 * initialised flag back into each entry, as it does with the
	 * compile-time tables on XPU3.
	 */
	*info = (struct hal_vmidmt_info *)data;
	*count = entries;

	return 0;
}

int vmidmt_cfg_get_cfg_array(const struct vmidmt_cfg **cfg, uint32_t *count)
{
	const void *data = NULL;
	int rc;

	rc = ac_cfg_get_vmidmt_table(AC_VMIDMT_TABLE_CFG,
				     sizeof(struct vmidmt_cfg), &data, count);
	if (rc != 0) {
		return rc;
	}

	*cfg = data;

	return 0;
}


int vmidmt_cfg_get_err_pos_map(const struct vmidmt_err_pos_to_hal_map **map,
			       uint32_t *reg_count, uint32_t *per_reg)
{
	const void *data = NULL;
	uint32_t entries = 0U;
	int rc;

	rc = ac_cfg_get_vmidmt_table(AC_VMIDMT_TABLE_INTR_POS_MAP,
				     sizeof(struct vmidmt_err_pos_to_hal_map),
				     &data, &entries);
	if (rc != 0) {
		return rc;
	}

	/*
	 * The image stores the map flat, one entry per status-register bit, so
	 * the register count follows from the total.
	 */
	*map = data;
	*per_reg = ACC_VMIDMT_ERR_NUM_PER_REG;
	*reg_count = entries / ACC_VMIDMT_ERR_NUM_PER_REG;

	return 0;
}

int vmidmt_cfg_get_intr_reg(enum vmidmt_intr_id id,
			    const struct vmidmt_intr_reg **reg)
{
	if (id >= VMIDMT_INTR_COUNT) {
		return -1;
	}

	if (!intr_reg_valid) {
		const struct vmidmt_intr_addr *addr = NULL;
		struct ac_interrupt_desc *desc = NULL;
		uint32_t addr_count = 0U;
		uint32_t desc_count = 0U;
		uint32_t i;

		/*
		 * Interrupt numbers come from the shared descriptor array in
		 * the config image, where slots 2 to 5 belong to VMIDMT.
		 */
		if (ac_cfg_get_xpu_vmidmt_intr_info(&desc, &desc_count) != 0 ||
		    desc == NULL ||
		    desc_count < (AC_INTR_DESC_VMIDMT_FIRST +
				  VMIDMT_INTR_COUNT)) {
			ERROR("vmidmt: interrupt descriptors missing\n");
			return -1;
		}

		/*
		 * The status and enable register addresses come from the config
		 * image (AC_VMIDMT_INTR_REGS_ARRAY). The image stores them as
		 * struct vmidmt_intr_addr entries alongside the interrupt numbers
		 * so that every SoC-specific address is in one signed artifact.
		 */
		if (ac_cfg_get_vmidmt_intr_regs(&addr, &addr_count) != 0 ||
		    addr_count < VMIDMT_INTR_COUNT) {
			ERROR("vmidmt: interrupt registers unavailable\n");
			return -1;
		}

		for (i = 0U; i < VMIDMT_INTR_COUNT; i++) {
			intr_reg[i].status_addr = addr[i].status_addr;
			intr_reg[i].status_mask = addr[i].status_mask;
			intr_reg[i].enable_addr = addr[i].enable_addr;
			intr_reg[i].intr_num =
				desc[AC_INTR_DESC_VMIDMT_FIRST + i].intr_num;
		}

		intr_reg_valid = true;
	}

	*reg = &intr_reg[id];

	return 0;
}
