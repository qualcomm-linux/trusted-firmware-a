/********************************************************************
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#include <stdint.h>
#include <stddef.h>
#include "drivers/qti/smmu/smmu.h"
#include <lib/utils_def.h>
#include <stdio.h>

void qti_smmu_init(void)
{
	for (int i = 0; i < qti_smmu_cfg_count; i++) {
		*((volatile uint32_t *)qti_smmu_cfg[i].addr) = qti_smmu_cfg[i].value;
	}
}
