/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stddef.h>
#include <stdint.h>

#include <lib/libc/errno.h>

#include <tmecom.h>
#include <tmecom_tfa.h>
#include <tmecom_interfaces.h>

#include "tme_interfaces.h"

int tme_passthrough(void *req_buf,
		   size_t      req_size,
		   void       *rsp_buf,
		   size_t     *rsp_buf_size)
{
	tmecom_client_t *client_ptr = NULL;
	int           ret = -EINVAL;

	do {
		if ((req_buf == NULL) ||
		    (req_size == 0U) || (req_size > TMECOM_MAX_REQUEST_SIZE) ||
		    (rsp_buf == NULL) || (rsp_buf_size == NULL) ||
		    (*rsp_buf_size == 0U) ||
		    (*rsp_buf_size > TMECOM_MAX_RESPONSE_SIZE)) {
			break;
		}

		ret = tmecom_interface_init(&client_ptr);
		if (ret != 0) {
			break;
		}
		if (client_ptr == NULL) {
			ret = -EIO;
			break;
		}

		ret = tmecom_client_send_message_sync(client_ptr,
						  (void *)req_buf,
						  req_size,
						  rsp_buf,
						  rsp_buf_size,
						  TMECOM_RESPONSE_TIMEOUT_MS);
	} while (0);

	return ret;
}

int tme_passthrough_cmd(void *req_buf, size_t req_size, uint32_t *handle)
{
	tmecom_client_t *client_ptr = NULL;
	int           ret        = -EINVAL;

	do {
		if ((req_buf == NULL) ||
		    (req_size == 0U) || (req_size > TMECOM_MAX_REQUEST_SIZE) ||
		    (handle == NULL)) {
			break;
		}

		ret = tmecom_interface_init(&client_ptr);
		if (ret != 0) {
			break;
		}
		if (client_ptr == NULL) {
			ret = -EIO;
			break;
		}

		ret = tmecom_client_send_message_async(client_ptr,
						   (void *)req_buf,
						   req_size,
						   handle);
	} while (0);

	return ret;
}

int tme_passthrough_await(uint32_t handle, void *rsp_buf, size_t *rsp_buf_size)
{
	tmecom_client_t *client_ptr = NULL;
	int           ret        = -EINVAL;

	do {
		if ((rsp_buf == NULL) || (rsp_buf_size == NULL) ||
		    (*rsp_buf_size == 0U) ||
		    (*rsp_buf_size > TMECOM_MAX_RESPONSE_SIZE)) {
			break;
		}

		ret = tmecom_interface_init(&client_ptr);
		if (ret != 0) {
			break;
		}
		if (client_ptr == NULL) {
			ret = -EIO;
			break;
		}

		ret = tmecom_client_recv_message_async(client_ptr, handle, rsp_buf,
							       rsp_buf_size);
	} while (0);

	return ret;
}
