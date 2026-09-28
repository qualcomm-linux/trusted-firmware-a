/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stddef.h>
#include <stdint.h>
#include <stringl.h>

#include <lib/libc/errno.h>

#include <tmecom.h>
#include <tmecom_interfaces.h>

#include "tme_interfaces.h"
#include "tme_message.h"

int tme_sha_digest(tme_hash_alg_id_t        in_hash_alg,
		 const uint8_t        *in_msg,
		 size_t                in_msg_len,
		 uint8_t              *out_digest,
		 size_t               *out_digest_len,
		 tme_extended_error_info_t *error_info)
{
	int ret = -EINVAL;

	do {
		if ((in_msg == NULL && in_msg_len != 0U) ||
		    !TME_ADDR_OK(in_msg, in_msg_len) ||
		    (out_digest == NULL) || (out_digest_len == NULL) ||
		    (*out_digest_len == 0) || (error_info == NULL)) {
			break;
		}

		tme_sha_rsp_t response     = {0};
		size_t      response_len = sizeof(response);
		tme_sha_req_t request      = {
			.algorithm = (uint32_t)in_hash_alg,
			.data      = (tme_com_addr_t)(uintptr_t)in_msg,
			.data_size = (uint32_t)in_msg_len,
			.key_id    = (uint32_t)TME_HA_INVALID,
		};

		ret = transceive_message(TME_MSG_CBOR_TAG_SHA_DIGEST,
					&request,
					 sizeof(request),
					 &response,
					 sizeof(response),
					 &response_len);
		if (0 != ret) {
			break;
		}

		if (sizeof(response) != response_len) {
			ret = -EIO;
			break;
		}

		ret = update_extended_error_info(error_info, response.info);
		if (0 != ret) {
			break;
		}

		*out_digest_len = memscpy(out_digest, *out_digest_len,
					  response.output, response.output_len);

		if (*out_digest_len != response.output_len) {
			ret = -ENOMEM;
			break;
		}

		ret = 0;
	} while (0);

	return ret;
}
