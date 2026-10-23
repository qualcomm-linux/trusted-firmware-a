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

/*
 * SCMI Power Domain power state values for CPUCP (level-type encoding,
 * SCMI spec §14.4.4).
 *   ON  = level 0 = 0x0
 *   OFF = level 1 = 0x1
 */
#define NCC_SCMI_PWR_STATE_ON	0U
#define NCC_SCMI_PWR_STATE_OFF	1U

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