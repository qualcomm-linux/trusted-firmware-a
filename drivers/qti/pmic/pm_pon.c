/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/delay_timer.h>

#include <spmi_arb.h>
#include <spmi_platform.h>
#include <pm_pon.h>

#define S2_RESET_EN			BIT(7)

void pm_app_ps_hold_cfg(enum pon_reset_type reset_type)
{
	spmi_arb_write8(PON_PS_HOLD_RESET_CTL, reset_type);
}