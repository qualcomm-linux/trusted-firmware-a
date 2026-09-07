/*
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Changes from Qualcomm Technologies, Inc. are provided under the following
 * license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_PM_H
#define PLATFORM_PM_H

/*----------------------------------------------------------------------------*/
/* Power-related macros */
/*----------------------------------------------------------------------------*/

#define QTI_PWR_LVL0 MPIDR_AFFLVL0
#define QTI_PWR_LVL1 MPIDR_AFFLVL1
#define QTI_PWR_LVL2 MPIDR_AFFLVL2
#define QTI_PWR_LVL3 MPIDR_AFFLVL3

/*
 *  Macros for local power states encoded by State-ID field
 *  within the power-state parameter.
 */
/* Local power state for power domains in Run state. */
#define QTI_LOCAL_STATE_RUN 0
/*
 * Local power state for clock-gating. Valid only for CPU and not cluster power
 * domains
 */
#define QTI_LOCAL_STATE_STB 1
/*
 * Local power state for retention. Valid for CPU and cluster power
 * domains
 */
#define QTI_LOCAL_STATE_RET 5
/*
 * Local power state for OFF/power down. Valid for CPU, cluster, RSC and PDC
 * power domains
 */
#define QTI_LOCAL_STATE_OFF 6
/*
 * Local power state for DEEPOFF/power rail down. Valid for CPU, cluster and RSC
 * power domains
 */
#define QTI_LOCAL_STATE_DEEPOFF 7

/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE QTI_LOCAL_STATE_RET

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE QTI_LOCAL_STATE_DEEPOFF

/*----------------------------------------------------------------------------*/
/* PSCI power domain topology definitions */
/*----------------------------------------------------------------------------*/
#define PLAT_CLUSTER_COUNT 3
#define PLAT_CORE_COUNT_PER_CLUSTER 4
#define PLAT_MAX_PWR_LVL 2

/* The QTI power domain tree descriptor */
#define PLAT_TREE_DESC			\
{					\
	1, /* Overall CX Rail */		\
	PLAT_CLUSTER_COUNT,		\
	4, /* 4 cores in cluster 0 */	\
	4, /* 4 cores in cluster 1 */	\
	4, /* 4 cores in cluster 2 */	\
}

#define PLATFORM_CORE_COUNT 12

/* Define indexes of qti_power_domain_tree_desc */
#define PLAT_RAIL_COUNT_IDX (0U)
#define PLAT_CLUSTER_COUNT_IDX (1U)
#define PLAT_CLUSTER_CORE_COUNT_START_IDX (2U)

#endif /* PLATFORM_PM_H */
