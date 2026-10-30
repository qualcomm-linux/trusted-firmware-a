/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPRH_INTERNAL_H
#define QTI_CPRH_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	const char *clk_name;
	uint8_t thread_num;
	uint8_t dn_threshold;
	uint8_t up_threshold;
} cprh_thread_cfg_t;

typedef struct {
	const char *rail_name;
	uintptr_t base_addr;
	uint32_t fsm_ctl;
	uint32_t cprh_ctl;
	uint32_t timer_auto_cont;
	uint32_t step_quot_init;
	uint32_t timer_clamp;
	uint32_t saw_error_step_limit;
	uint32_t misc_register;
	uint32_t margin_adj_ctl;
	uint32_t margin_temp_core_timers;
	uint8_t sensor_count;
	uint32_t sensor_thread_mask;
	uint32_t sensor_thread_mask_thread2;
	uint32_t sensor_mask;
	uint32_t sensor_bypass_mask;
	uint8_t thread_count;
	const cprh_thread_cfg_t *threads;
} cprh_controller_cfg_t;

extern const cprh_controller_cfg_t qti_cprh_controller;

#endif /* QTI_CPRH_INTERNAL_H */
