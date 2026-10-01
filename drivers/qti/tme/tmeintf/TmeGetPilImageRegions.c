/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*
 * PIL (Peripheral Image Loader) image region query over TMECOM.
 *
 * Copies only the entries TME actually reported (response.region_list_count,
 * bounds-checked against the caller's capacity below) into region_list, so a
 * caller-supplied buffer smaller than TMECOM_PIL_IMAGES_MAX_REGIONS cannot be
 * overflowed.
 */

#include <stddef.h>
#include <stdint.h>
#include <stringl/stringl.h>

#include "IxErrno.h"
#include "TmeInterfaces.h"
#include "TmeInterfacesDefs.h"
#include "TmeMessage.h"
#include "TmeMessagesTags.h"
#include "tmecom_interfaces.h"

int tme_get_pil_image_regions(uint32_t       *const sw_id_count,
		 uint32_t       *const sw_ids,
		 uint32_t       *const region_list_count,
		 tmePilRegion_t *const region_list)
{
	int                        ret         = E_FAILURE;
	tmeGetPilImageRegionsReq_t request     = {0};
	tmeGetPilImageRegionsRsp_t response    = {0};
	size_t                     response_len = 0;
	uint32_t                   region_list_capacity;

	do {
		if ((region_list_count == NULL) || (region_list == NULL) ||
				(sw_ids == NULL) || (sw_id_count == NULL)) {
			ret = E_INVALID_ARG;
			break;
		}

		if ((*sw_id_count > TMECOM_PIL_IMAGES_MAX_SWIDS) ||
				(*region_list_count > TMECOM_PIL_IMAGES_MAX_REGIONS)) {
			ret = E_OUT_OF_RANGE;
			break;
		}

		if ((*sw_id_count == 0U) || (*region_list_count == 0U)) {
			ret = E_INVALID_ARG;
			break;
		}

		region_list_capacity = *region_list_count;

		request.sw_id_count = *sw_id_count;
		memscpy(request.sw_ids, sizeof(uint32_t) * (*sw_id_count),
		 sw_ids, sizeof(uint32_t) * (*sw_id_count));

		if (E_SUCCESS != transceive_message(TME_MSG_CBOR_TAG_GET_PIL_REGIONS,
		 &request,
		 sizeof(request),
		 &response,
		 sizeof(response),
		 &response_len)) {
			break;
		}

		if ((E_SUCCESS != response.status) || (response_len != sizeof(response))) {
			break;
		}

		if (response.region_list_count > TMECOM_PIL_IMAGES_MAX_REGIONS) {
			ret = E_BAD_DATA;
			break;
		}

		if (region_list_capacity < response.region_list_count) {
			ret = E_INVALID_ARG;
			break;
		}

		*region_list_count = response.region_list_count;
		memscpy(region_list, region_list_capacity * sizeof(tmePilRegion_t),
		 response.region_list,
		 response.region_list_count * sizeof(tmePilRegion_t));

		ret = E_SUCCESS;
	} while (0);

	return ret;
}
