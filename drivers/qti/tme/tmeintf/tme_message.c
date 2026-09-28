/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stddef.h>
#include <stdint.h>
#include <stringl.h>

#include <bl31qtilib_cb_interface.h>
#include <common/debug.h>
#include <lib/libc/errno.h>
#include <qcbor.h>

#include <tmecom_tfa.h>
#include <tmecom_interfaces.h>

#include "tme_message.h"
#include "tme_messages_tags.h"

/*
 * In the TFA build, request/response message buffers are statically reserved
 * within the driver (mirroring tmecom.c's static allocation).
 */
static tmecom_msg_req_t g_tmecom_msg_req = { 0 };
static tmecom_msg_rsp_t g_tmecom_msg_rsp = { 0 };

static void *tme_msg_alloc_req(size_t size)
{
	if (size <= sizeof(g_tmecom_msg_req)) {
		return (void *)&g_tmecom_msg_req;
	}
	return NULL;
}

static void tme_msg_free_req(void *p_mem)
{
	/* No-op since the buffer is statically allocated. */
	(void)p_mem;
}

static void *tme_msg_alloc_rsp(size_t size)
{
	if (size <= sizeof(g_tmecom_msg_rsp)) {
		return (void *)&g_tmecom_msg_rsp;
	}
	return NULL;
}

static void tme_msg_free_rsp(void *p_mem)
{
	/* No-op since the buffer is statically allocated. */
	(void)p_mem;
}

size_t get_encoded_number_size(uint32_t number)
{
	size_t encoded_size = sizeof(uint8_t);

	if (number > 0xffff) {
		encoded_size += sizeof(uint32_t);
	} else if (number > 0xff) {
		encoded_size += sizeof(uint16_t);
	} else if (number >= 24) {
		encoded_size += sizeof(uint8_t);
	}
	return encoded_size;
}

size_t get_max_request_payload(void)
{
	return MAX_CBOR_REQ_LENGTH -
	       get_encoded_number_size(TME_MSG_CBOR_TAG_MAX) -
	       get_encoded_number_size(MAX_CBOR_REQ_LENGTH);
}

size_t get_max_response_payload(void)
{
	return MAX_CBOR_RSP_LENGTH -
	       get_encoded_number_size(TME_MSG_CBOR_TAG_MAX) -
	       get_encoded_number_size(MAX_CBOR_RSP_LENGTH);
}

int encode_message(uint32_t tag, const UsefulBufC message_buf,
			   UsefulBuf *encoded_buf)
{
	int                ret = -EILSEQ;
	QCBORError         qcbor_ret;
	size_t             encoded_len = 0;
	QCBOREncodeContext encode_ctx  = {0};

	do {
		if ((encoded_buf == NULL) ||
			((message_buf.ptr == NULL) && (message_buf.len != 0U))) {
			ret = -EINVAL;
			break;
		}

		QCBOREncode_Init(&encode_ctx, *encoded_buf);

		QCBOREncode_AddTag(&encode_ctx, tag);
		QCBOREncode_AddBytes(&encode_ctx, message_buf);

		qcbor_ret = QCBOREncode_FinishGetSize(&encode_ctx, &encoded_len);

		if (qcbor_ret != QCBOR_SUCCESS) {
			ERROR("TME: encode_message QCBOR encode failed: tag=0x%x ret=%d\n",
			      tag, qcbor_ret);
			break;
		}

		encoded_buf->len = encoded_len;
		ret = 0;
	} while (0);

	return ret;
}

