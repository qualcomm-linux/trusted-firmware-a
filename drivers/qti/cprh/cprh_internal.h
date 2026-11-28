/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Internal CPRH types, shared between the generic boot-init/HAL code
 * (common/) and shikra's register map/device config (shikra/).
 */

#ifndef CPRH_INTERNAL_H
#define CPRH_INTERNAL_H

#include <stdint.h>

#include <common/debug.h>

#define CPRH_CORE_VERIFY(x)					\
	do {							\
		if ((x) == 0) {				\
			ERROR("cprh: invariant failed: %s\n", #x); \
			panic();				\
		}						\
	} while (0)

/* SAW4 AVS ramp/step configuration for a controller's rail. */
typedef struct {
	uint8_t tmr_clk_div;
	uint8_t vlvl_width;
	uint8_t vlvl_step_up;
	uint8_t vlvl_step_dn;
	uint8_t ramp_en;
} cprh_saw_context;

/* Per-virtual-corner CPRH register programming. */
typedef struct {
	uint8_t vc;
	uint8_t vc_band;
	uint16_t ceil_voltage;
	uint16_t floor_voltage;
	uint16_t target_quot;
	uint16_t target_delta;
	uint8_t ro;
	uint8_t close_loop_disable;
	uint8_t core_temp_margin_disable;
	uint8_t voltage_cache_disable;
} cprh_virtual_corner_context;

/* CPR thread (one per clock domain monitored by a controller). */
typedef struct {
	const char *clk_name;
	uint8_t thread_num;
	uint8_t dn_threshold;
	uint8_t up_threshold;
	uint8_t consecutive_dn;
	uint8_t consecutive_up;
} cprh_thread_context;

/* CPR controller (rail) context. */
typedef struct {
	const char *rail_name;
	uintptr_t base_addr;
	uint8_t controller_num;
	uint8_t num_threads;
	uint8_t count_mode;
	uint8_t count_repeat;
	uint8_t idle_clocks;
	uint8_t step_quot_max;
	uint8_t step_quot_min;
	uint8_t reset_step_quot_loop_en;
	uint8_t sensor_count;
	uint32_t sensor_thread_mask;
	uint32_t sensor_mask;
	uint32_t sensor_thread_mask_thread2;
	uint32_t sensor_bypass;
	uint32_t auto_cont_interval;
	uint32_t base_voltage_mV;
	uint8_t voltage_multiplier;
	uint8_t target_multiplier;
	uint8_t initial_mode;
	uint8_t temp_sensor_id_start;
	uint8_t temp_sensor_id_end;
	uint8_t error_step_limit_dn;
	uint8_t error_step_limit_up;
	uint8_t thread_aggr_en;
	uint8_t thread_has_always_vote;
	uint32_t timer_settle_voltage;
	uint32_t timer_lower_voltage;
	uint8_t initial_temp_band;
	uint8_t max_num_cores;
	uint8_t lower_voltage_wait_select;
	uint8_t pmic_step_size;
	uint8_t close_loop_en;
	uint8_t core_adj_en;
	uint8_t temp_adj_en;
	uint32_t thread_disable_mask;
	uint32_t cpr_bist_chain_check;
	uint32_t kv_margin_adj_single_step_cnt;
	uint32_t cpr_misc_reg;
	uint32_t cpr_avg_abort_autoconttmr;
	uint32_t cpr_acd_dcvs_disable_config;
	cprh_thread_context *thread_context;
} cprh_controller_context;

#endif /* CPRH_INTERNAL_H */
