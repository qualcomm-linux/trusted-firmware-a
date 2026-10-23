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

enum core_lpm_status {
        CORE_NOT_IN_LPM,
        CORE_IN_LPM = 255
};



#define CPUCP_IPC_SEC_BUF_BASE		APSEC_CPUCP_SCMI_IPC_BASE
#define CPUCP_IPC_SEC_BUF_SIZE		APSEC_CPUCP_SCMI_IPC_SIZE

#define CPU_LPM_SYNC_DATA_BASE		(CPUCP_IPC_SEC_BUF_BASE + CPUCP_IPC_SEC_BUF_SIZE)

#endif /* __CPUCP_SHARED_H__ */
