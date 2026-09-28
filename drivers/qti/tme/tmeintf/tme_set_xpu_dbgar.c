/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*
 * Programs a set of XPU DBGAR (debug access region) addresses via TME.
 */

#include <stddef.h>
#include <stdint.h>

#include <lib/libc/errno.h>
#include "tme_interfaces.h"
#include "tme_interfaces_defs.h"
#include "tme_message.h"
#include "tme_messages_tags.h"

int tme_set_xpu_dbgar(uint32_t *dbgars, size_t count)
{
	int                 ret         = -EINVAL;
	tmeSetXpuDbgarRsp_t rsp         = {0};
	size_t              response_len = sizeof(rsp);

	if ((dbgars == NULL) || (count == 0U)) {
		goto bail;
	}

	if (count > (get_max_request_payload() / sizeof(*dbgars))) {
		return -E2BIG;
	}

	ret = transceive_message(TME_MSG_CBOR_TAG_SET_XPU_DBG_AR,
		dbgars,
		count * sizeof(*dbgars),
		&rsp,
		sizeof(rsp),
		&response_len);
	if (ret != 0) {
		goto bail;
	}

	if (response_len != sizeof(rsp)) {
		ret = -EIO;
		goto bail;
	}

	ret = (rsp.status == 0U) ? 0 : -EIO;

bail:
	return ret;
}
