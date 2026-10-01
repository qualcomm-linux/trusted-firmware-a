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

#include "IxErrno.h"
#include "TmeInterfaces.h"
#include "TmeInterfacesDefs.h"
#include "TmeMessage.h"
#include "TmeMessagesTags.h"
#include "tmecom_interfaces.h"

int tme_update_rollback_version(void)
{
	int                           ret         = E_FAILURE;
	tmeUpdateRollbackVersionRsp_t rsp         = {0};
	size_t                        response_len = sizeof(rsp);

	/* Empty request payload - see tmeUpdateRollbackVersionRsp_t. */
	CHECK_BAIL(E_SUCCESS == transceive_message(TME_MSG_CBOR_TAG_UPDATE_ROLLBACK_VERSION,
		NULL,
		0,
		&rsp,
		sizeof(rsp),
		&response_len));

	CHECK_BAIL(response_len == sizeof(rsp));

	ret = (0 == rsp.status) ? E_SUCCESS : E_FAILURE;

bail:
	return ret;
}
