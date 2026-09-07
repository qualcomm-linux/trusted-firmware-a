/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/qti/fuseprov/fuseprov_port_tme.h>

#include "IxErrno.h"
#include "TmeInterfaces.h"
#include "TmeInterfacesDefs.h"

/* TME transport: read fuse row via TME.
 *
 * fuseprov_addr_space_t values (FUSEPROV_ADDR_RAW=0, FUSEPROV_ADDR_CORR=1)
 * are numerically identical to TmeQfpromAddrSpace_t, so a direct cast is
 * safe.
 */
static fuseprov_err_t tme_read_row(void *ctx, uint32_t addr,
				   fuseprov_addr_space_t space,
				   uint32_t out[2])
{
	uint32_t qfprom_status = 0;
	int ret;

	(void)ctx;

	ret = TmeFuseRead((TmeQfpromAddrSpace_t)space, addr, out, &qfprom_status);
	if (ret != E_SUCCESS || qfprom_status != TME_QFPROM_NO_ERR) {
		ERROR("Fuseprov: TME fuse read failed at addr 0x%x (ret=%d, qfprom_status=%u)\n",
		      addr, ret, qfprom_status);
		return FUSEPROV_ERR_TRANSPORT;
	}

	return FUSEPROV_OK;
}

/* TME transport: write fuse rows via TME. */
static fuseprov_err_t tme_write_rows(void *ctx, const uint32_t addr[],
				     const uint64_t data[], uint32_t count,
				     uintptr_t *addr_err)
{
	uint32_t qfprom_status = 0;
	TMEFuse_t fuses[TME_MAX_FUSE_WRITE_REQ];
	uint32_t i;
	int ret;

	(void)ctx;

	if (count > TME_MAX_FUSE_WRITE_REQ) {
		ERROR("Fuseprov: too many fuses to write (%u > %u)\n",
		      count, TME_MAX_FUSE_WRITE_REQ);
		return FUSEPROV_ERR_ADDR_INVALID;
	}

	for (i = 0; i < count; i++) {
		fuses[i].addr = addr[i];
		fuses[i].data[0] = (uint32_t)(data[i]);        /* LSB */
		fuses[i].data[1] = (uint32_t)(data[i] >> 32);  /* MSB */
	}

	ret = TmeFuseWriteMultiple(fuses, (size_t)count, &qfprom_status);
	if (ret != E_SUCCESS || qfprom_status != TME_QFPROM_NO_ERR) {
		ERROR("Fuseprov: TME fuse write failed (ret=%d, qfprom_status=%u)\n",
		      ret, qfprom_status);
		(void)addr_err; /* TmeFuseWriteMultiple() does not report the failing address */
		return FUSEPROV_ERR_TRANSPORT;
	}

	return FUSEPROV_OK;
}

/* Get the TME transport instance */
const fuseprov_transport_t *fuseprov_port_tme_get(void)
{
	static const fuseprov_transport_t tme_transport = {
		.read_row = tme_read_row,
		.write_rows = tme_write_rows,
		.ctx = NULL,
	};

	return &tme_transport;
}
