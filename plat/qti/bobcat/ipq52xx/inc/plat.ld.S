/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_LD_S
#define PLAT_LD_S

MEMORY {
    IMEM_RING_LOG (rw): ORIGIN = QTI_IMEM_RING_LOG_BASE, LENGTH = QTI_IMEM_RING_LOG_SIZE
}

SECTIONS {
    imem_rw_ring_log (NOLOAD) : {
        . = ALIGN(8);
        __QTI_RING_LOG_START__ = .;
        . = . + QTI_IMEM_RING_LOG_SIZE;
        __QTI_RING_LOG_END__ = .;
    } >IMEM_RING_LOG

    ASSERT(__QTI_RING_LOG_START__ == QTI_IMEM_RING_LOG_BASE,
        "TF-A ring buffer is not at the start of the IMEM reservation.")
    ASSERT(__QTI_RING_LOG_END__ <= QTI_IMEM_RING_LOG_BASE + QTI_IMEM_RING_LOG_SIZE,
        "TF-A ring buffer has exceeded the IMEM reservation.")
}

#endif /* PLAT_LD_S */
