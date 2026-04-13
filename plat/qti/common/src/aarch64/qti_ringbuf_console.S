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

#include <asm_macros.S>
#include <console_macros.S>

#include <platform_def.h>
#include <qti_ringbuf_console.h>

       .extern plat_qti_console_ring_buf;
       .extern plat_qti_console_buf_size;
/*
 * This driver implements console logging into a ring buffer.
 */

       .globl qti_console_ringbuf_register

       /* -----------------------------------------------
        * int qti_console_ringbuf_register(console_t *console,
        *                                  uintptr_t ringbuf_base_addr)
        * Registers ringbuf console instance.
        * In:  x0 - pointer to empty console_t struct
        *      x1 - start address of ringbuf block.
        * Out: x0 - 1 to indicate success
        * Clobber list: x0, x1, x14
        * -----------------------------------------------
        */
func qti_console_ringbuf_register
       str     x1, [x0, #CONSOLE_T_BASE]       /* Save addr of ringbuf struct */
       finish_console_register ringbuf putc=1, flush=0
endfunc qti_console_ringbuf_register

       /* -----------------------------------------------
        * int qti_console_ringbuf_puts(int c, console_t *console)
        * Writes a character to the ringbuf console.
        * The character must be preserved in x0.
        * In: x0 - character to be stored
        *     x1 - pointer to console_t struct
        * Clobber list: x1, x2, x16-x17 (per the porting guide)
        * -----------------------------------------------
        */
func console_ringbuf_putc
       /* set x1 = UART base. */
       ldr     x1, [x1, #CONSOLE_T_BASE]
       ldr     x2, [x1, #CONSOLE_RINGBUF_T_SIZE]
       ldr     x16, [x1, #CONSOLE_RINGBUF_T_POS]
       add     x17, x1, #CONSOLE_RINGBUF_T_BUF
       add     x17, x17, x16
       strb    w0, [x17]
       add     x16, x16, #1
       cmp     x16, x2
       b.lt    1f
       mov     x16, xzr

1:
       str     x16, [x1, #CONSOLE_RINGBUF_T_POS]
       ret
endfunc        console_ringbuf_putc

       /* -----------------------------------------------
        * int qti_console_ringbuf_flush(console_t *console)
        * In:  x0 - pointer to console_t struct
        * Out: x0 - 0 for success
        * Clobber list: x0, x1
        * -----------------------------------------------
        */
func console_ringbuf_flush
       mov     w0, #0
       ret
endfunc console_ringbuf_flush
