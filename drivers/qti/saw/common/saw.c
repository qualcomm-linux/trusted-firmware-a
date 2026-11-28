/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/qti/saw/saw.h>
#include <lib/mmio.h>

#include "saw_hwio.h"

#define SAW_AVS_CTL_ADDR	HWIO_SAW4_AVS_CTL_ADDR(SAW_AVS_BASE_ADDR)

#define SAW_AVS_WRITE(field, val)					\
	mmio_write_32(SAW_AVS_CTL_ADDR,				\
		(mmio_read_32(SAW_AVS_CTL_ADDR) &			\
		 ~HWIO_SAW4_AVS_CTL_##field##_BMSK) |			\
		(((val) << HWIO_SAW4_AVS_CTL_##field##_SHFT) &		\
		 HWIO_SAW4_AVS_CTL_##field##_BMSK))

void saw_avs_enable(uint32_t enable)
{
	if (!enable) {
		SAW_AVS_WRITE(EN, 0x0U);
		SAW_AVS_WRITE(CTL_SEL, 0x0U);
	} else {
		SAW_AVS_WRITE(EN, 0x1U);
		SAW_AVS_WRITE(CTL_SEL, 0x1U);
	}
}

void saw_avs_ramp_en(uint32_t ramp_en)
{
	SAW_AVS_WRITE(CPR_RAMP_EN, ramp_en);
	SAW_AVS_WRITE(VCTL_RAMP_EN, ramp_en);
}

void saw_avs_set_clock_divider(uint32_t divider)
{
	SAW_AVS_WRITE(TMR_CLK_DIV, divider);
}

void saw_avs_set_voltage_step_size(uint32_t step_up, uint32_t step_dn)
{
	SAW_AVS_WRITE(VLVL_STEP_DN, step_up);
	SAW_AVS_WRITE(VLVL_STEP_UP, step_dn);
}

void saw_avs_set_voltage_width(uint32_t width)
{
	SAW_AVS_WRITE(VLVL_WIDTH, width);
}
