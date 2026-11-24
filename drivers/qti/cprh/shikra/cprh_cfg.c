/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../cprh_internal.h"
#include "cprh_cfg.h"

/* Unspecified fields use C's static zero-initialization. */
static cprh_thread_context cprh_pwr_threads[] = {
	{
		.clk_name = "Silver",
		.dn_threshold = 2,
		.up_threshold = 2,
	},
	{
		.clk_name = "L3",
		.thread_num = 1,
		.dn_threshold = 2,
		.up_threshold = 2,
	},
	{
		.clk_name = "Gold",
		.thread_num = 2,
		.dn_threshold = 2,
		.up_threshold = 2,
	},
};

static cprh_saw_context cprh_pwr_saw = {
	.vlvl_width = 1,
	.vlvl_step_up = 1,
	.vlvl_step_dn = 1,
	.ramp_en = 1,
};

static cprh_controller_context cprh_pwr = {
	.rail_name = "APC",
	.base_addr = 0x0fba0000UL,
	.num_threads = 3,
	.count_repeat = 20,
	.idle_clocks = 10,
	.step_quot_max = 7,
	.step_quot_min = 7,
	.reset_step_quot_loop_en = 1,
	.sensor_count = 10,
	.sensor_thread_mask = 0x0F,
	.sensor_thread_mask_thread2 = 0x380,
	.sensor_mask = 0x380,
	.auto_cont_interval = 0x17700,
	.base_voltage_mV = 400,
	.voltage_multiplier = 4,
	.target_multiplier = 4,
	.temp_sensor_id_start = 1,
	.temp_sensor_id_end = 6,
	.error_step_limit_dn = 1,
	.error_step_limit_up = 1,
	.thread_aggr_en = 1,
	.timer_settle_voltage = 23,
	.timer_lower_voltage = 0x4B00,
	.max_num_cores = 4,
	.lower_voltage_wait_select = 1,
	.pmic_step_size = 4,
	.close_loop_en = 1,
	.temp_adj_en = 1,
	.cpr_bist_chain_check = 0x42,
	.kv_margin_adj_single_step_cnt = 5,
	.cpr_misc_reg = 0xFFF52AF0,
	.cpr_avg_abort_autoconttmr = 1,
	.cpr_acd_dcvs_disable_config = 0x1e000,
	.thread_context = cprh_pwr_threads,
};

cprh_controller_context *cprh_context[CPRH_MAX_CONTROLLERS];
cprh_saw_context *saw_context[CPRH_MAX_CONTROLLERS];

void cprh_init_config(void)
{
	cprh_context[0] = &cprh_pwr;
	saw_context[0] = &cprh_pwr_saw;
}
