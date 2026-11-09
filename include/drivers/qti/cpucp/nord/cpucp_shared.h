/*===========================================================================
 *
 * Copyright (c) QUALCOMM Technologies Incorporated.
 * All Rights Reserved.
 * QUALCOMM Proprietary and Confidential.
 *
 * $Header: cpucp_shared.h
 *
 *==========================================================================*/

#ifndef __CPUCP_SHARED_H__
#define __CPUCP_SHARED_H__

#include "cpucp_hwio.h"

#define CPUCP_IPC_SEC_BUF_BASE		APSEC_CPUCP_SCMI_IPC_BASE
#define CPUCP_IPC_SEC_BUF_SIZE		APSEC_CPUCP_SCMI_IPC_SIZE

#define CPU_LPM_SYNC_DATA_BASE		(CPUCP_IPC_SEC_BUF_BASE + CPUCP_IPC_SEC_BUF_SIZE)
#define CPU_LPM_SYNC_DATA_SIZE           0x100

/*
 * RVBAR shared-memory address.
 *
 * TF-A writes the BL31 warm-boot entry point here before issuing the
 * SCMI Reset Domain request to CPUCP.  CPUCP reads these two 32-bit
 * words and programs the per-core RVBAR registers before releasing
 * the core from reset.
 */

/*
 * Packed structure to guarantee exact hardware memory layout.
 * Aligned to 64 bytes to match a standard Arm cache-line.
 */
typedef struct __attribute__((packed, aligned(64))) {
    uint32_t magic;         /* Validation tag (e.g., APSEC_BOOT_COOKIE_MAGIC) */
    uint32_t reserved;      /* Padding for 8-byte alignment of the address */
    uint64_t rvbar_addr;    /* The actual warmboot entrypoint address */
    /* 48 bytes of explicit padding to completely isolate the cache line */
    uint8_t  pad[48];
} apsec_boot_cookie_t;

/* Define a distinct magic value to validate the cookie state */
#define APSEC_BOOT_COOKIE_MAGIC   0x544F4F42  /* "BOOT" in ASCII */
#define APSEC_BOOT_COOKIE_INVALID 0x00000000

#define APSEC_CPUCP_BOOT_COOKIE_BASE 	(CPU_LPM_SYNC_DATA_BASE + CPU_LPM_SYNC_DATA_SIZE)
#define APSEC_CPUCP_BOOT_COOKIE_SIZE	0x100

#endif /* __CPUCP_SHARED_H__ */
