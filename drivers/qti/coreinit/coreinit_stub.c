/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Core initialization driver - target-specific configuration data.
 * Provides coreinit_info, consumed by coreinit_init() / coreinit_deinit().
 *
 * To add a new chip variant:
 *   1. Define its routes[], requests[], and handle-storage arrays.
 *   2. Define a coreinit_vote[] and a coreinit_config for it.
 *   3. Add a pointer to the new coreinit_config in coreinit_configs[].
 */

#include <stddef.h>

#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/icb/icbuarb.h>
#include <drivers/qti/icb/icbid.h>

#include "coreinit.h"

/*
 * Array of all chip configurations, searched in order by coreinit_init().
 */
static struct coreinit_config *coreinit_configs[] = {
	[0] = NULL,
};

/*
 * Exported top-level symbol.
 * Declared extern in coreinit.c; resolved by the linker at build time.
 */
struct coreinit_info coreinit_info = {
	.num_configs = ARRAY_SIZE(coreinit_configs),
	.configs     = coreinit_configs,
};