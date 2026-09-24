/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FUSEPROV_TRANSPORT_H
#define FUSEPROV_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>

/* Address space for fuse read/write operations */
typedef enum {
	FUSEPROV_ADDR_RAW  = 0,  /* Raw (uncorrected) address space */
	FUSEPROV_ADDR_CORR = 1,  /* Corrected address space */
} fuseprov_addr_space_t;

/* Low-level transport error codes */
typedef enum {
	FUSEPROV_OK = 0,
	FUSEPROV_ERR_ADDR_INVALID,
	FUSEPROV_ERR_NOT_READABLE,
	FUSEPROV_ERR_NOT_WRITEABLE,
	FUSEPROV_ERR_FEC_ENABLED_NOT_WRITEABLE,
	FUSEPROV_ERR_VERIFY_FAILED,
	FUSEPROV_ERR_ROW_BOUNDARY,
	FUSEPROV_ERR_TRANSPORT,  /* RMB/IPC failure, timeout, etc. */
	FUSEPROV_ERR_UNKNOWN,
} fuseprov_err_t;

/* Transport contract: function pointers for fuse read/write operations */
typedef struct {
	/* Read a single fuse row from QFPROM
	 * @ctx: opaque context pointer (port-specific)
	 * @addr: fuse row address
	 * @space: address space (raw or corrected)
	 * @out: output buffer for row data [LSB, MSB]
	 * @return: FUSEPROV_OK on success, error code on failure
	 */
	fuseprov_err_t (*read_row)(void *ctx, uint32_t addr,
				   fuseprov_addr_space_t space, uint32_t out[2]);

	/* Write multiple fuse rows to QFPROM atomically
	 * @ctx: opaque context pointer (port-specific)
	 * @addr: array of fuse row addresses
	 * @data: array of fuse row data (64-bit: [LSB, MSB])
	 * @count: number of rows to write
	 * @addr_err: output parameter for address that failed (if any)
	 * @return: FUSEPROV_OK on success, error code on failure
	 */
	fuseprov_err_t (*write_rows)(void *ctx, const uint32_t addr[],
				     const uint64_t data[], uint32_t count,
				     uintptr_t *addr_err);

	/* Opaque context pointer passed to read_row/write_rows */
	void *ctx;
} fuseprov_transport_t;

#endif /* FUSEPROV_TRANSPORT_H */
