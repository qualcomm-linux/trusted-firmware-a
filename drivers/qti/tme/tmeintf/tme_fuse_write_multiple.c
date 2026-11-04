/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*
 * QFPROM multi-row fuse write over TMECOM.
 *
 * ###########################################################################
 * # QFPROM FUSES ARE ONE-TIME-PROGRAMMABLE.                                 #
 * #                                                                         #
 * # Every set bit in a TMEFuse_t.data[] word permanently blows that fuse bit #
 * # on real silicon.  It cannot be un-blown, the part cannot be recovered,   #
 * # and blowing the wrong row can brick the device or lock it out of secure  #
 * # boot.  An all-zero data[] blows nothing and is the only value that is    #
 * # safe to send speculatively.                                             #
 * #                                                                         #
 * # This function does not and cannot validate the caller's intent - TME FW  #
 * # will happily program whatever it is handed, subject only to its own      #
 * # region permission checks.  The responsibility is entirely at the call    #
 * # site.                                                                   #
 * ###########################################################################
 *
 * The request is built in a file-scope static rather than on the stack or
 * via heap allocation: the struct is ~772 bytes (see
 * tmeFuseWriteMultipleReq_t), too much for a BL31 stack frame, and TF-A has
 * no heap.  This is safe here: BL31 is single-threaded and TME processes one
 * request at a time, so at most one fuse-write request is ever in flight.
 *
 * Error codes follow the TMECOM convention: 0 on success, negative errno
 * values on failure.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <lib/libc/errno.h>
#include "tme_interfaces.h"
#include "tme_interfaces_defs.h"
#include "tme_message.h"
#include "tme_messages_tags.h"

/*
 * Request scratch.  Static rather than automatic - see the note in the file
 * header about the struct size and the absence of a heap in TF-A.
 */
static tmeFuseWriteMultipleReq_t s_fuse_write_req;

int tme_fuse_write_multiple(TMEFuse_t      *fuse_array,
		 size_t          fuse_array_len,
		 uint32_t *const qfprom_api_status)
{
	int                       ret         = -EIO;
	tmeFuseWriteMultipleRsp_t rsp         = {0};
	size_t                    response_len = sizeof(rsp);
	size_t                    i;

	if ((fuse_array == NULL) || (qfprom_api_status == NULL)) {
		return -EINVAL;
	}

	if (fuse_array_len == 0U) {
		return -EINVAL;
	}

	if (fuse_array_len > TME_MAX_FUSE_WRITE_REQ) {
		return -E2BIG;
	}

	*qfprom_api_status = TME_QFPROM_STATUS_UNSET;

	/*
	 * The scratch buffer is reused across calls and the whole fixed-size struct
	 * goes on the wire, so clear it first: without this, row addresses left in
	 * the unused tail by an earlier request would be re-presented to TME FW.
	 * TME FW only honours the first fuse_array_len entries, but do not rely on
	 * that to keep stale addresses harmless.
	 */
	memset(&s_fuse_write_req, 0, sizeof(s_fuse_write_req));

	s_fuse_write_req.fuse_array_len = (uint32_t)fuse_array_len;

	for (i = 0U; i < fuse_array_len; i++) {
		s_fuse_write_req.fuse_array[i].addr    = fuse_array[i].addr;
		s_fuse_write_req.fuse_array[i].data[0] = fuse_array[i].data[0];
		s_fuse_write_req.fuse_array[i].data[1] = fuse_array[i].data[1];
	}

	ret = transceive_message(TME_MSG_CBOR_TAG_FUSE_WRITE_MULTIPLE,
		&s_fuse_write_req,
		sizeof(s_fuse_write_req),
		&rsp,
		sizeof(rsp),
		&response_len);

	if (ret != 0) {
		return ret;
	}

	if (response_len != sizeof(rsp)) {
		return -EIO;
	}

	/*
	 * The caller's qfprom_api_status receives rsp.status (TME's handler status),
	 * not rsp.addr_err.  rsp.addr_err carries the qfprom driver's address/error
	 * detail and is dropped here - surface it through a wider signature if it
	 * is ever needed for diagnostics.
	 */
	*qfprom_api_status = rsp.status;

	return (rsp.status == TME_QFPROM_NO_ERR) ? 0 : -EIO;
}
