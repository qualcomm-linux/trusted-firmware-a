/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * NCC CPU power management via SCMI Power Domain protocol.
 *
 * On NCC targets (e.g. Nord), secondary CPU cores (cores 1–17) and clusters
 * are brought up and torn down by the CPUCP co-processor firmware.  TF-A
 * communicates with CPUCP using the SCMI Power Domain Management Protocol
 * (SCMI spec §14, protocol ID 0x11).
 *
 * Core 0 is brought up by the Primary Boot Loader (PBL) and is never
 * powered on via SCMI.
 *
 * This file implements the NCC CPU power API for TF-A using the ARM CSS SCMI
 * stack (drivers/arm/css/scmi/scmi_pwr_dmn_proto.c).  The ARM CSS layer
 * handles mailbox layout, channel locking, doorbell signalling, and response
 * polling.  This file only needs to:
 *   1. Obtain the QTI SCMI channel (qti_scmi_get_channel).
 *   2. Map MPIDR → SCMI domain_id via NCC_SCMI_DOMAIN_ID() from
 *      ncc_cpu_power.h.
 *   3. Call scmi_pwr_state_set / scmi_pwr_state_get with
 *      NCC_SCMI_PWR_STATE_ON / NCC_SCMI_PWR_STATE_OFF.
 *
 * Power state encoding (SCMI spec §14.4.4, level-type):
 *   NCC_SCMI_PWR_STATE_ON  = 0x0  (level 0 = powered on)
 *   NCC_SCMI_PWR_STATE_OFF = 0x1  (level 1 = powered off)
 *   (defined in plat/qti/cpu/ncc/include/ncc_cpu_power.h)
 *
 * Note: SCMI_PWR_STATE_OFF = BIT_32(30) in scmi.h is the composite-state
 * type bit and must NOT be used for CPUCP power state commands.
 *
 * Exported symbols (declared in ncc_cpu_power.h):
 *   cpucp_cluster_core_power_on()       – PSCI CPU_ON path (cores 1–17)
 *   cpucp_cluster_core_power_off()      – PSCI CPU_OFF / suspend path
 *   cpucp_get_cluster_core_power_info() – query current power state
 *   cpuss_pwron_lib_init()              – initialise SCMI channel at boot
 *
 * Reference:
 *   cpucp/hostlib/host_v2/init/common/scmi_power.c
 */

#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>

/*
 * ARM CSS SCMI stack: provides scmi_channel_t, scmi_init(),
 * scmi_pwr_state_set(), scmi_pwr_state_get(), and SCMI_E_SUCCESS.
 */
#include <drivers/arm/css/scmi.h>

/*
 * NCC CPU power API declarations and SCMI power state / domain ID
 * definitions for this module.
 *   NCC_SCMI_PWR_STATE_ON  = 0U
 *   NCC_SCMI_PWR_STATE_OFF = 1U
 *   NCC_SCMI_DOMAIN_ID(cluster, core) = (cluster << 8) | core
 */
#include <ncc_cpu_power.h>

/*
 * SCMI channel handle – set by cpuss_pwron_lib_init() and used by all
 * subsequent cpucp_cluster_core_power_* calls.
 */
static void *ncc_scmi_ch;

/* Provided by plat/qti/common/src/qti_scmi_doorbell.c */
void *qti_scmi_get_channel(void);

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

/*
 * cpucp_cluster_core_power_on
 *
 * Called by the PSCI CPU_ON handler (plat_qti_pwr_domain_on in
 * qcom_ncc_psci.c) to bring up a secondary core via CPUCP.
 *
 * Sends SCMI POWER_STATE_SET(domain_id, NCC_SCMI_PWR_STATE_ON) to the CPUCP
 * firmware.  The CPUCP releases the core from reset and it enters TF-A at
 * the warm-boot entry point.
 *
 * Domain ID: NCC_SCMI_DOMAIN_ID(cluster_id, core_id) = cluster_id << 8 | core_id
 *   cluster_id = MPIDR.AFF2 (bits [23:16])
 *   core_id    = MPIDR.AFF0 (bits  [7:0])
 *
 * @mpidr   MPIDR of the target core
 * Returns  0 on success, positive SCMI error code on failure.
 */
uint32_t cpucp_cluster_core_power_on(uint64_t mpidr)
{
	uint8_t  core_id    = (uint8_t)((mpidr >> MPIDR_AFF1_SHIFT) & 
					MPIDR_AFFLVL_MASK);
	uint8_t  cluster_id = (uint8_t)((mpidr >> MPIDR_AFF2_SHIFT) &
					MPIDR_AFFLVL_MASK);
	uint32_t domain_id  = NCC_SCMI_DOMAIN_ID(cluster_id, core_id);
	int ret;

	assert(ncc_scmi_ch != NULL);

	VERBOSE("NCC: SCMI CPU-ON  cluster=%u core=%u domain=0x%x\n",
		cluster_id, core_id, domain_id);

	ret = scmi_pwr_state_set(ncc_scmi_ch, domain_id, NCC_SCMI_PWR_STATE_ON);
	if (ret != SCMI_E_SUCCESS) {
		ERROR("NCC: SCMI CPU-ON failed cluster=%u core=%u ret=%d\n",
		      cluster_id, core_id, ret);
		return (uint32_t)(-ret);
	}

	return 0U;
}

