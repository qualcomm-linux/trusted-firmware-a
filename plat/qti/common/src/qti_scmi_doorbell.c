/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * QTI platform SCMI doorbell (mailbox) implementation.
 *
 * Platform-specific SCMI channel parameters (mailbox base address, doorbell
 * register, masks, timeout) are obtained from platform_def.h so that this
 * file remains SoC-agnostic.
 *
 * This file provides:
 *   - qti_scmi_ring_doorbell()    – the ring_doorbell callback used by the
 *                                   ARM CSS SCMI driver
 *   - qti_scmi_plat_info         – the scmi_channel_plat_info_t descriptor
 *   - qti_scmi_get_channel()     – initialise and return the SCMI channel
 *   - plat_css_get_scmi_info()   – weak platform hook consumed by the
 *                                   ARM CSS SCMI driver
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>

#include <drivers/arm/css/scmi.h>

#include <platform_def.h>

/*
 * qti_scmi_ring_doorbell - write the doorbell register to notify the remote
 * processor (CPUCP/SCP) that a new SCMI packet is ready in shared memory.
 *
 * The register is a write-1-to-set trigger; bits not written are unaffected,
 * so db_preserve_mask is 0 and we simply write db_modify_mask directly.
 */
static void qti_scmi_ring_doorbell(struct scmi_channel_plat_info *plat_info)
{
	mmio_write_32((uintptr_t)plat_info->db_reg_addr,
		      plat_info->db_modify_mask);
}

/*
 * qti_scmi_plat_info - SCMI channel platform descriptor.
 *
 * All addresses and masks are sourced from platform_def.h so that this
 * file does not need to be modified when porting to a new SoC.
 */
static scmi_channel_plat_info_t qti_scmi_plat_info = {
	.scmi_mbx_mem     = QTI_SCMI_MBX_MEM_BASE,
	.db_reg_addr      = QTI_SCMI_DB_REG_ADDR,
	.db_preserve_mask = QTI_SCMI_DB_PRESERVE_MASK,
	.db_modify_mask   = QTI_SCMI_DB_MODIFY_MASK,
	.ring_doorbell    = qti_scmi_ring_doorbell,
	.cookie           = NULL,
	.delay            = QTI_SCMI_POLL_DELAY_US,
	.timeout          = QTI_SCMI_TIMEOUT_US,
};

/* SCMI channel lock – type depends on HW_ASSISTED_COHERENCY. */
static scmi_lock_t qti_scmi_lock;

/* SCMI channel instance. */
static scmi_channel_t qti_scmi_channel = {
	.info           = &qti_scmi_plat_info,
	.lock           = &qti_scmi_lock,
	.is_initialized = 0,
};

/*
 * qti_scmi_get_channel - initialise and return the QTI SCMI channel.
 *
 * Calls scmi_init() to initialise the channel lock via the standard ARM CSS
 * SCMI driver path.  scmi_init() also queries the Power Domain (0x11) and
 * System Power (0x12) protocol versions; CPUCP on Nord only implements the
 * Reset Domain protocol (0x16), so those queries will fail and scmi_init()
 * will return NULL.  In that case the channel lock has already been
 * initialised by scmi_init(), so we mark the channel ready manually to allow
 * the Reset Domain protocol to proceed.
 *
 * Returns a pointer to the initialised scmi_channel_t cast to void *, or
 * NULL if the channel could not be initialised at all.
 */
void *qti_scmi_get_channel(void)
{
	if (qti_scmi_channel.is_initialized != 0) {
		return (void *)&qti_scmi_channel;
	}

	/*
	 * scmi_init() initialises the channel lock and then queries the
	 * Power Domain and System Power protocol versions.  CPUCP on Nord
	 * does not implement those protocols, so scmi_init() is expected to
	 * return NULL after printing a WARN.  The lock is already initialised
	 * at that point, so we just set is_initialized = 1 to allow the
	 * Reset Domain protocol (0x16) to be used.
	 */
	if (scmi_init(&qti_scmi_channel, NULL) == NULL) {
		INFO("QTI SCMI: scmi_init() returned NULL; CPUCP does not "
		     "implement power-domain/sys-power protocols. "
		     "Channel usable for Reset Domain protocol.\n");
		qti_scmi_channel.is_initialized = 1;
	}

	return (void *)&qti_scmi_channel;
}

/*
 * plat_css_get_scmi_info - return the platform SCMI channel descriptor.
 *
 * This is the weak hook declared in include/drivers/arm/css/scmi.h that
 * platform code must override to supply the channel parameters.  The ARM CSS
 * SCMI driver calls this during scmi_init() to obtain the mailbox base
 * address, doorbell register, and ring_doorbell callback.
 */
scmi_channel_plat_info_t *plat_css_get_scmi_info(unsigned int channel_id __unused)
{
	return &qti_scmi_plat_info;
}
