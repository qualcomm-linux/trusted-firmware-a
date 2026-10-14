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

#include <lib/libc/errno.h>
#include "tme_interfaces.h"
#include "tme_interfaces_defs.h"
#include "tme_message.h"
#include "tme_messages_tags.h"

int tme_write_config_register(tmeConfigRegisterId_e register_id, uint32_t value)
{
	int                         ret = -EIO;
	tmeWriteConfigRegisterReq_t req = {
		.id    = (uint8_t)register_id,
		.value = value,
	};
	tmeWriteConfigRegisterRsp_t rsp         = {0};
	size_t                      response_len = sizeof(rsp);

	ret = transceive_message(TME_MSG_CBOR_TAG_WRITE_CONFIG_REGISTER,
		&req,
		sizeof(req),
		&rsp,
		sizeof(rsp),
		&response_len);
	if (ret != 0) {
		goto bail;
	}

	if (response_len != sizeof(rsp)) {
		ret = -EIO;
		goto bail;
	}

	ret = (rsp.status == 0U) ? 0 : -EIO;

bail:
	return ret;
}
