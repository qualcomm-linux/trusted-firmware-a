/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/mmio.h>

#include "cprh_hal.h"
#include "cprh_hwio.h"
#include "../cprh_hal_internal.h"

void cprh_hal_create_controller(cprh_controller_context *controller_context)
{
	uintptr_t base = controller_context->base_addr;
	uint32_t thread_disable_mask;
	uint8_t idx;

	/* Registers required for minimal CPR operation. */
	CPR_HWIO_OUTM(base, CPR_FSM_CTL, LOOP_EN, 0U);
	CPR_HWIO_OUTM(base, CPR_FSM_CTL, COUNT_MODE, controller_context->count_mode);
	CPR_HWIO_OUTM(base, CPR_FSM_CTL, COUNT_REPEAT, controller_context->count_repeat);
	CPR_HWIO_OUTM(base, CPR_FSM_CTL, IDLE_CLOCKS, controller_context->idle_clocks);
	CPR_HWIO_OUTM(base, CPRH_CTL, INITIAL_MODE, controller_context->initial_mode);
	CPR_HWIO_OUTM(base, CPRH_CTL, BASE_VOLTAGE,
		      controller_context->base_voltage_mV / controller_context->voltage_multiplier);
	CPR_HWIO_OUTM(base, CPRH_CTL, VOLTAGE_MULTIPLIER, controller_context->voltage_multiplier);
	CPR_HWIO_OUTM(base, CPR_TIMER_AUTO_CONT, AUTO_CONT_INTERVAL, controller_context->auto_cont_interval);
	CPR_HWIO_OUTM(base, CPR_STEP_QUOT_INIT, STEP_QUOT_INIT_MAX, controller_context->step_quot_max);
	CPR_HWIO_OUTM(base, CPR_STEP_QUOT_INIT, STEP_QUOT_INIT_MIN, controller_context->step_quot_min);
	CPR_HWIO_OUTM(base, CPR_TIMER_CLAMP, THREAD_AGGREGATION_EN, controller_context->thread_aggr_en);
	CPR_HWIO_OUTM(base, CPR_SAW_ERROR_STEP_LIMIT, DN_ERROR_STEP_LIMIT, controller_context->error_step_limit_dn);
	CPR_HWIO_OUTM(base, CPR_SAW_ERROR_STEP_LIMIT, UP_ERROR_STEP_LIMIT, controller_context->error_step_limit_up);
	CPR_HWIO_OUTM(base, CPR_MISC_REGISTER, RESET_STEP_QUOT_LOOP_EN, controller_context->reset_step_quot_loop_en);
	CPR_HWIO_OUTI(base, CPR_SENSOR_MASK_WRITEn, 0, controller_context->sensor_mask);
	CPR_HWIO_OUTI(base, CPR_SENSOR_BYPASS_WRITEn, 0, controller_context->sensor_bypass);

	/* Registers required for per core/temperature adjustment. */
	CPR_HWIO_OUTM(base, CPR_MISC_REGISTER, TEMP_SENSOR_ID_START, controller_context->temp_sensor_id_start);
	CPR_HWIO_OUTM(base, CPR_MISC_REGISTER, TEMP_SENSOR_ID_END, controller_context->temp_sensor_id_end);
	CPR_HWIO_OUTM(base, CPR_MISC_REGISTER, THREAD_HAS_ALWAYS_VOTE, controller_context->thread_has_always_vote);
	CPR_HWIO_OUTM(base, CPR_MARGIN_ADJ_CTL, CORE_ADJ_EN, controller_context->core_adj_en);
	CPR_HWIO_OUTM(base, CPR_MARGIN_ADJ_CTL, TEMP_ADJ_EN, controller_context->temp_adj_en);
	CPR_HWIO_OUTM(base, CPR_MARGIN_ADJ_CTL, KV_MARGIN_ADJ_EN,
		      (controller_context->core_adj_en | controller_context->temp_adj_en));
	CPR_HWIO_OUTM(base, CPR_MARGIN_ADJ_CTL, INITIAL_TEMP_BAND, controller_context->initial_temp_band);
	CPR_HWIO_OUTM(base, CPR_MARGIN_ADJ_CTL, LOWER_VOLTAGE_WAIT_SELECT, controller_context->lower_voltage_wait_select);
	CPR_HWIO_OUTM(base, CPR_MARGIN_ADJ_CTL, PMIC_STEP_SIZE, controller_context->pmic_step_size);
	CPR_HWIO_OUTM(base, CPR_MARGIN_ADJ_CTL, CLOSED_LOOP_EN, controller_context->close_loop_en);
	CPR_HWIO_OUTM(base, CPR_MARGIN_ADJ_CTL, MAX_NUM_CORES, controller_context->max_num_cores);
	CPR_HWIO_OUTM(base, CPR_MARGIN_TEMP_CORE_TIMERS, TIMER_SETTLE_VOLTAGE_COUNT, controller_context->timer_settle_voltage);
	CPR_HWIO_OUTM(base, CPR_MARGIN_TEMP_CORE_TIMERS, TIMER_LOWER_VOLTAGE_COUNT, controller_context->timer_lower_voltage);

	/* Assign sensor to a thread. */
	if (controller_context->sensor_thread_mask != 0U) {
		for (idx = 0; idx < controller_context->sensor_count; idx++) {
			CPR_HWIO_OUTI(base, CPR_SENSOR_THREAD_n, idx,
				      (controller_context->sensor_thread_mask >> idx) & 0x1U);
		}
	}

	/* Write 0x2 only where the thread2 mask bits are set. */
	if (controller_context->sensor_thread_mask_thread2 != 0U) {
		for (idx = 0; idx < controller_context->sensor_count; idx++) {
			if (((controller_context->sensor_thread_mask_thread2 >> idx) & 0x1U) != 0U) {
				CPR_HWIO_OUTI(base, CPR_SENSOR_THREAD_n, idx, 0x2U);
			}
		}
	}

	CPR_HWIO_OUTM(base, CPR_MARGIN_ADJ_CTL, KV_MARGIN_ADJ_SINGLE_STEP_QUOT,
		      controller_context->kv_margin_adj_single_step_cnt);
	CPR_HWIO_OUTM(base, CPR_BIST_CHAIN_CHECK0, SCLK_CNT_EXPECTED, controller_context->cpr_bist_chain_check);
	CPR_HWIO_OUT(base, CPRH_MISC_REGISTER2, controller_context->cpr_misc_reg);
	CPR_HWIO_OUTM(base, CPRH_DCVS_SETTLE_TIMER_CONFIG, AVG_ABORT_AUTOCONTTIMER,
		      controller_context->cpr_avg_abort_autoconttmr);
	CPR_HWIO_OUT(base, CPRH_ACD_DCVS_DISABLE_CONFIG, controller_context->cpr_acd_dcvs_disable_config);

	/* Disable threads named in the mask. */
	thread_disable_mask = controller_context->thread_disable_mask;
	idx = 0;
	while (thread_disable_mask != 0U) {
		CPR_HWIO_OUTMI(base, CPR_MASK_THREAD_t, idx, DISABLE_THREAD, thread_disable_mask & 0x1U);
		thread_disable_mask >>= 1;
		idx++;
	}
}

