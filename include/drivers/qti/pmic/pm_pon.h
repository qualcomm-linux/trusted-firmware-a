/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PM_PON_H
#define PM_PON_H

#include <stdint.h>

#define PON_PS_HOLD_RESET_CTL		0x852U
#define PON_PS_HOLD_RESET_CTL2		0x853U

enum pon_reset_type {
	RESET_TYPE_WARM_RESET = 0x1,
	RESET_TYPE_SHUTDOWN = 0x4,
	RESET_TYPE_HARD_RESET = 0x7,
};

void pm_app_ps_hold_cfg(enum pon_reset_type reset_type);

#endif /* PM_PON_H */