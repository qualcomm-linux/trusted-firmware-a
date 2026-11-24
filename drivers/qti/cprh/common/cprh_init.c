/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/qti/cprh/cprh.h>
#include <drivers/qti/saw/saw.h>
#include <drivers/qti/vpp/vpp.h>

#include "../cprh_internal.h"
#include "cprh_cfg.h"
#include "cprh_hal.h"

static uint32_t cprh_round_up(uint32_t val, uint32_t div)
{
	if ((val % div) == 0U) {
		return val;
	}

	return val + div - (val % div);
}

static void cprh_init_base_target(cprh_controller_context *controller_context,
				   cprh_thread_context *thread_context)
{
	const vpp_t *vpp;
	uint32_t base_target;
	uint32_t idx;

	vpp = vpp_get_plan(controller_context->rail_name, thread_context->clk_name);
	CPRH_CORE_VERIFY(vpp != NULL);

	for (idx = 0; idx < vpp->cnt; idx++) {
		/* Base target is the lowest target quotient required by a frequency. */
		base_target = cprh_hal_get_target(controller_context, thread_context->thread_num,
						   vpp->cprh[idx].ro);

		if (base_target == 0U || base_target > vpp->cprh[idx].target_quot) {
			cprh_hal_config_target(controller_context, thread_context->thread_num,
						vpp->cprh[idx].ro, vpp->cprh[idx].target_quot);
		}

		/* Multiple gcnt can be programmed. */
		cprh_hal_config_gcnt(controller_context, vpp->cprh[idx].ro,
				     CPRH_REFCLK_RATE_KHZ / 1000U);

		/* Ensure floor voltage is larger than the base voltage. */
		CPRH_CORE_VERIFY((cprh_hal_get_base_voltage(controller_context) *
				  controller_context->voltage_multiplier) < vpp->cprh[idx].floor_mV);
	}
}

static void cprh_init_crossover_virtual_corner(cprh_controller_context *controller_context,
						cprh_thread_context *thread_context)
{
	cprh_virtual_corner_context vc = {
		.core_temp_margin_disable = 1U,
		.voltage_cache_disable = 1U,
	};
	uint16_t threshold_mV;
	uint8_t required;

	vpp_get_apm_vc(controller_context->rail_name, thread_context->clk_name, &vc.vc,
		       &threshold_mV, &vc.ceil_voltage, &required);

	if (required != 0U) {
		vc.ceil_voltage = vc.ceil_voltage / controller_context->voltage_multiplier -
				   (uint16_t)cprh_hal_get_base_voltage(controller_context);
		vc.floor_voltage = vc.ceil_voltage;
		cprh_hal_config_virtual_corner(controller_context, thread_context->thread_num, &vc);
		if (required == APM_AUTO_MODE) {
			cprh_hal_apm_threshold_set(controller_context, threshold_mV);
			cprh_hal_apm_enable(controller_context, 1);
		}
	}
}

static void cprh_init_virtual_corner(cprh_controller_context *controller_context,
				      cprh_thread_context *thread_context)
{
	cprh_virtual_corner_context vc = {
		.core_temp_margin_disable = 1U,
		.voltage_cache_disable = 1U,
	};
	const vpp_t *vpp;
	uint32_t base_target;
	uint32_t target_delta;
	uint32_t idx;

	vpp = vpp_get_plan(controller_context->rail_name, thread_context->clk_name);
	CPRH_CORE_VERIFY(vpp != NULL);

	for (idx = 0; idx < vpp->cnt; idx++) {
		/* Update target delta. */
		base_target = cprh_hal_get_target(controller_context, thread_context->thread_num,
						   vpp->cprh[idx].ro);

		target_delta = cprh_round_up(vpp->cprh[idx].target_quot - base_target,
					      controller_context->target_multiplier) /
			       controller_context->target_multiplier;

		/* Validate target delta fits in 9 bits. */
		CPRH_CORE_VERIFY(target_delta < (1U << 9));

		vc.vc = vpp->cprh[idx].vc;
		vc.vc_band = vpp->cprh[idx].band;
		vc.ceil_voltage = vpp->cprh[idx].ceil_mV;
		vc.floor_voltage = vpp->cprh[idx].floor_mV;
		vc.target_quot = vpp->cprh[idx].target_quot;
		vc.target_delta = (uint16_t)target_delta;
		vc.ro = vpp->cprh[idx].ro;

		/* Update floor/ceil voltage based on the base voltage and voltage multiplier. */
		vc.ceil_voltage = vc.ceil_voltage / controller_context->voltage_multiplier -
				   cprh_hal_get_base_voltage(controller_context);
		vc.floor_voltage = vc.floor_voltage / controller_context->voltage_multiplier -
				    cprh_hal_get_base_voltage(controller_context);

		/* Validate voltage being programmed fits in 8 bits. */
		CPRH_CORE_VERIFY(vc.ceil_voltage < (1U << 8));
		CPRH_CORE_VERIFY(vc.floor_voltage < (1U << 8));

		cprh_hal_config_virtual_corner(controller_context, thread_context->thread_num, &vc);
	}
}

static void cprh_saw_enable(cprh_controller_context *controller_context)
{
	cprh_saw_context *saw = saw_context[controller_context->controller_num];

	saw_avs_ramp_en(saw->ramp_en);
	saw_avs_set_clock_divider(saw->tmr_clk_div);
	saw_avs_set_voltage_step_size(saw->vlvl_step_up, saw->vlvl_step_dn);
	saw_avs_set_voltage_width(saw->vlvl_width);
	saw_avs_enable(1);
}

static void cprh_create_controller(cprh_controller_context *controller_context)
{
	uint8_t idx;

	cprh_hal_create_controller(controller_context);

	for (idx = 0; idx < controller_context->num_threads; idx++) {
		cprh_hal_create_thread(controller_context, &controller_context->thread_context[idx]);
		cprh_init_base_target(controller_context, &controller_context->thread_context[idx]);
		cprh_init_virtual_corner(controller_context, &controller_context->thread_context[idx]);
		cprh_init_crossover_virtual_corner(controller_context,
						    &controller_context->thread_context[idx]);
	}

	cprh_saw_enable(controller_context);

	cprh_hal_enable_dcvsh(controller_context, 1);

	cprh_hal_enable_measurement(controller_context, 1);
}

void cprh_init(void)
{
	uint8_t idx;

	cprh_init_config();

	for (idx = 0; idx < CPRH_MAX_CONTROLLERS; idx++) {
		cprh_create_controller(cprh_context[idx]);
	}
}
