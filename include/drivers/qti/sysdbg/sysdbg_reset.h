/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSDBG_RESET_H
#define SYSDBG_RESET_H

#include <stdbool.h>
#include <stdint.h>

bool sysdbg_cold_boot_init(void);
uintptr_t sysdbg_get_shared_imem_base(void);

#endif /* SYSDBG_RESET_H */
