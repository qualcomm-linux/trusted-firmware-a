/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lemans_def.h>

#define MAX_IO_HANDLES			U(2)
#define MAX_IO_DEVICES			U(2)
#define MAX_IO_BLOCK_DEVICES		U(1)

#define BL2_BASE			0x1c00e000
#define BL2_SIZE			0x100000
#define BL2_LIMIT			(BL2_BASE + BL2_SIZE)

#define BL31_BASE			0x1c200000
#define BL31_SIZE			0x00100000
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

#define BL32_BASE			0x1c300000
#define BL32_SIZE			0x00200000
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE)

#define BL33_BASE			0xaf400000
#define BL33_SIZE			0x00400000

#define PLAT_QTI_FIP_IOBASE		0xaf000000
#define PLAT_QTI_FIP_MAXSIZE		0x00400000

#ifdef SHARED_IMEM_BASE
#define TFA_BL31_SHARED_IMEM_TFA_AREA_BASE      (SHARED_IMEM_BASE + 0x734 + 340)

#define TFA_BL31_IMEM_ADDR(offset) \
	(TFA_BL31_SHARED_IMEM_TFA_AREA_BASE + (offset))

/*
 * Platform slot that stores the 64-bit address of the TF-A ring buffer.
 * On Lemans this resolves to SHARED_IMEM_BASE + 0xCF0.
 */
#define TFA_BL31_RING_LOG_BASE      (SHARED_IMEM_BASE + 0xCF0)

#endif /* SHARED_IMEM_BASE */

/*----------------------------------------------------------------------------*/
/* SCMI transport configuration */
/*----------------------------------------------------------------------------*/
#define PLAT_SCMI_MBX_MEM_BASE                  (0x18509000U)
#define PLAT_SCMI_DB_REG_ADDR                   (0x17c00008U)
#define PLAT_SCMI_DB_OSM_IPC_SHIFT              (28U)

#endif /* PLATFORM_DEF_H */
