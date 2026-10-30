/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../cprh_internal.h"

static const cprh_thread_cfg_t shikra_cprh_threads[] = {
	{
		.clk_name = "Silver",
		.thread_num = 0,
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

const cprh_controller_cfg_t qti_cprh_controller = {
	.rail_name = "APC",
	.base_addr = 0x0FBA0000UL,
	.fsm_ctl = 0x00002814,
	.cprh_ctl = 0x080000C8,
	.timer_auto_cont = 0x17700,
	.step_quot_init = 0x1C7,
	.timer_clamp = 0x08000000,
	.saw_error_step_limit = 0x21,
	.misc_register = 0x61000004,
	.margin_adj_ctl = 0x01024000,
	.margin_temp_core_timers = 0x005CCB00,
	.sensor_count = 10,
	.sensor_thread_mask = 0x0F,
	.sensor_thread_mask_thread2 = 0x380,
	.sensor_mask = 0x380,
	.sensor_bypass_mask = 0,
	.thread_count = 3,
	.threads = shikra_cprh_threads,
};
