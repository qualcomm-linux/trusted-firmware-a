/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_ERR_H
#define SYSDBG_ERR_H

#include <cdefs.h>
#include <stdint.h>

#include <sysdbg_log.h>

__dead2 void SysdbgWaitForReset(void);
void SysdbgResetSoC(void);

void sysdbg_err_fatal_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3);

#endif /* SYSDBG_ERR_H */
