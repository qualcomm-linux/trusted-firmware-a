/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stddef.h>
#include <stdint.h>

#include "TmeInterfaces.h"
#include "tmecom_interfaces.h"
#include "tmecom.h"
#include "tmecomTFA.h"
#include "IxErrno.h"

int tme_passthrough_await(uint32_t handle, void *rsp_buf, size_t *rsp_buf_size)
{
	tmecomClient *client_ptr = NULL;
	int           ret       = E_INVALID_ARG;

	CHECK_BAIL(rsp_buf);
	CHECK_BAIL(rsp_buf_size);
	CHECK_BAIL(*rsp_buf_size > 0U && *rsp_buf_size <= TMECOM_MAX_RESPONSE_SIZE);

	ret = tmecom_interface_init(&client_ptr);
	if (ret != E_SUCCESS || client_ptr == NULL) {
		return E_FAILURE;
	}

	ret = tmecom_client_recv_message_async(client_ptr, handle, rsp_buf, rsp_buf_size);
	if (ret == 0) {
		return E_SUCCESS;
	}
	else if (ret == -E_IN_PROGRESS) {
		return E_IN_PROGRESS ;
	}

	/* Covers -EINVAL (invalid/stale handle). */
	return E_FAILURE;

bail:
	return ret;
}
