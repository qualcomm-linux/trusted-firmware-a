/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_HWIO_H
#define QTI_CPUCP_HWIO_H

/*
 * Hardware register addresses for the CPUCP (CPU Control Processor) on
 * Nord (SM8750 / Wildcat family).
 *
 * SCMI shared memory is located in CPUCP DTIM0 (AP-side base 0x18b30000).
 * The AP Secure to CPUCP SCMI IPC buffer sits at DTIM0 + 0x3000.
 *
 * The AP rings the CPUCP doorbell by writing the OSM_IPC bit (bit 28) to
 * the APSS_INTU TZ_IPC_INTERRUPT register at 0x17824004.
 */

/* AP-side physical base address of CPUCP DTIM0. */
#define CPUCP_DTIM0_BASE_AP			(0x18b30000U)

/*
 * AP Secure to CPUCP SCMI IPC buffer.
 *
 * Located in CPUCP DTIM0 at offset 0x3000 (physical address 0x18b33000).
 * Size: 0x400 bytes (1 KB).
 */
#define APSEC_CPUCP_SCMI_IPC_BASE		(CPUCP_DTIM0_BASE_AP + 0x3000U)
#define APSEC_CPUCP_SCMI_IPC_SIZE		(0x400U)

/* Aliases kept for any existing callers. */
#define CPUCP_IPC_SEC_BUF_BASE			APSEC_CPUCP_SCMI_IPC_BASE
#define CPUCP_SECIPC_RAM_LENGTH			APSEC_CPUCP_SCMI_IPC_SIZE

/*
 * AP to CPUCP doorbell: APSS_INTU TZ_IPC_INTERRUPT register.
 *
 * Base: APSS_INTU_IPC_BASE = 0x17824000
 * Register offset: 0x4
 * OSM_IPC bit: bit 28
 *
 * Writing BIT(28) to this register raises an interrupt on CPUCP,
 * notifying it that a new SCMI message is ready in the IPC buffer.
 */
#define APSS_SHARED_TZ_IPC_BASE				(0x17824000U)
#define APSS_SHARED_TZ_IPC_INTERRUPT_ADDR		(APSS_SHARED_TZ_IPC_BASE + 0x4U)
#define APSS_SHARED_TZ_IPC_INTERRUPT_OSM_IPC_SHFT	(28U)
#define APSS_SHARED_TZ_IPC_INTERRUPT_BIT		(1U << APSS_SHARED_TZ_IPC_INTERRUPT_OSM_IPC_SHFT)

#endif /* QTI_CPUCP_HWIO_H */