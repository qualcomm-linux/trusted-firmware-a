/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * SAW4 AVS (Adaptive Voltage Scaling) public API.
 *
 * The SAW4 AVS_CTL register arbitrates between SW and CPRH hardware control
 * of the rail voltage. CPRH's boot-init sequence configures the ramp timing
 * then hands control to hardware via saw_avs_enable(1).
 */

#ifndef SAW_H
#define SAW_H

#include <stdint.h>

/* saw_avs_enable - hand control of the Vdd level to hardware (CPRH). */
void saw_avs_enable(uint32_t enable);

/* saw_avs_ramp_en - enable CPR/VCTL voltage ramping. */
void saw_avs_ramp_en(uint32_t ramp_en);

/* saw_avs_set_clock_divider - AVS timer clock divider. */
void saw_avs_set_clock_divider(uint32_t divider);

/*
 * saw_avs_set_voltage_step_size - max up/down voltage step per AVS timer
 * tick. Faithfully preserves the vendor HAL's swapped assignment: step_up
 * programs the VLVL_STEP_DN field and step_dn programs VLVL_STEP_UP.
 */
void saw_avs_set_voltage_step_size(uint32_t step_up, uint32_t step_dn);

/* saw_avs_set_voltage_width - width of each AVS voltage level step. */
void saw_avs_set_voltage_width(uint32_t width);

#endif /* SAW_H */
