/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <drivers/qti/smmu/smmu.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

void qti_smmu_init(void)
{
	for (int i = 0; i < qti_smmu_cfg_count; i++) {
		mmio_write_32(qti_smmu_cfg[i].addr, qti_smmu_cfg[i].value);
	}
}