int decode_message(uint32_t tag, const UsefulBufC encoded_buf,
			   UsefulBuf *message_buf)
{
	int                ret         = -EILSEQ;
	QCBORError         qcbor_ret;
	size_t             decoded_len = 0;
	QCBORDecodeContext decode_ctx  = {0};
	QCBORItem          item        = {0};

	do {
		if ((message_buf == NULL) ||
			((encoded_buf.ptr == NULL) && (encoded_buf.len != 0U)) ||
			((message_buf->ptr == NULL) && (message_buf->len != 0U))) {
			ret = -EINVAL;
			break;
		}

		QCBORDecode_Init(&decode_ctx, encoded_buf, QCBOR_DECODE_MODE_NORMAL);

		qcbor_ret = QCBORDecode_GetNext(&decode_ctx, &item);
		if (qcbor_ret != QCBOR_SUCCESS) {
			ERROR("TME: decode_message QCBOR decode failed: tag=0x%x ret=%d\n",
			      tag, qcbor_ret);
			break;
		}

		/* v1.2 QCBOR no longer exposes a single item.uTag field; fetch the
		* outermost tag (index 0) of the decoded item via the accessor. */
		uint64_t item_tag = QCBORDecode_GetNthTag(&decode_ctx, &item, 0);

		/* Either tag matches the message, or TME responded with an error tag. */
		if ((item_tag != tag) && (item_tag != TME_MSG_CBOR_TAG_ERROR)) {
			ERROR("TME: decode_message unexpected tag: got=0x%llx expected=0x%x\n",
				(unsigned long long)item_tag, tag);
			break;
		}

		/*
		* Generic errors are sent as integers.  A valid bstr message is handled
		* below.  For example, the message may have failed some integrity check
		* due to corruption in flight.
		*/
		if ((item.uDataType == QCBOR_TYPE_UINT64) ||
		    (item.uDataType == QCBOR_TYPE_INT64)) {
			ERROR("TME: decode_message TME returned integer error: 0x%llx\n",
				(unsigned long long)item.val.uint64);
			break;
		}

		/* A valid, handled message has a bstr-formatted response. */
		if (item.uDataType != QCBOR_TYPE_BYTE_STRING) {
			ERROR("TME: decode_message unexpected CBOR data type: %u\n", item.uDataType);
			break;
		}

		decoded_len = memscpy(message_buf->ptr,
					message_buf->len,
					item.val.string.ptr,
					item.val.string.len);

		if (decoded_len != item.val.string.len) {
			message_buf->len = 0;
			ERROR("TME: decode_message response buffer copy overflow for "
			      "tag=0x%x\n", tag);
			ret = -ENOMEM;
			break;
		}

		message_buf->len = decoded_len;
		ret = 0;
	} while (0);

	return ret;
}

