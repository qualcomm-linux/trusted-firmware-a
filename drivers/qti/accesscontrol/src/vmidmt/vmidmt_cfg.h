/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VMIDMT_CFG_H
#define VMIDMT_CFG_H

#include <stdint.h>

#include <vmidmt_hal.h>
#include <vmidmt_internal.h>

/*
 * VMIDMT configuration provider.
 *
 * The VMIDMT driver itself is revision-agnostic; where its configuration comes
 * from is not. Exactly one provider is linked, selected by XPU_VERSION:
 *
 *   XPU3 - cfg/${CHIPSET}/vmidmt_static_config.c returns compile-time tables
 *          and the target's TCSR register addresses.
 *   XPU4 - src/vmidmt/vmidmt_cfg_ac.c returns the same information parsed out
 *          of the access-control config image, because XPU4 targets have no
 *          cfg/${CHIPSET} directory.
 *
 * Every getter returns 0 on success and a negative value if the information is
 * unavailable.
 */

/* The four VMIDMT error interrupts, in the order the image describes them. */
enum vmidmt_intr_id {
	VMIDMT_INTR_CLT_SEC = 0,
	VMIDMT_INTR_CLT_NONSEC,
	VMIDMT_INTR_CFG_SEC,
	VMIDMT_INTR_CFG_NONSEC,
	VMIDMT_INTR_COUNT
};

/* Where one VMIDMT error interrupt is reported and how it is unmasked. */
struct vmidmt_intr_reg {
	uint32_t status_addr;
	uint32_t status_mask;
	uint32_t enable_addr;
	uint32_t intr_num;
};

/* Per-instance base addresses and probed parameters, indexed by instance id. */
int vmidmt_cfg_get_info_array(struct hal_vmidmt_info **info, uint32_t *count);

/*
 * Per-instance options: SSD table, error reporting, static-config state, secure
 * bypass VMID, and the VMID mappings belonging to that instance. The mappings
 * are reached through vmid_map/vmid_map_count on each entry rather than from a
 * separate flat table.
 */
int vmidmt_cfg_get_cfg_array(const struct vmidmt_cfg **cfg, uint32_t *count);

/*
 * Maps a bit position in an error status register back to the VMIDMT instance
 * that raised it. Returned flat: the entry for bit b of register r is at
 * index (r * per_reg + b).
 */
int vmidmt_cfg_get_err_pos_map(const struct vmidmt_err_pos_to_hal_map **map,
			       uint32_t *reg_count, uint32_t *per_reg);

/* Status and enable registers, plus the interrupt number, for one interrupt. */
int vmidmt_cfg_get_intr_reg(enum vmidmt_intr_id id,
			    const struct vmidmt_intr_reg **reg);

#endif /* VMIDMT_CFG_H */
