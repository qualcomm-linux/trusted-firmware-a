/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <lib/mmio.h>

#include <sysdbg_hwio.h>
#include <sysdbg_timer.h>

void sysdbg_enable_mpm_timer(bool enable)
{
	static bool mpm_timer_enabled;

	if (enable) {
		if (!mpm_timer_enabled) {
			mmio_setbits_32(HWIO_MPM_CONTROL_CNTCR_ADDR,
					HWIO_MPM_CONTROL_CNTCR_EN_BMSK);
			mpm_timer_enabled = true;
		}
	} else {
		mmio_clrbits_32(HWIO_MPM_CONTROL_CNTCR_ADDR, HWIO_MPM_CONTROL_CNTCR_EN_BMSK);
	}
}

/*
 * The generic delay timer (dst_repo's equivalent of source's busywait_init())
 * is already initialised platform-wide in qti_bl31_setup.c before BL31
 * reaches the reset-debug path - nothing left for this function to do.
 */
bool SysdbgTimerInit(void)
{
	return true;
}

uint64_t sysdbg_get_mpm_counter(void)
{
	return (((uint64_t)mmio_read_32(HWIO_MPM_CONTROL_CNTCV_HI_ADDR) << 32) |
		mmio_read_32(HWIO_MPM_CONTROL_CNTCV_LO_ADDR));
}

uint64_t SysdbgGetTimestamp(void)
{
	return sysdbg_get_mpm_counter();
}
