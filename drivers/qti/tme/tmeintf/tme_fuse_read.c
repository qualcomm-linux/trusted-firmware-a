/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*
 * QFPROM fuse read over TMECOM.
 *
 * The write side lives in tme_fuse_write_multiple.c - read its header before
 * touching it, fuses are one-time-programmable.
 */

#include <stddef.h>
#include <stdint.h>
#include <stringl.h>

#include <lib/libc/errno.h>
#include "tme_interfaces.h"
#include "tme_interfaces_defs.h"
#include "tme_message.h"
#include "tme_messages_tags.h"

int tme_fuse_read(TmeQfpromAddrSpace_t addr_type,
		uint32_t             fuse_addr,
		uint32_t *const      fuse_data,
		uint32_t *const      qfprom_api_status)
{
	int              ret         = -EINVAL;
	tmeFuseReadReq_t fuse_read_req = {0};
	tmeFuseReadRsp_t fuse_read_rsp = {0};
	size_t           response_len = sizeof(fuse_read_rsp);

	if ((fuse_data == NULL) || (qfprom_api_status == NULL)) {
		goto bail;
	}

	fuse_read_req.addr_type = (uint32_t)addr_type;
	fuse_read_req.fuse_addr = fuse_addr;

	ret = transceive_message(TME_MSG_CBOR_TAG_FUSE_READ,
		&fuse_read_req,
		sizeof(fuse_read_req),
		&fuse_read_rsp,
		sizeof(fuse_read_rsp),
		&response_len);
	if (ret != 0) {
		goto bail;
	}

	if (response_len != sizeof(fuse_read_rsp)) {
		ret = -EIO;
		goto bail;
	}

	/*
	 * memscpy() copies min(dst,src) and returns that count, so compare against
	 * the SOURCE size to detect a short copy - the caller is required to supply
	 * room for the whole row (TME_QFPROM_FUSE_DATA_WORDS words).
	 */
	if (sizeof(fuse_read_rsp.fuse_data) !=
		memscpy(fuse_data,
		sizeof(fuse_read_rsp.fuse_data),
		fuse_read_rsp.fuse_data,
		sizeof(fuse_read_rsp.fuse_data))) {
		ret = -EIO;
		goto bail;
	}

	*qfprom_api_status = fuse_read_rsp.qfprom_api_status;

	ret = 0;

bail:
	return ret;
}
