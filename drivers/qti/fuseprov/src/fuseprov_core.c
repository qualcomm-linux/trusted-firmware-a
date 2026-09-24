/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/qti/fuseprov/fuseprov_port.h>

/* Transport wrapper: read a single fuse row
 * Maps transport errors to parser-level errors
 */
fuseprov_err_t fuseprov_row_read(const fuseprov_transport_t *t,
				 uint32_t addr,
				 fuseprov_addr_space_t space,
				 uint32_t out[2])
{
	if (!t || !t->read_row || !out) {
		ERROR("Fuseprov: invalid transport or output buffer\n");
		return FUSEPROV_ERR_ADDR_INVALID;
	}

	return t->read_row(t->ctx, addr, space, out);
}

/* Transport wrapper: write multiple fuse rows atomically
 * Maps transport errors to parser-level errors
 */
fuseprov_err_t fuseprov_rows_write(const fuseprov_transport_t *t,
				   const uint32_t addr[],
				   const uint64_t data[],
				   uint32_t count,
				   uintptr_t *addr_err)
{
	if (!t || !t->write_rows) {
		ERROR("Fuseprov: invalid transport\n");
		return FUSEPROV_ERR_TRANSPORT;
	}

	if (count == 0)
		return FUSEPROV_OK;

	if (!addr || !data) {
		ERROR("Fuseprov: invalid address or data array\n");
		return FUSEPROV_ERR_ADDR_INVALID;
	}

	return t->write_rows(t->ctx, addr, data, count, addr_err);
}
