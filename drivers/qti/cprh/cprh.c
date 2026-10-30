/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/qti/cprh/cprh.h>
#include <drivers/qti/vpp/vpp.h>
#include <lib/mmio.h>

#include "cprh_internal.h"

#define CPR_FSM_CTL				0x0004U
#define CPR_TIMER_AUTO_CONT			0x000CU
#define CPR_TIMER_CLAMP			0x0010U
#define CPR_STEP_QUOT_INIT			0x0014U
#define CPR_GCNT0				0x00A0U
#define CPR_SENSOR_BYPASS_WRITE0		0x00E0U
#define CPR_SENSOR_MASK_WRITE0			0x0120U
#define CPR_SENSOR_THREAD(n)			(0x0200U + (0x4U * (n)))
#define CPR_MISC_REGISTER			0x0700U
#define CPR_SAW_ERROR_STEP_LIMIT		0x07A4U
#define CPR_MARGIN_TEMP_CORE_TIMERS		0x07A8U
#define CPR_MARGIN_ADJ_CTL			0x07F8U
#define CPR_THRESHOLD(t)			(0x0808U + (0x440U * (t)))
#define CPR_TARGET0(t, ro)			(0x0840U + (0x440U * (t)) + \
						 (0x4U * (ro)))
#define CPRH_VIRTUAL_CORNER(t, vc)		(0x3500U + (0xA0U * (t)) + \
						 (0x4U * (vc)))
#define CPRH_CTL				0x3A80U

#define BITFIELD(mask, shift, val)		(((uint32_t)(val) << (shift)) & (mask))

#define CPR_FSM_CTL_INIT_MASK			0xFFFFFEFFU
#define CPRH_CTL_INIT_MASK			0x1E01FFFFU
#define CPR_STEP_QUOT_INIT_MASK		0x00000FFFU
#define CPR_TIMER_CLAMP_INIT_MASK		0x08000000U
#define CPR_SAW_ERROR_STEP_LIMIT_MASK		0x000003FFU
#define CPR_MISC_REGISTER_INIT_MASK		0xFF00000CU
#define CPR_MARGIN_ADJ_CTL_INIT_MASK		0x03FFF116U
#define CPR_MARGIN_TEMP_CORE_TIMERS_MASK	0x1FFFFFFFU

static bool cprh_initialized;

static void cprh_update(uintptr_t base, uint32_t offset, uint32_t mask,
			uint32_t value)
{
	uintptr_t addr = base + offset;
	uint32_t reg = mmio_read_32(addr);

	reg &= ~mask;
	reg |= value & mask;
	mmio_write_32(addr, reg);
}

static uint32_t cprh_read(uintptr_t base, uint32_t offset)
{
	return mmio_read_32(base + offset);
}

static void cprh_write(uintptr_t base, uint32_t offset, uint32_t value)
{
	mmio_write_32(base + offset, value);
}

static uint32_t cprh_round_up(uint32_t value, uint32_t div)
{
	if ((value % div) == 0U) {
		return value;
	}

	return value + div - (value % div);
}

static void cprh_config_controller(const cprh_controller_cfg_t *cfg)
{
	uintptr_t base = cfg->base_addr;

	cprh_update(base, CPR_FSM_CTL, CPR_FSM_CTL_INIT_MASK, cfg->fsm_ctl);
	cprh_update(base, CPRH_CTL, CPRH_CTL_INIT_MASK, cfg->cprh_ctl);
	cprh_write(base, CPR_TIMER_AUTO_CONT, cfg->timer_auto_cont);
	cprh_update(base, CPR_STEP_QUOT_INIT, CPR_STEP_QUOT_INIT_MASK,
		    cfg->step_quot_init);
	cprh_update(base, CPR_TIMER_CLAMP, CPR_TIMER_CLAMP_INIT_MASK,
		    cfg->timer_clamp);
	cprh_update(base, CPR_SAW_ERROR_STEP_LIMIT,
		    CPR_SAW_ERROR_STEP_LIMIT_MASK, cfg->saw_error_step_limit);
	cprh_update(base, CPR_MISC_REGISTER, CPR_MISC_REGISTER_INIT_MASK,
		    cfg->misc_register);
	cprh_update(base, CPR_MARGIN_ADJ_CTL, CPR_MARGIN_ADJ_CTL_INIT_MASK,
		    cfg->margin_adj_ctl);
	cprh_update(base, CPR_MARGIN_TEMP_CORE_TIMERS,
		    CPR_MARGIN_TEMP_CORE_TIMERS_MASK,
		    cfg->margin_temp_core_timers);
	cprh_write(base, CPR_SENSOR_MASK_WRITE0, cfg->sensor_mask);
	cprh_write(base, CPR_SENSOR_BYPASS_WRITE0, cfg->sensor_bypass_mask);

	for (uint8_t i = 0; i < cfg->sensor_count; i++) {
		uint32_t thread = (cfg->sensor_thread_mask >> i) & 0x1U;

		if (((cfg->sensor_thread_mask_thread2 >> i) & 0x1U) != 0U) {
			thread = 0x2U;
		}
		cprh_write(base, CPR_SENSOR_THREAD(i), thread);
	}
}

