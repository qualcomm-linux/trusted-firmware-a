/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <drivers/qcom/smmu/smmu.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

void qcom_smmu_init(void)
{
	for (int i = 0; i < qcom_smmu_cfg_count; i++) {
		mmio_write_32(qcom_smmu_cfg[i].addr, qcom_smmu_cfg[i].value);
	}
}
