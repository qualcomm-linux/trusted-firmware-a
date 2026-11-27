/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Shikra Target Configuration Data for System Debug Image (SDI)
 *
 * Ported from TZ.XF.5.36-00138-SHIKRA-1:
 * trustzone_images/core/systemdebug/sysdbg/qsee/src/target/shikra/sysdbg_target_data.c
 *
 * This file defines the target-specific configuration for Shikra SDI cold boot,
 * including debug enable flags and timer values.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <drivers/qti/sysdbg/sysdbg_main.h>

/*===========================================================================
 * SDI Configuration Data for Shikra
 *===========================================================================
 */

/*
 * shikra_sysdbg_config - System Debug Configuration for Shikra
 *
 * Debug enable flags:
 *   reset_dbgen      - Global reset debug enable (1 = enabled)
 *   wdog_dbgen       - Watchdog reset debug (1 = enabled)
 *   tsense_dbgen     - Temperature sensor reset debug (1 = enabled)
 *   pmic_resin_dbgen - PMIC RESIN reset debug (1 = enabled)
 *   dbg_ctl_rst      - Debug control reset (0 = disabled)
 *   acc_ctl_rst      - Access control reset (0 = disabled)
 *
 * Timeout configuration:
 *   timeout1_en      - First pass timeout enable (1 = enabled)
 *   timeout1         - First pass timeout value in milliseconds (20000 = 20 seconds)
 *                      NOTE: Set to 0 to debug SDI path without timeout
 *
 * Timer delays (in milliseconds):
 *   wind_down_dly    - Hydra wind down timer delay (50 ms)
 *   save_data_dly    - Hydra save contents and flush ETR timer (8 ms)
 *   stop_trace_dly   - Stop ETR/ETB timer (8 ms)
 *   prep_ares_dly    - Pre-ARES preparation phase timer (30 ms)
 *
 * Feature flags:
 *   mhm_scandumpen   - MHM scandump enable (true = enabled)
 */
static const sysdbg_config_t shikra_sysdbg_config = {
	/* Debug enable flags */
	.reset_dbgen      = 1,
	.wdog_dbgen       = 1,
	.tsense_dbgen     = 1,
	.pmic_resin_dbgen = 1,
	.dbg_ctl_rst      = 0,
	.acc_ctl_rst      = 0,

	/* First pass timeout configuration */
	.timeout1_en      = 1,
	.timeout1         = 20000,  /* 20 seconds */

	/* Timer delays for reset debug phases (milliseconds) */
	.wind_down_dly    = 50,     /* Wind down phase */
	.save_data_dly    = 8,      /* Save and flush trace data */
	.stop_trace_dly   = 8,      /* Stop trace capture */
	.prep_ares_dly    = 30,     /* Prepare for reset */

	/* MHM scandump feature */
	.mhm_scandumpen   = true,
};

/*===========================================================================
 * Public API Functions
 *===========================================================================
 */

/**
 * sysdbg_get_config - Get SDI configuration for Shikra
 *
 * Returns pointer to the static configuration structure.
 * This configuration is used by SDI during cold boot to control
 * reset debug features and timing parameters.
 *
 * Return: Pointer to sysdbg_config_t structure
 */
const sysdbg_config_t *sysdbg_get_config(void)
{
	return &shikra_sysdbg_config;
}