static void cprh_config_thread(uintptr_t base, const cprh_thread_cfg_t *thread)
{
	uint32_t value = 0U;

	value |= BITFIELD(0x0003E000U, 13, thread->up_threshold);
	value |= BITFIELD(0x00001F00U, 8, thread->dn_threshold);
	cprh_write(base, CPR_THRESHOLD(thread->thread_num), value);
}

static void cprh_config_vc(uintptr_t base, uint8_t thread_num,
			   const vpp_cprh_t *entry)
{
	uint32_t base_voltage;
	uint32_t target;
	uint32_t target_delta;
	uint32_t ceil_voltage;
	uint32_t floor_voltage;
	uint32_t value;

	target = cprh_read(base, CPR_TARGET0(thread_num, entry->ro));
	if ((target == 0U) || (target > entry->target_quot)) {
		target = entry->target_quot;
		cprh_write(base, CPR_TARGET0(thread_num, entry->ro), target);
	}

	cprh_write(base, CPR_GCNT0 + (entry->ro * 4U), 19U);

	base_voltage = (cprh_read(base, CPRH_CTL) & 0x000007FEU) >> 1;
	if (entry->target_quot < target) {
		target_delta = 0U;
	} else {
		target_delta = cprh_round_up(entry->target_quot - target, 4U) / 4U;
	}

	ceil_voltage = (entry->ceil_mV / 4U) - base_voltage;
	floor_voltage = (entry->floor_mV / 4U) - base_voltage;

	if ((target_delta > 0x1FFU) || (ceil_voltage > 0xFFU) ||
	    (floor_voltage > 0xFFU)) {
		WARN("CPRH: skipping invalid VC %u for thread %u\n",
		     entry->vc, thread_num);
		return;
	}

	value = BITFIELD(0x40000000U, 30, 1);
	value |= BITFIELD(0x20000000U, 29, 1);
	value |= BITFIELD(0x1E000000U, 25, entry->ro);
	value |= BITFIELD(0x01FF0000U, 16, target_delta);
	value |= BITFIELD(0x0000FF00U, 8, floor_voltage);
	value |= BITFIELD(0x000000FFU, 0, ceil_voltage);

	cprh_write(base, CPRH_VIRTUAL_CORNER(thread_num, entry->vc), value);
}

void qti_cprh_init(void)
{
	const cprh_controller_cfg_t *cfg = &qti_cprh_controller;

	if (cprh_initialized) {
		return;
	}

	cprh_config_controller(cfg);

	for (uint8_t i = 0; i < cfg->thread_count; i++) {
		const cprh_thread_cfg_t *thread = &cfg->threads[i];
		const vpp_t *plan = qti_vpp_get_plan(cfg->rail_name,
						     thread->clk_name);

		if (plan == NULL) {
			ERROR("CPRH: missing VPP plan for %s/%s\n",
			      cfg->rail_name, thread->clk_name);
			continue;
		}

		cprh_config_thread(cfg->base_addr, thread);
		for (uint16_t j = 0; j < plan->cnt; j++) {
			cprh_config_vc(cfg->base_addr, thread->thread_num,
				       &plan->cprh[j]);
		}
	}

	cprh_initialized = true;
}