/*
 * cpucp_cluster_core_power_off
 *
 * Called by the PSCI CPU_OFF / suspend handler (plat_qti_pwr_domain_off
 * and plat_qti_pwr_domain_suspend in qcom_ncc_psci.c) to power down a
 * core via CPUCP.
 *
 * Sends SCMI POWER_STATE_SET(domain_id, NCC_SCMI_PWR_STATE_OFF) to the CPUCP
 * firmware.  The CPUCP gates the core's power rail / clocks after the core
 * executes WFI.
 *
 * @mpidr   MPIDR of the calling core
 * Returns  0 on success, positive SCMI error code on failure.
 */
uint32_t cpucp_cluster_core_power_off(uint64_t mpidr)
{
	uint8_t  core_id    = (uint8_t)((mpidr >> MPIDR_AFF1_SHIFT) & 
					MPIDR_AFFLVL_MASK);
	uint8_t  cluster_id = (uint8_t)((mpidr >> MPIDR_AFF2_SHIFT) &
					MPIDR_AFFLVL_MASK);
	uint32_t domain_id  = NCC_SCMI_DOMAIN_ID(cluster_id, core_id);
	int ret;

	assert(ncc_scmi_ch != NULL);

	VERBOSE("NCC: SCMI CPU-OFF cluster=%u core=%u domain=0x%x\n",
		cluster_id, core_id, domain_id);

	ret = scmi_pwr_state_set(ncc_scmi_ch, domain_id, NCC_SCMI_PWR_STATE_OFF);
	if (ret != SCMI_E_SUCCESS) {
		ERROR("NCC: SCMI CPU-OFF failed cluster=%u core=%u ret=%d\n",
		      cluster_id, core_id, ret);
		return (uint32_t)(-ret);
	}

	return 0U;
}

/*
 * cpucp_get_cluster_core_power_info
 *
 * Query the current SCMI power state of a core from the CPUCP firmware.
 * Uses SCMI POWER_STATE_GET (scmi_pwr_state_get from scmi_pwr_dmn_proto.c).
 *
 * @mpidr   MPIDR of the target core
 * @state   Output: NCC_SCMI_PWR_STATE_ON (0) or NCC_SCMI_PWR_STATE_OFF (1)
 * Returns  0 on success, positive SCMI error code on failure.
 */
uint32_t cpucp_get_cluster_core_power_info(uint64_t mpidr, uint32_t *state)
{
	uint8_t  core_id    = (uint8_t)((mpidr >> MPIDR_AFF1_SHIFT) & 
					MPIDR_AFFLVL_MASK);
	uint8_t  cluster_id = (uint8_t)((mpidr >> MPIDR_AFF2_SHIFT) &
					MPIDR_AFFLVL_MASK);
	uint32_t domain_id  = NCC_SCMI_DOMAIN_ID(cluster_id, core_id);
	int ret;

	assert(ncc_scmi_ch != NULL);
	assert(state != NULL);

	ret = scmi_pwr_state_get(ncc_scmi_ch, domain_id, state);
	if (ret != SCMI_E_SUCCESS) {
		ERROR("NCC: SCMI PWR-GET failed cluster=%u core=%u ret=%d\n",
		      cluster_id, core_id, ret);
		return (uint32_t)(-ret);
	}

	return 0U;
}

static int qti_cpu_scmi_protocol_init(scmi_channel_t *ch)
{
	int ret;

	ret = scmi_pwr_init(ch);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

/*
 * cpuss_pwron_lib_init
 *
 * Initialise the NCC SCMI Power Domain channel.  Must be called once
 * during platform setup (from plat_qti_pwr_psci_init in qcom_ncc_psci.c)
 * before any cpucp_cluster_core_power_* call.
 *
 * Obtains the QTI SCMI channel (configured with the CPUCP mailbox base
 * address QTI_SCMI_MBX_MEM_BASE and doorbell register QTI_SCMI_DB_REG_ADDR
 * from platform_def.h) and calls scmi_init() to initialise the ARM CSS SCMI
 * channel infrastructure.
 */
void cpuss_pwron_lib_init(void)
{
	scmi_channel_t *ch = (scmi_channel_t *)qti_scmi_get_channel();

	assert(ch != NULL);

	ncc_scmi_ch = scmi_init(ch, qti_cpu_scmi_protocol_init);
	assert(ncc_scmi_ch != NULL);

	INFO("NCC: CPUCP SCMI Power Domain channel ready\n");
}
