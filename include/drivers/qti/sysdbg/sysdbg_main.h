/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_MAIN_H
#define SYSDBG_MAIN_H

#include <stdint.h>
#include <stdbool.h>
#include <sysdbg_target_config.h>
#include <drivers/qti/sysdbg/sysdbg.h>
#include <drivers/qti/sysdbg/sysdbg.h>
#include <drivers/qti/sysdbg/sysdbg_system.h>

/**
 * @file sysdbg_main.h
 * @brief Public API for System Debug Infrastructure (SDI)
 *
 * Ported from: TZ.XF.5.36-00138-SHIKRA-1
 */

/*
 * TARGET_APPS_CPUS and the clock frequencies come from the chipset's
 * sysdbg_target_config.h
 */
#define CONVERT_TO_CYCLES(time_ms, clk_freq) ((time_ms * clk_freq) / 1000)

/*
 * SDI Configuration Structure
 *
 * Ported from TZ: sysdbg/inc/sysdbg_main.h (sysdbg_config_type)
 */
typedef struct {
	uint32_t reset_dbgen;
	uint32_t wdog_dbgen;
	uint32_t tsense_dbgen;
	uint32_t pmic_resin_dbgen;
	uint32_t dbg_ctl_rst;
	uint32_t acc_ctl_rst;
	uint32_t timeout1_en;
	uint32_t timeout1;
	uint32_t wind_down_dly;
	uint32_t save_data_dly;
	uint32_t stop_trace_dly;
	uint32_t prep_ares_dly;
	bool mhm_scandumpen;
} sysdbg_config_t;

const sysdbg_config_t *sysdbg_get_config(void);
uint32_t sysdbg_get_chip_rev(void);

/*
 * Shared Data Structure for DDR Self-Refresh Coordination
 *
 * Used to pass DDR function pointers from BL31 (Monitor) to QSEE crash path
 */
typedef struct {
	uint64_t enter_ddr_self_refresh_addr;
	uint64_t mon_imem_stack_ptr;
} sysdbg_shared_data;

/**
 * system_debug_reset_entry() - Crash path entry point
 */
void system_debug_reset_entry(void);

#endif /* SYSDBG_MAIN_H */
