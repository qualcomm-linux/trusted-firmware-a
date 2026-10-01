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

#include "IxErrno.h"
#include "TmeInterfaces.h"
#include "TmeInterfacesDefs.h"
#include "TmeMessage.h"
#include "TmeMessagesTags.h"
#include "tmecom_interfaces.h"

int tme_get_signed_image_ids(tmeSoftwareRootCaIds signing_authority,
		uint32_t            *output_sw_ids,
		size_t               output_sw_id_max,
		size_t              *output_sw_id_count)
{
	int                 ret         = E_FAILURE;
	size_t              response_len = 0;
	tmeSignedSwIdsReq_t request     = {.signing_authority = (uint32_t)signing_authority};
	tmeSignedSwIdsRsp_t response    = {0};

	do {
		if ((output_sw_ids == NULL) || (output_sw_id_max == 0U) || (output_sw_id_count == NULL)) {
			ret = E_INVALID_ARG;
			break;
		}

		if (E_SUCCESS != transceive_message(TME_MSG_CBOR_TAG_GET_SIGNED_IMAGE_IDS,
		 &request,
		 sizeof(request),
		 &response,
		 sizeof(response),
		 &response_len)) {
			break;
		}

		if (response_len != sizeof(response)) {
			break;
		}

		if (E_SUCCESS != response.status) {
			break;
		}

		if (response.sw_id_count > TME_SIGNED_IMAGE_SWIDS_MAX) {
			ret = E_BAD_DATA;
			break;
		}

		if (response.sw_id_count > output_sw_id_max) {
			ret = E_DATA_TOO_LARGE;
			break;
		}

		for (size_t sw_id = 0U; sw_id < response.sw_id_count; ++sw_id) {
			output_sw_ids[sw_id] = response.sw_ids[sw_id];
		}

		*output_sw_id_count = response.sw_id_count;
		ret              = E_SUCCESS;
	} while (0);

	return ret;
}
