/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include "fuseprov_port_stub.h"

/* In-memory fuse storage for off-target testing */
static uint32_t stub_fuse_storage[1024][2] = {0};

/* Stub transport: read from in-memory storage */
static fuseprov_err_t stub_read_row(void *ctx, uint32_t addr,
				    fuseprov_addr_space_t space,
				    uint32_t out[2])
{
	(void)ctx;
	(void)space;

	if (addr >= 1024) {
		ERROR("Fuseprov stub: address out of range\n");
		return FUSEPROV_ERR_ADDR_INVALID;
	}

	out[0] = stub_fuse_storage[addr][0];
	out[1] = stub_fuse_storage[addr][1];

	return FUSEPROV_OK;
}

/* Stub transport: write to in-memory storage */
static fuseprov_err_t stub_write_rows(void *ctx, const uint32_t addr[],
				      const uint64_t data[], uint32_t count,
				      uintptr_t *addr_err)
{
	(void)ctx;

	for (uint32_t i = 0; i < count; i++) {
		if (addr[i] >= 1024) {
			ERROR("Fuseprov stub: address out of range\n");
			if (addr_err)
				*addr_err = addr[i];
			return FUSEPROV_ERR_ADDR_INVALID;
		}

		/* Fuses are write-once: OR new bits with existing */
		stub_fuse_storage[addr[i]][0] |= (uint32_t)(data[i]);
		stub_fuse_storage[addr[i]][1] |= (uint32_t)(data[i] >> 32);
	}

	return FUSEPROV_OK;
}

/* Get the stub transport instance */
const fuseprov_transport_t *fuseprov_port_stub_get(void)
{
	static const fuseprov_transport_t stub_transport = {
		.read_row = stub_read_row,
		.write_rows = stub_write_rows,
		.ctx = NULL,
	};

	return &stub_transport;
}

/* Reset stub storage for testing */
void fuseprov_port_stub_reset(void)
{
	for (int i = 0; i < 1024; i++) {
		stub_fuse_storage[i][0] = 0;
		stub_fuse_storage[i][1] = 0;
	}
}

/* Get fuse value from stub storage (for testing) */
void fuseprov_port_stub_get_fuse(uint32_t addr, uint32_t out[2])
{
	if (addr < 1024) {
		out[0] = stub_fuse_storage[addr][0];
		out[1] = stub_fuse_storage[addr][1];
	}
}
