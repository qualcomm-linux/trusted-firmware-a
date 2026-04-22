/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <platform_def.h>

#include "smem_internal.h"
#include "smem_os.h"
#include "smem_target.h"
#include "smem_toc.h"
#include "smem_type.h"

static struct smem_targ_host_info smem_targ_host_info = { 0 };

#define TCSR_TZ_WONCE_0_ADDR	(QTI_CORE_TOP_CSR_BASE + 0x000C6000)
#define TCSR_TZ_WONCE_1_ADDR	(QTI_CORE_TOP_CSR_BASE + 0x000C6004)

const uint32_t smem_targ_hwio_wonce_regs[] = {
	TCSR_TZ_WONCE_0_ADDR,
	TCSR_TZ_WONCE_1_ADDR
};

struct smem_targ_host_info *smem_targ_init(void)
{
	struct smem_targ_host_info *host_info = &smem_targ_host_info;
	uint64_t targ_info_phy_addr;
	struct smem_targ_info *targ_info;
	uint32_t wonce_val_0, wonce_val_1;

	if (host_info->size) {
		return host_info;
	}

	wonce_val_0 = mmio_read_32(smem_targ_hwio_wonce_regs[0]);
	wonce_val_1 = mmio_read_32(smem_targ_hwio_wonce_regs[1]);

	targ_info_phy_addr = wonce_val_0 | ((uint64_t)wonce_val_1 << 32);

	/*
	 * Verify SMEM target info address is within statically mapped region.
	 */
	if (!smem_os_verify_addr_mapped((uintptr_t)targ_info_phy_addr,
					sizeof(struct smem_targ_info))) {
		ERROR("SMEM: target info addr=0x%016lX is not mapped\n",
		      targ_info_phy_addr);
		plat_error_handler(EFAULT);
		goto bail_out;
	}

	/*
	 * SMEM target info is statically mapped as part of QTI_DEVICE region.
	 * Since it's 1:1 mapping, use physical address directly as virtual address.
	 */
	targ_info = (struct smem_targ_info *)(uintptr_t)targ_info_phy_addr;

	if (targ_info->identifier != SMEM_TARG_INFO_IDENTIFIER) {
		ERROR("SMEM: target info is not present\n");
		plat_error_handler(EFAULT);
		goto bail_out;
	}

	host_info->size = targ_info->smem_size;
	host_info->phys_addr = targ_info->smem_base_phys_addr;
	host_info->max_items = targ_info->smem_max_items;
	host_info->targ_info = targ_info;

	host_info->this_host = SMEM_TZ;
	host_info->version_offset = SMEM_VERSION_TZ_HYP_OFFSET;
	host_info->targ_info_phy_addr = targ_info_phy_addr;

	return host_info;

bail_out:
	return NULL;
}
