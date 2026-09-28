/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*
 * Antirollback version commit over TMECOM.
 */

#include <stddef.h>
#include <stdint.h>

#include <lib/libc/errno.h>
#include "tme_interfaces.h"
#include "tme_interfaces_defs.h"
#include "tme_message.h"
#include "tme_messages_tags.h"

int tme_update_rollback_version(void)
{
	int                           ret         = -EIO;
	tmeUpdateRollbackVersionRsp_t rsp         = {0};
	size_t                        response_len = sizeof(rsp);

	/* Empty request payload - see tmeUpdateRollbackVersionRsp_t. */
	ret = transceive_message(TME_MSG_CBOR_TAG_UPDATE_ROLLBACK_VERSION,
		NULL,
		0,
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
