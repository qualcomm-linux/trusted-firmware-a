/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*
 * QFPROM configuration register write over TMECOM.
 */

#include <stddef.h>
#include <stdint.h>

#include "IxErrno.h"
#include "TmeInterfaces.h"
#include "TmeInterfacesDefs.h"
#include "TmeMessage.h"
#include "TmeMessagesTags.h"
#include "tmecom_interfaces.h"

int tme_write_config_register(tmeConfigRegisterId_e register_id, uint32_t value)
{
	int                         ret = E_FAILURE;
	tmeWriteConfigRegisterReq_t req = {
		.id    = (uint8_t)register_id,
		.value = value,
	};
	tmeWriteConfigRegisterRsp_t rsp         = {0};
	size_t                      response_len = sizeof(rsp);

	CHECK_BAIL(E_SUCCESS == transceive_message(TME_MSG_CBOR_TAG_WRITE_CONFIG_REGISTER,
		&req,
		sizeof(req),
		&rsp,
		sizeof(rsp),
		&response_len));

	CHECK_BAIL(response_len == sizeof(rsp));

	ret = (0 == rsp.status) ? E_SUCCESS : E_FAILURE;

bail:
	return ret;
}
