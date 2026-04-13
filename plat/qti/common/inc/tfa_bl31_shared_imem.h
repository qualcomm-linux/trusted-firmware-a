/*
 * Copyright (c) 2018-2020, 2025. The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Changes from Qualcomm Technologies, Inc. are provided under the following license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TFA_BL31_SHARED_IMEM_H
#define TFA_BL31_SHARED_IMEM_H

#include <bl31qtilib_defs_plat.h>

#define TFA_BL31_SHARED_IMEM_TFA_AREA_BASE      (SHARED_IMEM_BASE + 0x734 + 340)

#define TFA_BL31_IMEM_ADDR(offset) \
        (TFA_BL31_SHARED_IMEM_TFA_AREA_BASE + (offset))

#define TFA_BL31_SHARED_IMEM_RING_BUF_BASE      TFA_BL31_IMEM_ADDR(0x0) /* 8 bytes */
#define TFA_BL31_SHARED_IMEM_RING_BUF_SIZE      TFA_BL31_IMEM_ADDR(0x10) /* 4 bytes */

/* Next available: TFA_BL31_IMEM_ADDR(0x18), 8-byte aligned */
/* RESERVED until TFA_BL31_IMEM_ADDR(0x2c), 20 bytes available*/

#endif /* TFA_BL31_SHARED_IMEM_H */
