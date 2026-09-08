/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_HWIO_H
#define QTI_CPUCP_HWIO_H

/*
 * CPUCP / EPSS shared-memory and APSS IPC register addresses for Nord
 * (SA8797P).
 *
 *   CPUCP_DTIM0_BASE          = 0x18b30000
 *   CPUCP_PDP_SCMI_IPC_BASE   = CPUCP_DTIM0_BASE
 *   CPUCP_PDP_SCMI_IPC_SIZE   = 0x3000 (3 x 4KB PDP SCMI channels)
 *   APSEC_CPUCP_SCMI_IPC_BASE = CPUCP_PDP_SCMI_IPC_BASE + CPUCP_PDP_SCMI_IPC_SIZE
 *                             = 0x18b33000
 * The APSS->CPUCP doorbell is the shared APSS_INTU TZ_IPC_INTERRUPT register
 * (0x17824004): writing its OSM_IPC field kicks CPUCP. Confirmed via IPCAT
 * (nordschleife_2.0): APSS_INTU_TZ_IPC_INTERRUPT @ 0x17824004, OSM_IPC[31:28].
 */
#define CPUCP_IPC_SEC_BUF_BASE			(0x18b33000U)

/* Length of the secure IPC shared-memory window used for SCMI packets. */
#define CPUCP_SECIPC_RAM_LENGTH			(0x400U)

/* APSS -> CPUCP doorbell (shared with the AOP TZ_IPC_INTERRUPT register). */
#define APSS_SHARED_TZ_IPC_INTERRUPT_ADDR	(0x17824004U)
#define APSS_SHARED_TZ_IPC_INTERRUPT_OSM_IPC_SHFT	(28U)

#endif /* QTI_CPUCP_HWIO_H */
