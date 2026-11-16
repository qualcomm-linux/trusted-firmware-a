/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PM_PON_H
#define PM_PON_H

#include <stdint.h>

#include <lib/utils_def.h>

#if defined(QTI_PON5)
/* PM4125-generation register layout; selected by the QTI_PON_PBS_VERSION
 * build variable. See pm_pon5.h.
 */
#include <pm_pon5.h>
#else
/*
 * PON register layout for PM8998-compatible PMICs (this can include other
 * part numbers like PM6150).
 */
#define PON_PS_HOLD_RESET_CTL		0x852U
#define PON_PS_HOLD_RESET_CTL2		0x853U
#endif

#define S2_RESET_EN					BIT(7)

enum pon_reset_type {
	RESET_TYPE_WARM_RESET = 0x1,
	RESET_TYPE_SHUTDOWN = 0x4,
	RESET_TYPE_HARD_RESET = 0x7,
};

void pm_app_ps_hold_cfg(enum pon_reset_type reset_type);

#endif /* PM_PON_H */
