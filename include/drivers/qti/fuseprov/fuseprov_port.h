/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FUSEPROV_PORT_H
#define FUSEPROV_PORT_H

#include <stdint.h>
#include "fuseprov_transport.h"
#include "fuseprov_sec_elf_v3.h"

/* Transport wrapper functions - implemented in fuseprov_core.c */

/* Read a single fuse row from QFPROM
 * @t: transport contract pointer
 * @addr: fuse row address
 * @space: address space (raw or corrected)
 * @out: output buffer for row data [LSB, MSB]
 * @return: FUSEPROV_OK on success, error code on failure
 */
fuseprov_err_t fuseprov_row_read(const fuseprov_transport_t *t,
				 uint32_t addr,
				 fuseprov_addr_space_t space,
				 uint32_t out[2]);

/* Write multiple fuse rows to QFPROM atomically
 * @t: transport contract pointer
 * @addr: array of fuse row addresses
 * @data: array of fuse row data (64-bit: [LSB, MSB])
 * @count: number of rows to write
 * @addr_err: output parameter for address that failed (if any)
 * @return: FUSEPROV_OK on success, error code on failure
 */
fuseprov_err_t fuseprov_rows_write(const fuseprov_transport_t *t,
				   const uint32_t addr[],
				   const uint64_t data[],
				   uint32_t count,
				   uintptr_t *addr_err);

/* Main SEC.DAT v3 parser and fuse blower - implemented in fuseprov_sec_elf_v3.c
 * @t: transport contract pointer
 * @buf: SEC.DAT buffer
 * @len: SEC.DAT buffer length
 * @return: FUSEPROV_SUCCESS on success, error code on failure
 */
fuseprov_error_etype fuseprov_blow_fuses_sec_elf_v3(
	const fuseprov_transport_t *t,
	uint8_t *buf,
	uint32_t len);

#endif /* FUSEPROV_PORT_H */
