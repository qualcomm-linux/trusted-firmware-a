/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMEM_VERSION_H
#define SMEM_VERSION_H

#include "smem_os.h"

/*
 * SMEM_VERSION_INFO_SIZE defines the size of a version array.
 * SMEM_VERSION_INFO_OFFSET is the index into a version array for this processor
 * or special code.
 *
 * DO NOT CHANGE THE FOLLOWING VALUES AS IT MAY BREAK VERSION CHECKING, ONLY
 * ADD TO THE END OF THE LIST!!
 */
#define SMEM_VERSION_INFO_SIZE	32

#define SMEM_VERSION_BOOT_OFFSET	7
#define SMEM_VERSION_TZ_HYP_OFFSET	0
#define SMEM_VERSION_RPM_OFFSET	14
#define SMEM_VERSION_TME_OFFSET	19
#define SMEM_VERSION_SOCCP_OFFSET	21
#define SMEM_VERSION_OOB_NS_OFFSET	22
#define SMEM_VERSION_OOB_TEE_OFFSET	23
#define SMEM_VERSION_DCP_OFFSET	24
#define SMEM_VERSION_QECP_OFFSET	25

#define SMEM_LEGACY_VERSION_ID	0x000B0000

#endif /*SMEM_VERSION_H*/
