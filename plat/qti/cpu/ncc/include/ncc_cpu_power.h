/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * NCC CPU power management interface.
 *
 * Declares the NCC CPU power API used by the PSCI layer (qcom_ncc_psci.c).
 * The implementation (ncc_cpu_power.c) uses the ARM CSS SCMI Power Domain
 * Management Protocol (drivers/arm/css/scmi/scmi_pwr_dmn_proto.c) to
 * communicate with the CPUCP co-processor firmware.
 *
 * Power state encoding (SCMI spec §14.4.4, level-type):
 *   NCC_SCMI_PWR_STATE_ON  = 0x0  (level 0 = powered on)
 *   NCC_SCMI_PWR_STATE_OFF = 0x1  (level 1 = powered off)
 *
 * Domain ID encoding:
 *   NCC_SCMI_DOMAIN_ID(cluster, core) = (cluster << 8) | core
 */

#ifndef NCC_CPU_POWER_H
#define NCC_CPU_POWER_H

#include <stdint.h>

#define POWER_STATE_CURR_MASK   0x40000FFF
#define POWER_STATE_PREV_MASK   0x00FFF000
#define POWER_STATE_PREV_SHFT   0xC

#define POWER_STATE_CURR(state) (state & POWER_STATE_CURR_MASK)
#define POWER_STATE_PREV(state) ((state & POWER_STATE_PREV_MASK) >> POWER_STATE_PREV_SHFT)

#define POWER_STATE_ON          0x00000000
#define POWER_STATE_OFF         0x40000000
#define POWER_STATE_WARM_ON     0x0FFFFFFF

#define POWER_STATE_C1          0x00000001
#define POWER_STATE_C2          0x00000002
#define POWER_STATE_C3          0x00000003
#define POWER_STATE_C4          0x00000004

#define POWER_STATE_CL1         0x00000011
#define POWER_STATE_CL2         0x00000012
#define POWER_STATE_CL3         0x00000013
#define POWER_STATE_CL4         0x00000014
#define POWER_STATE_CL5         0x00000015

#define POWER_STATE_SS1         0x00000021
#define POWER_STATE_SS2         0x00000022
#define POWER_STATE_SS3         0x00000023

#define POWER_STATE_DS_ENTRY    0x00000031

/* Domain ID Masks and Shifts */
#define DOMAIN_ID_CORE_ID_SHFT          0x0
#define DOMAIN_ID_CORE_ID_BMSK          (0x7F << DOMAIN_ID_CORE_ID_SHFT)

#define DOMAIN_ID_CLUSTER_ID_SHFT               0x8
#define DOMAIN_ID_CLUSTER_ID_BMSK               (0x7F << DOMAIN_ID_CLUSTER_ID_SHFT)

#define FIELD_FROM_DOMAIN_ID(domain_id, field)  ((domain_id & DOMAIN_ID_##field##_BMSK) >> DOMAIN_ID_##field##_SHFT)



#define NCC_SCMI_PWR_STATE_ON	POWER_STATE_ON
#define NCC_SCMI_PWR_STATE_OFF	POWER_STATE_OFF

/*
 * SCMI domain ID for a CPU core:
 *   bits [15:8] = cluster index (MPIDR AFF2)
 *   bits  [7:0] = core index   (MPIDR AFF0)
 */
#define NCC_SCMI_DOMAIN_ID(cluster, core) \
	(((uint32_t)(cluster) << 8U) | (uint32_t)(core))

/*
 * NCC CPU power management API.
 * Implemented in ncc_cpu_power.c using the SCMI Power Domain protocol
 * (drivers/arm/css/scmi/scmi_pwr_dmn_proto.c).
 */
uint32_t cpucp_cluster_core_power_on(uint64_t mpidr);
uint32_t cpucp_cluster_core_power_off(uint64_t mpidr);
uint32_t cpucp_get_cluster_core_power_info(uint64_t mpidr, uint32_t *state);
void cpuss_pwron_lib_init(void);

#endif /* NCC_CPU_POWER_H */
