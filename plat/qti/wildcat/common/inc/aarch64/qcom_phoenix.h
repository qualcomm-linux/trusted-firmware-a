/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCOM_PHOENIX_H
#define QCOM_PHOENIX_H

#define PHOENIX_PERF_MIDR			U(0x510F0010)
#define PHOENIX_POWER_MIDR			U(0x510F0020)
#define PHOENIX_MIDR				U(0x515F0014)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define PHOENIX_CPUECTLR_EL1			S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define PHOENIX_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define PHOENIX_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

#endif /* QCOM_PHOENIX_H */
