/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCOM_ORYON_V1_H
#define QCOM_ORYON_V1_H

#define ORYON_V1_PERF_MIDR			U(0x510F0010)
#define ORYON_V1_POWER_MIDR			U(0x510F0020)
#define ORYON_V1_MIDR				U(0x515F0014)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define ORYON_V1_CPUECTLR_EL1			S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define ORYON_V1_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define ORYON_V1_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

#endif /* QCOM_ORYON_V1_H */
