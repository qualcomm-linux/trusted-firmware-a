/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_TIMER_H
#define SYSDBG_TIMER_H

#include <stdbool.h>
#include <stdint.h>

void sysdbg_enable_mpm_timer(bool enable);
bool SysdbgTimerInit(void);
uint64_t sysdbg_get_mpm_counter(void);
uint64_t SysdbgGetTimestamp(void);

#endif /* SYSDBG_TIMER_H */
