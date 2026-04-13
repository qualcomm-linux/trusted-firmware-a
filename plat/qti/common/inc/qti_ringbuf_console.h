/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020 The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Changes from Qualcomm Technologies, Inc. are provided under the following license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_RINGBUF_CONSOLE_H
#define QTI_RINGBUF_CONSOLE_H

#include <drivers/console.h>
#include <lib/utils_def.h>

#define CONSOLE_RINGBUF_T_SIZE (U(0) * REGSZ)
#define CONSOLE_RINGBUF_T_POS  (U(1) * REGSZ)
#define CONSOLE_RINGBUF_T_BUF  (U(2) * REGSZ)

#ifndef __ASSEMBLER__

#define PLAT_QTI_RING_BUF_SIZE 0x1000

typedef struct console_ringbuf {
       u_register_t rb_size;
       u_register_t rb_pos;
       uint8_t      rb_buffer[PLAT_QTI_RING_BUF_SIZE];
} console_ringbuf_t;


int qti_console_ringbuf_register(console_t *console, uintptr_t ringbuf_base_addr);

#endif /* __ASSEMBLER__ */

#endif /* QTI_RINGBUF_CONSOLE_H */
