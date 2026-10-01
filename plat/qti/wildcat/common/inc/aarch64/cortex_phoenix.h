/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_PHOENIX_H
#define CORTEX_PHOENIX_H

#define CORTEX_ORION_MIDR				U(0x6E0F0000)
#define CORTEX_PHOENIX_PERF_MIDR			U(0x510F0010)
#define CORTEX_PHOENIX_POWER_MIDR			U(0x510F0020)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_PHOENIX_CPUECTLR_EL1			S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_PHOENIX_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define CORTEX_PHOENIX_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

#endif /* CORTEX_PHOENIX_H */
