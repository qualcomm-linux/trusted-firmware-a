/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <platform_def.h>

#include <ac_cfg.h>

int plat_qti_ac_get_config_image(uintptr_t *base, size_t *size)
{
	if (base == NULL || size == NULL) {
		return -EINVAL;
	}

	*base = TZ_TZ_AC_CONFIG_BASE_ADDR;
	*size = TZ_TZ_AC_CONFIG_SIZE;

	return 0;
}
