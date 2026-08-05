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
#include <lib/cassert.h>
#include <lib/utils_def.h>

#define CONSOLE_RINGBUF_T_VERSION	(U(0) * U(4))
#define CONSOLE_RINGBUF_T_MAGIC		(U(1) * U(4))
#define CONSOLE_RINGBUF_T_BUF_SIZE	(U(3) * U(4))
#define CONSOLE_RINGBUF_T_WRAP		(U(4) * U(4))
#define CONSOLE_RINGBUF_T_HEAD		(U(5) * U(4))
#define CONSOLE_RINGBUF_T_LOG_BUF	(U(6) * U(4))


#ifndef __ASSEMBLER__

#define TFA_BUFFER_REGION_SIZE	0x1000  /* 4k bytes */
#define PLAT_QTI_RING_BUF_SIZE	\
	(TFA_BUFFER_REGION_SIZE - 32) /* 4k - 32 (24 bytes for members) bytes */
#define DIAG_VERSION		1          /* Structure version */
#define DIAG_MAGIC_INIT		0x47414944 /* "DIAG" in ASCII, buffer initialized and ready */
#define DIAG_MAGIC_FAILED	0xDEADBEEF /* Initialization failed */
#define DIAG_MAGIC_DLOAD	0xD15AB1ED /* DLOAD mode - logging disabled */
#define DLOAD_MAGIC_COOKIE	0x10       /* Download mode detection value */

/**
 * struct diag_hdr - Diagnostic region identity header
 *
 * @version:    Layout version of the diagnostic region (DIAG_VERSION)
 * @magic:      State indicator written during init; one of DIAG_MAGIC_INIT,
 *              DIAG_MAGIC_FAILED, or DIAG_MAGIC_DLOAD
 */
struct diag_hdr {
	uint32_t version;
	uint32_t magic;
};

/**
 * struct diag_conf - Ring buffer location descriptor
 *
 * @buf_offset: Byte offset from the start of the diagnostic region to the
 *              first byte of the ring buffer data
 * @buf_size:   Usable size of the ring buffer in bytes (PLAT_QTI_RING_BUF_SIZE)
 */
struct diag_conf {
	uint32_t buf_offset;
	uint32_t buf_size;
};

/**
 * struct circ_wo_buf - Write-only circular log buffer
 *
 * @wrap:   Number of times the write head has wrapped around the buffer;
 *          used by readers to detect overrun
 * @head:   Byte offset of the next write position within @buf; advances
 *          monotonically and wraps at PLAT_QTI_RING_BUF_SIZE
 * @buf:    Raw log data storage (TFA_BUFFER_REGION_SIZE minus header overhead)
 */
struct circ_wo_buf {
	uint32_t wrap;
	uint32_t head;
	uint8_t buf[PLAT_QTI_RING_BUF_SIZE];
};

/**
 * struct console_ringbuf - Top-level diagnostic region
 *
 * @hdr:     Identity header — version and magic sentinel
 * @conf:    Locates the ring buffer within the region (offset, size)
 * @wo_cbuf: Circular log buffer written by TF-A firmware
 */
typedef struct console_ringbuf {
	struct diag_hdr hdr;
	struct diag_conf conf;
	struct circ_wo_buf wo_cbuf;
} console_ringbuf_t;

/*
 * TF-A ringbuf console utility functions
 */

/**
 * Registers a ringbuf console to the global console list
 *
 * @param[in]    console     console object ptr
 * @param[in]    ringbuf     ringbuf object ptr
 * @return - cluster id if found successful, otherwise return error
 */
int qti_console_ringbuf_register(console_t *console, console_ringbuf_t *ringbuf);

/**
 * Initializes a TF-A ringbuf object
 *
 * @param[in]    ringbuf     ringbuf obj ptr
 */
void qti_console_ringbuf_init(console_ringbuf_t *ringbuf);


#endif /* __ASSEMBLER__ */

#endif /* QTI_RINGBUF_CONSOLE_H */
