/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*
 * Generic TME access-control (AC) module invocation over TMECOM.
 *
 * Uses static scratch buffers for the CBOR request/response rather than the
 * stack or a heap allocator: TF-A has no heap, and BL31 on this platform has
 * only a 4KB per-core stack (PLATFORM_STACK_SIZE), nowhere near enough for
 * ~1.8KB+2KB scratch buffers.  Static reuse is safe here because, like every
 * other tmeintf call, only one TME request may be outstanding at a time -
 * see TmeMessage.c's own g_tmecom_msg_req/g_tmecom_msg_rsp for the same pattern.
 *
 * The wire format is transceive_message()'s usual outer tag(bstr(...))
 * envelope wrapped around an inner CBOR array [request_id, bstr(in_buffer),
 * out_size]; TME replies with an inner array [status, bstr(output)].  This is
 * a message-specific inner encoding, layered on top of - not a replacement
 * for - the outer envelope every tmeintf call goes through.
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
#include "qcbor.h"
#include "qcbor_spiffy_decode.h"

static uint8_t s_invoke_ac_req_buf[MAX_CBOR_REQ_LENGTH];
static uint8_t s_invoke_ac_rsp_buf[MAX_CBOR_RSP_LENGTH];

int tme_invoke_ac(uint32_t request_id,
		 uint8_t *in_buffer,
		 size_t   in_size,
		 uint8_t *out_buffer,
		 size_t   out_size)
{
	int    ret         = E_FAILURE;
	size_t request_len  = get_max_request_payload();
	size_t response_len = get_max_response_payload();

	do {
		if ((out_buffer == NULL) || (out_size == 0U) ||
				((in_buffer == NULL) && (in_size != 0U)) ||
				(in_size > get_max_request_payload()) ||
				(out_size > get_max_response_payload())) {
			ret = E_INVALID_ARG;
			break;
		}

		QCBOREncodeContext ECtx = {0};
		const UsefulBufC   in_data = {in_buffer, in_size};
		int                cbor_ret;

		QCBOREncode_Init(&ECtx, (UsefulBuf){s_invoke_ac_req_buf, request_len});

		QCBOREncode_OpenArray(&ECtx);
		QCBOREncode_AddUInt64(&ECtx, request_id);
		QCBOREncode_AddBytes(&ECtx, in_data);
		/* Tell TME how big our response buffer is, so it can size its reply. */
		QCBOREncode_AddUInt64(&ECtx, out_size);
		QCBOREncode_CloseArray(&ECtx);

		cbor_ret = QCBOREncode_FinishGetSize(&ECtx, &request_len);
		if (cbor_ret != QCBOR_SUCCESS) {
			break;
		}

		if (E_SUCCESS != transceive_message(TME_MSG_CBOR_TAG_INVOKE_AC,
		 s_invoke_ac_req_buf,
		 request_len,
		 s_invoke_ac_rsp_buf,
		 response_len,
		 &response_len)) {
			break;
		}

		QCBORDecodeContext DCtx;
		QCBORItem          array_item = {0};
		UsefulBufC         output    = {0};
		uint64_t           status    = 0;

		QCBORDecode_Init(&DCtx, (UsefulBufC){s_invoke_ac_rsp_buf, response_len},
		 QCBOR_DECODE_MODE_NORMAL);

		QCBORDecode_EnterArray(&DCtx, &array_item);
		if (array_item.val.uCount != 2U) {
			break;
		}

		QCBORDecode_GetUInt64(&DCtx, &status);
		QCBORDecode_GetByteString(&DCtx, &output);
		QCBORDecode_ExitArray(&DCtx);

		if (QCBOR_SUCCESS != QCBORDecode_GetError(&DCtx)) {
			break;
		}

		if (E_SUCCESS != status) {
			break;
		}

		/*
		 * Check the SOURCE length against out_buffer's capacity BEFORE copying -
		 * see TmeMessage.c's decode_message() for why comparing memscpy()'s
		 * return value (min(dst,src)) against out_size instead would always be
		 * true and silently accept a truncated copy.
		 */
		if (output.len > out_size) {
			ret = E_NO_MEMORY;
			break;
		}

		memscpy(out_buffer, out_size, output.ptr, output.len);
		ret = E_SUCCESS;
	} while (0);

	return ret;
}