static int transceive_message_internal(uint32_t tag,
				     void     *client_ptr,
				     void     *req_buf,
				     size_t    req_buf_len,
				     void     *resp_buf,
				     size_t    resp_buf_capacity,
				     size_t   *resp_buf_len,
				     uint32_t  timeout_msec)
{
	int ret = 0;

	tmecom_msg_req_t *tmecm_req         = NULL;
	size_t          tmecm_req_len      = 0;
	UsefulBufC      request_buf        = {req_buf, req_buf_len};
	UsefulBuf       encoded_req_buf    = {0};
	size_t          encoded_req_len    = 0;

	tmecom_msg_rsp_t *tmecm_rsp         = NULL;
	size_t          tmecm_rsp_len      = 0;
	UsefulBuf       response_buf       = {resp_buf, resp_buf_capacity};
	UsefulBuf       encoded_rsp_buf    = {0};
	size_t          encoded_rsp_len    = 0;

	do {
		if ((client_ptr == NULL) || (req_buf == NULL) ||
			((resp_buf == NULL) && (resp_buf_capacity != 0U)) ||
			(resp_buf_len == NULL)) {
			ret = -EINVAL;
			ERROR("TME: transceive_message NULL client for tag=0x%x\n", tag);
			break;
		}

		/* Calculate the CBOR-encoded request size and allocate a framed buffer. */
		encoded_req_len = get_encoded_number_size(tag) +
				get_encoded_number_size(req_buf_len) +
				req_buf_len;

		if (encoded_req_len > MAX_CBOR_REQ_LENGTH) {
			ERROR("TME: request encoded size %zu exceeds "
			      "MAX_CBOR_REQ_LENGTH for tag=0x%x\n",
				encoded_req_len, tag);
			ret = -EMSGSIZE;
			break;
		}

		tmecm_req_len = sizeof(tmecom_msg_hdr_t) + encoded_req_len;
		tmecm_req     = tme_msg_alloc_req(tmecm_req_len);

		if (tmecm_req == NULL) {
			ERROR("TME: request buffer allocation failed (size=%zu) for tag=0x%x\n",
				encoded_req_len, tag);
			ret = -ENOMEM;
			break;
		}

		encoded_req_buf.ptr = tmecm_req->enc_req_buf;
		encoded_req_buf.len = encoded_req_len;

		/* Calculate the CBOR-encoded response size and allocate a framed buffer. */
		encoded_rsp_len = get_encoded_number_size(tag) +
				get_encoded_number_size(resp_buf_capacity) +
				resp_buf_capacity;

		if (encoded_rsp_len > MAX_CBOR_RSP_LENGTH) {
			ERROR("TME: response encoded size %zu exceeds "
			      "MAX_CBOR_RSP_LENGTH for tag=0x%x\n",
				encoded_rsp_len, tag);
			ret = -EMSGSIZE;
			break;
		}

		tmecm_rsp_len = sizeof(tmecom_msg_hdr_t) + encoded_rsp_len;
		tmecm_rsp     = tme_msg_alloc_rsp(tmecm_rsp_len);

		if (tmecm_rsp == NULL) {
			ERROR("TME: response buffer allocation failed (size=%zu) for tag=0x%x\n",
				encoded_rsp_len, tag);
			ret = -ENOMEM;
			break;
		}

		encoded_rsp_buf.ptr = tmecm_rsp->enc_rsp_buf;
		encoded_rsp_buf.len = encoded_rsp_len;

		/* CBOR-encode the raw request struct. */
		ret = encode_message(tag, request_buf, &encoded_req_buf);
		if (ret != 0) {
			ERROR("TME: encode_message failed in transceive_message: tag=0x%x ret=%d\n",
			      tag, ret);
			break;
		}

		/* Send the framed request synchronously to TME. */
		ret = tmecom_client_send_message_sync(client_ptr,
						tmecm_req,
						tmecm_req_len,
						tmecm_rsp,
						&tmecm_rsp_len,
						timeout_msec);
		if (ret != 0) {
			/* A transport failure communicating with TME is non-recoverable. */
			bl31qtilib_cb_error_fatal(TMECOM_PROTOCOL_FAILURE);
			break;
		}

		/* Strip the tmecom_msg_hdr_t from the returned length before CBOR decode. */
		if (sizeof(tmecom_msg_hdr_t) > tmecm_rsp_len) {
			ERROR("TME: response encoded size %zu is lower than the size "
			      "of a TME message header, %zu\n",
				tmecm_rsp_len, sizeof(tmecom_msg_hdr_t));
			ret = -EMSGSIZE;
			break;
		}
		if (tmecm_rsp_len - sizeof(tmecom_msg_hdr_t) > encoded_rsp_buf.len) {
			ERROR("TME: response encoded size %zu exceeds response buffer "
			      "capacity %zu for tag=0x%x\n",
				tmecm_rsp_len - sizeof(tmecom_msg_hdr_t), encoded_rsp_buf.len, tag);
			ret = -EMSGSIZE;
			break;
		}
		encoded_rsp_buf.len = tmecm_rsp_len - sizeof(tmecom_msg_hdr_t);

		/* CBOR-decode the response. */
		ret = decode_message(tag, UsefulBuf_Const(encoded_rsp_buf), &response_buf);

		if (ret != 0) {
			ERROR("TME: decode_message failed in transceive_message: tag=0x%x ret=%d\n",
			      tag, ret);
			break;
		}

		*resp_buf_len = response_buf.len;
	} while (0);

	if (tmecm_req != NULL) {
		tme_msg_free_req(tmecm_req);
	}
	if (tmecm_rsp != NULL) {
		tme_msg_free_rsp(tmecm_rsp);
	}

	return ret;
}

int transceive_message(uint32_t tag,
		      void    *req_buf,
		      size_t   req_buf_len,
		      void    *resp_buf,
		      size_t   resp_buf_len,
		      size_t  *resp_len)
{
	tmecom_client_t *client = NULL;
	int           ret = -EIO;

	do {
		ret = tmecom_interface_init(&client);
		if (ret != 0) {
			break;
		}

		ret = transceive_message_internal(tag,
							client,
							req_buf,
							req_buf_len,
							resp_buf,
							resp_buf_len,
							resp_len,
							TMECOM_RESPONSE_TIMEOUT_MS);
	} while (0);

	if (ret != 0) {
		ERROR("TME: transceive_message failed: tag=0x%x ret=%d\n", tag, ret);
	}

	return ret;
}

int update_extended_error_info(tme_extended_error_info_t *error_info,
				     tme_extended_error_info_t  result)
{
	int ret = -EINVAL;
	bool is_failure = true;

	do {
		if (error_info == NULL) {
			break;
		}

		error_info->tme_error_status = result.tme_error_status;
		error_info->seq_error_status = result.seq_error_status;
		error_info->seq_kp_error_status0 = result.seq_kp_error_status0;
		error_info->seq_kp_error_status1 = result.seq_kp_error_status1;
		error_info->seq_rsp_status = result.seq_rsp_status;

		is_failure = (error_info->tme_error_status != 0U) ||
			(error_info->seq_error_status != 0U) ||
			(error_info->seq_kp_error_status0 != 0U) ||
			(error_info->seq_kp_error_status1 != 0U);
		ret = is_failure ? -EIO : 0;
	} while (0);

	return ret;
}
