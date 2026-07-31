/*
 * Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCOM_XPU_H
#define QCOM_XPU_H

#include <stdint.h>

#ifdef QCOM_XPU_BYPASS
void qcom_msm_xpu_bypass(void);
#else
static inline void qcom_msm_xpu_bypass(void) {}
#endif

#endif /* QCOM_XPU_H */
