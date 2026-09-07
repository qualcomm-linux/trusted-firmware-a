/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTISECLIB_DEFS_PLAT_H
#define QTISECLIB_DEFS_PLAT_H

/* Chipset specific secure interrupt number/ID defs. */
#define QTISECLIB_INT_ID_SEC_WDOG_BARK			(0x112)
#define QTISECLIB_INT_ID_NON_SEC_WDOG_BITE		(0x1)

#define QTISECLIB_INT_ID_VMIDMT_ERR_CLT_SEC		(0x177)
#define QTISECLIB_INT_ID_VMIDMT_ERR_CLT_NONSEC		(0x178)
#define QTISECLIB_INT_ID_VMIDMT_ERR_CFG_SEC		(0x179)
#define QTISECLIB_INT_ID_VMIDMT_ERR_CFG_NONSEC		(0x17A)

#define QTISECLIB_INT_ID_XPU_SEC			(0x174)
#define QTISECLIB_INT_ID_XPU_NON_SEC			(0x175)

/* NOC Interrupt */
#define QTISECLIB_INT_ID_SYSTEM_NOC_ERROR		(0xF9)
#define QTISECLIB_INT_ID_CONFIG_NOC_ERROR		(0xFA)
#define QTISECLIB_INT_ID_MEM_NOC_ERROR			(0xFC)
#define QTISECLIB_INT_ID_DC_NOC_ERROR			(0x11D)

#endif /* QTISECLIB_DEFS_PLAT_H */
