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

int tme_forward_request(void *req_buf,
		size_t      req_size,
		void       *rsp_buf,
		size_t     *rsp_buf_size)
{
	tmecomClient *client_ptr = NULL;
	int           ret       = E_INVALID_ARG;

	CHECK_BAIL(req_buf);
	CHECK_BAIL(req_size > 0U && req_size <= TMECOM_MAX_REQUEST_SIZE);
	CHECK_BAIL(rsp_buf);
	CHECK_BAIL(*rsp_buf_size > 0U && *rsp_buf_size <= TMECOM_MAX_RESPONSE_SIZE);

	ret = tmecom_interface_init(&client_ptr);
	if (ret != E_SUCCESS || client_ptr == NULL) {
		return E_FAILURE;
	}

	ret = tmecom_client_send_message_sync(client_ptr,
		(void *)req_buf,
		req_size,
		rsp_buf,
		rsp_buf_size,
		TMECOM_RESPONSE_TIMEOUT_MS);
	return (ret == 0) ? E_SUCCESS : E_FAILURE;

bail:
	return ret;
}