void cprh_hal_create_thread(cprh_controller_context *controller_context,
			     cprh_thread_context *thread_context)
{
	uintptr_t base = controller_context->base_addr;

	CPR_HWIO_OUTMI(base, CPR_THRESHOLD_t, thread_context->thread_num,
		       DN_THRESHOLD, thread_context->dn_threshold);
	CPR_HWIO_OUTMI(base, CPR_THRESHOLD_t, thread_context->thread_num,
		       UP_THRESHOLD, thread_context->up_threshold);
	CPR_HWIO_OUTMI(base, CPR_THRESHOLD_t, thread_context->thread_num,
		       CONSECUTIVE_DN, thread_context->consecutive_dn);
	CPR_HWIO_OUTMI(base, CPR_THRESHOLD_t, thread_context->thread_num,
		       CONSECUTIVE_UP, thread_context->consecutive_up);
}

uint32_t cprh_hal_get_target(cprh_controller_context *controller_context,
			      uint8_t cprh_thread, uint8_t ro)
{
	return mmio_read_32(HWIO_CPR_TARGET0_t_m_ADDR(controller_context->base_addr, cprh_thread, 0) +
			     ((uint32_t)ro * 4U));
}

void cprh_hal_config_target(cprh_controller_context *controller_context,
			     uint8_t cprh_thread, uint8_t ro, uint32_t target)
{
	mmio_write_32(HWIO_CPR_TARGET0_t_m_ADDR(controller_context->base_addr, cprh_thread, 0) +
		      ((uint32_t)ro * 4U), target);
}

