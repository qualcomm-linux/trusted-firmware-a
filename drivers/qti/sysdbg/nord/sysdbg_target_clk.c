/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <sysdbg_clock.h>
#include <sysdbg_hwio.h>

/*
 * Statically defined structure with clock branches to be turned on. Empty
 * on hawi/hamoa/kaanapali and on nord's own TZ build (sysdbg_target_clk.c)
 * - ported as-is, not a nord-specific omission.
 */
static const cbcr_type target_clock_branches[] = {
};

bool SysdbgClockInit(void)
{
	static bool initialized;
	uint32_t i;

	if (!initialized) {
		/* Votable clocks. */
		mmio_setbits_32(HWIO_GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_ADDR,
				 HWIO_GCC_APCS_TZ_CLOCK_BRANCH_ENA_VOTE_QDSS_CFG_AHB_CLK_ENA_BMSK);

		for (i = 0; i < ARRAY_SIZE(target_clock_branches); i++) {
			mmio_setbits_32(target_clock_branches[i], 0x1);
		}

		initialized = true;
	}

	return true;
}
