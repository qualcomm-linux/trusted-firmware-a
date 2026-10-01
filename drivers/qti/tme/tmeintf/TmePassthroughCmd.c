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

int tme_passthrough_cmd(void *req_buf, size_t req_size, uint32_t *handle)
{
	tmecomClient *client_ptr = NULL;
	int           ret       = E_INVALID_ARG;

	CHECK_BAIL(req_buf);
	CHECK_BAIL(req_size > 0U && req_size <= TMECOM_MAX_REQUEST_SIZE);
	CHECK_BAIL(handle);

	ret = tmecom_interface_init(&client_ptr);
	if (ret != E_SUCCESS || client_ptr == NULL) {
		return E_FAILURE;
	}

	ret = tmecom_client_send_message_async(client_ptr,
		 (void *)req_buf,
		 req_size,
		 handle);
	if (ret == 0) {
		return E_SUCCESS;
	}
	else if (ret == -E_AGAIN) {
		return E_AGAIN;
	}

	return E_FAILURE;

bail:
	return ret;
}