void cprh_hal_config_gcnt(cprh_controller_context *controller_context,
			   uint8_t ro, uint32_t gcnt)
{
	mmio_write_32(HWIO_CPR_GCNT0_ADDR(controller_context->base_addr, ro),
		      gcnt & HWIO_CPR_GCNT0_GCNT0_BMSK);
}

void cprh_hal_config_virtual_corner(cprh_controller_context *controller_context,
				     uint8_t cprh_thread,
				     cprh_virtual_corner_context *vc_context)
{
	uintptr_t base = controller_context->base_addr;

	CPR_HWIO_OUTMI2(base, CPRH_VIRTUAL_CORNER_t_v, cprh_thread, vc_context->vc,
			INITIAL_VOLTAGE, vc_context->ceil_voltage);
	CPR_HWIO_OUTMI2(base, CPRH_VIRTUAL_CORNER_t_v, cprh_thread, vc_context->vc,
			FLOOR_VOLTAGE, vc_context->floor_voltage);
	CPR_HWIO_OUTMI2(base, CPRH_VIRTUAL_CORNER_t_v, cprh_thread, vc_context->vc,
			RO_SELECT, vc_context->ro);
	CPR_HWIO_OUTMI2(base, CPRH_VIRTUAL_CORNER_t_v, cprh_thread, vc_context->vc,
			CPR_CL_DISABLE, vc_context->close_loop_disable);
	CPR_HWIO_OUTMI2(base, CPRH_VIRTUAL_CORNER_t_v, cprh_thread, vc_context->vc,
			CORE_TEMP_MARGIN_DISABLE, vc_context->core_temp_margin_disable);
	CPR_HWIO_OUTMI2(base, CPRH_VIRTUAL_CORNER_t_v, cprh_thread, vc_context->vc,
			LAST_GOOD_KNOWN_VOLTAGE_EN, !vc_context->voltage_cache_disable);
	CPR_HWIO_OUTMI2(base, CPRH_VIRTUAL_CORNER_t_v, cprh_thread, vc_context->vc,
			TARGET_DELTA, vc_context->target_delta);
}

uint32_t cprh_hal_get_base_voltage(cprh_controller_context *controller_context)
{
	return CPR_HWIO_INM(controller_context->base_addr, CPRH_CTL, BASE_VOLTAGE);
}

void cprh_hal_enable_dcvsh(cprh_controller_context *controller_context, uint8_t enable)
{
	CPR_HWIO_OUTM(controller_context->base_addr, CPRH_CTL, DCVSH_ENABLED, enable);
}

void cprh_hal_apm_threshold_set(cprh_controller_context *controller_context,
				 uint16_t apm_threshold)
{
	CPR_HWIO_OUTM(controller_context->base_addr, CPRH_MISC_REGISTER2, APM_THRESHOLD, apm_threshold);
}

void cprh_hal_apm_enable(cprh_controller_context *controller_context, uint8_t enable)
{
	CPR_HWIO_OUTM(controller_context->base_addr, CPRH_MISC_REGISTER2, APM_INTERFACE_EN, enable);
}

void cprh_hal_enable_measurement(cprh_controller_context *controller_context, uint8_t enable)
{
	CPR_HWIO_OUTM(controller_context->base_addr, CPR_FSM_CTL, LOOP_EN, enable);
}
