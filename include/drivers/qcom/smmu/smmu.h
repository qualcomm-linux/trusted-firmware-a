/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMMU_H
#define SMMU_H

#include <stddef.h>
#include <stdint.h>

#ifdef QCOM_SMMU_ENABLED
struct smmu_cfg_s {
	uintptr_t addr;
	uint32_t value;
};

void qcom_smmu_init(void);
extern struct smmu_cfg_s qcom_smmu_cfg[];
extern const size_t qcom_smmu_cfg_count;
#else
static inline void qcom_smmu_init(void) {}
#endif

#endif
