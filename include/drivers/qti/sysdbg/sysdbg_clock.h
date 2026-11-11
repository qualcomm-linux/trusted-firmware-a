/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_CLOCK_H
#define SYSDBG_CLOCK_H

#include <stdbool.h>
#include <stdint.h>

/* Type for statically defined clock branches. */
typedef uint32_t cbcr_type;

bool SysdbgClockInit(void);

#endif /* SYSDBG_CLOCK_H */
