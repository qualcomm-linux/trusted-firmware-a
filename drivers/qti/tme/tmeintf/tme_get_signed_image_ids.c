/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*
 * Retrieves the list of software image IDs signed by a given signing
 * authority, over TMECOM.
 */

#include <stddef.h>
#include <stdint.h>

#include <lib/libc/errno.h>
#include "tme_interfaces.h"
#include "tme_interfaces_defs.h"
#include "tme_message.h"
#include "tme_messages_tags.h"

int tme_get_signed_image_ids(tmeSoftwareRootCaIds signing_authority,
		uint32_t            *output_sw_ids,
		size_t               output_sw_id_max,
		size_t              *output_sw_id_count)
{
	int                 ret         = -EIO;
	size_t              response_len = 0;
	tmeSignedSwIdsReq_t request     = {.signing_authority = (uint32_t)signing_authority};
	tmeSignedSwIdsRsp_t response    = {0};

	do {
		if ((output_sw_ids == NULL) || (output_sw_id_max == 0U) || (output_sw_id_count == NULL)) {
			ret = -EINVAL;
			break;
		}

		ret = transceive_message(TME_MSG_CBOR_TAG_GET_SIGNED_IMAGE_IDS,
			&request,
			sizeof(request),
			&response,
			sizeof(response),
			&response_len);
		if (ret != 0) {
			break;
		}

		if (response_len != sizeof(response)) {
			ret = -EIO;
			break;
		}

		if (response.status != 0U) {
			ret = -EIO;
			break;
		}

		if (response.sw_id_count > TME_SIGNED_IMAGE_SWIDS_MAX) {
			ret = -EIO;
			break;
		}

		if (response.sw_id_count > output_sw_id_max) {
			ret = -E2BIG;
			break;
		}

		for (size_t sw_id = 0U; sw_id < response.sw_id_count; ++sw_id) {
			output_sw_ids[sw_id] = response.sw_ids[sw_id];
		}

		*output_sw_id_count = response.sw_id_count;
		ret              = 0;
	} while (0);

	return ret;
}
