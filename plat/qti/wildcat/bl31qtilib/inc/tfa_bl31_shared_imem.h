/*
 * Copyright (c) 2018-2020, 2025. The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Changes from Qualcomm Technologies, Inc. are provided under the following
 * license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TFA_BL31_SHARED_IMEM_H
#define TFA_BL31_SHARED_IMEM_H

#include <platform_def.h>

/**
 * The following variables should be in sync with the offsets defined in the
 * parser script 'parse_tfa_logs.py'
 */
#define TFA_BL31_SHARED_IMEM_TFA_AREA_BASE	\
	(SHARED_IMEM_BASE + 0x734 + 340)

#define TFA_BL31_IMEM_ADDR(offset)	\
	(TFA_BL31_SHARED_IMEM_TFA_AREA_BASE + (offset))

#define TFA_BL31_SHARED_IMEM_SMC_LOG_BASE	\
	TFA_BL31_IMEM_ADDR(0x0) /* 8 bytes */
#define TFA_BL31_SHARED_IMEM_SMC_LOG_SIZE	\
	TFA_BL31_IMEM_ADDR(0x8) /* 4 bytes */

/* Pointer address that can be picked up by crashscope */
/*
 * ==========================
 *  4KB starting 0x14680000
 * ==========================
 *
 * Start Address    Page Size       Region
 * =======================================
 * 14680CF0         8Bytes          TFABL31-LOG-BasePtr
 * ... other shared data
 * 14680000
 */
#define TFA_BL31_SHARED_IMEM_RING_LOG_BASE      (SHARED_IMEM_BASE + 0xCF0)

/* Next available: TFA_BL31_IMEM_ADDR(0x0C), 4-byte aligned */
/* RESERVED until TFA_BL31_IMEM_ADDR(0x2C), 32 bytes available */

#endif /* TFA_BL31_SHARED_IMEM_H */
