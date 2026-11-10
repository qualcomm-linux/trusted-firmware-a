/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_RESET_H
#define SYSDBG_RESET_H

#include <stdbool.h>
#include <stdint.h>

bool SysdbgHWInit(void);
bool SysdbgResetCore(void);
bool SysdbgMemoryReset(void);
bool SysdbgColdBootInit(void);
bool SysdbgRecoverRegisters(void);
void system_debug_exit(uint32_t cpu);

#endif /* SYSDBG_RESET_H */
