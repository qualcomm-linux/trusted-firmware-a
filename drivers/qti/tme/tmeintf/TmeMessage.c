/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stdint.h>
#include <stddef.h>
#include <stringl/stringl.h>

#include "IxErrno.h"
#include "tzbsp_err_fatal.h"
#include "bl31qtilib_cb_interface.h"
#include "tzbsp_log.h"
#include "tmecomTFA.h"
#include "tmecom_interfaces.h"
#include "TmeMessage.h"
#include "TmeMessagesTags.h"
#include "qcbor.h"

/*
 * In the TFA build, request/response message buffers are statically reserved
 * within the driver (mirroring tmecom.c's static allocation).
 */
static tmecomMsgReq_t g_tmecom_msg_req = { 0 };
static tmecomMsgRsp_t g_tmecom_msg_rsp = { 0 };

static void *tme_msg_alloc_req(size_t size)
{
	if (size <= sizeof(g_tmecom_msg_req)) {
		return (void *)&g_tmecom_msg_req;
	}

	return NULL;
}

static void tme_msg_free_req(void *p_mem)
{
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
	(void)p_mem;
}

size_t get_encoded_number_size(uint32_t number)
{
	size_t encoded_size = sizeof(uint8_t);

	/* Unreachable for uint32_t argument. Mirrors original implementation. */
	if (number > 0xffffffff) {
		encoded_size += sizeof(uint64_t);
	} else if (number > 0xffff) {
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

int encode_message(uint32_t tag, const UsefulBufC message_buf, UsefulBuf *encoded_buf)
{
	int                ret          = E_FAILURE;
	size_t             encoded_len   = 0;
	QCBOREncodeContext encode_context = {0};

	QCBOREncode_Init(&encode_context, *encoded_buf);

	QCBOREncode_AddTag(&encode_context, tag);
	QCBOREncode_AddBytes(&encode_context, message_buf);

	ret = QCBOREncode_FinishGetSize(&encode_context, &encoded_len);

	if (ret != QCBOR_SUCCESS) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_ENCODE_MESSAGE_QCBOR_ERROR, tag);
		return ret;
	}

	if (encoded_len <= encoded_buf->len) {
		encoded_buf->len = encoded_len;
	}
	else {
		encoded_buf->len = 0;
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_ENCODE_MESSAGE_LENGTH_ERROR, tag, encoded_len, encoded_buf->len);
		return E_NO_MEMORY;
	}

	return E_SUCCESS;
}

int decode_message(uint32_t tag, const UsefulBufC encoded_buf, UsefulBuf *message_buf)
{
	int                ret           = E_FAILURE;
	size_t             decoded_len    = 0;
	QCBORDecodeContext decode_context = {0};
	QCBORItem          item          = {0};

	QCBORDecode_Init(&decode_context, encoded_buf, QCBOR_DECODE_MODE_NORMAL);

	ret = QCBORDecode_GetNext(&decode_context, &item);
	if (ret != QCBOR_SUCCESS) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_QCBOR_ERROR, tag);
		return ret;
	}

	/* v1.2 QCBOR no longer exposes a single item.uTag field; fetch the
	 * outermost tag (index 0) of the decoded item via the accessor. */
	uint64_t item_tag = QCBORDecode_GetNthTag(&decode_context, &item, 0);

	/* Either tag matches the message, or TME responded with an error tag. */
	if ((item_tag != tag) && (item_tag != TME_MSG_CBOR_TAG_ERROR)) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_TAG_ERROR, item_tag, tag);
		return E_FAILURE;
	}

	/*
	 * Generic errors are sent as integers.  A valid bstr message is handled
	 * below.  For example, the message may have failed some integrity check
	 * due to corruption in flight.
	 */
	if ((item.uDataType == QCBOR_TYPE_UINT64) || (item.uDataType == QCBOR_TYPE_INT64)) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_DATA_ERROR, item.val.uint64);
		return E_FAILURE;
	}

	/* A valid, handled message has a bstr-formatted response. */
	if (item.uDataType != QCBOR_TYPE_BYTE_STRING) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_TYPE_ERROR, item.uDataType);
		return E_FAILURE;
	}

	/*
	 * Check the SOURCE length against the caller's capacity BEFORE copying.
	 *
	 * memscpy() copies min(dst_size, src_size) and returns that count, so
	 * testing its return value against message_buf->len can never fail - the
	 * old `decoded_len <= message_buf->len` check was always true and silently
	 * truncated an over-long response while still reporting E_SUCCESS.  Per
	 * memscpy()'s contract (see drivers/qti/tme/tme_tfa_glue.c), truncation is
	 * detected by comparing against the SOURCE size.
	 */
	if (item.val.string.len > message_buf->len) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_BUF_COPY_ERROR, tag);
		message_buf->len = 0;
		return E_NO_MEMORY;
	}

	decoded_len = memscpy(message_buf->ptr,
		 message_buf->len,
		 item.val.string.ptr,
		 item.val.string.len);

	message_buf->len = decoded_len;

	return E_SUCCESS;
}

static int transceive_message_internal(uint32_t  tag,
		 void     *client_ptr,
		 void     *req_buf,
		 size_t    req_buf_len,
		 void     *resp_buf,
		 size_t    resp_buf_capacity,
		 size_t   *resp_buf_len,
		 uint32_t  timeout_msec)
{
	int ret = 0;

	tmecomMsgReq_t *tmecom_msg_req      = NULL;
	size_t          tmecom_msg_req_len    = 0;
	UsefulBufC      request_buf         = {req_buf, req_buf_len};
	UsefulBuf       encoded_request_buf  = {0};
	size_t          encoded_request_len  = 0;

	tmecomMsgRsp_t *tmecom_msg_rsp       = NULL;
	size_t          tmecom_msg_rsp_len    = 0;
	UsefulBuf       response_buf        = {resp_buf, resp_buf_capacity};
	UsefulBuf       encoded_response_buf = {0};
	size_t          encoded_response_len = 0;

	if (client_ptr == NULL) {
		ret = E_INVALID_ARG;
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_CLIENT_INVALID, tag);
		goto exit;
	}

	/* Calculate the CBOR-encoded request size and allocate a framed buffer. */
	encoded_request_len = get_encoded_number_size(tag) +
		get_encoded_number_size(req_buf_len) +
		req_buf_len;

	if (encoded_request_len > MAX_CBOR_REQ_LENGTH) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_ALLOCATE_FAILED, tag, encoded_request_len);
		ret = E_DATA_TOO_LARGE;
		goto exit;
	}

	tmecom_msg_req_len = sizeof(tmecomMsgHdr) + encoded_request_len;
	tmecom_msg_req    = tme_msg_alloc_req(tmecom_msg_req_len);

	if (tmecom_msg_req == NULL) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_ALLOCATE_FAILED, tag, encoded_request_len);
		ret = E_NO_MEMORY;
		goto exit;
	}

	encoded_request_buf.ptr = tmecom_msg_req->enc_req_buf;
	encoded_request_buf.len = encoded_request_len;

	/* Calculate the CBOR-encoded response size and allocate a framed buffer. */
	encoded_response_len = get_encoded_number_size(tag) +
		 get_encoded_number_size(resp_buf_capacity) +
		 resp_buf_capacity;

	if (encoded_response_len > MAX_CBOR_RSP_LENGTH) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_ALLOCATE_FAILED, tag, encoded_response_len);
		ret = E_DATA_TOO_LARGE;
		goto exit;
	}

	tmecom_msg_rsp_len = sizeof(tmecomMsgHdr) + encoded_response_len;
	tmecom_msg_rsp    = tme_msg_alloc_rsp(tmecom_msg_rsp_len);

	if (tmecom_msg_rsp == NULL) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_ALLOCATE_FAILED, tag, encoded_response_len);
		ret = E_NO_MEMORY;
		goto exit;
	}

	encoded_response_buf.ptr = tmecom_msg_rsp->enc_rsp_buf;
	encoded_response_buf.len = encoded_response_len;

	/* CBOR-encode the raw request struct. */
	ret = encode_message(tag, request_buf, &encoded_request_buf);
	if (ret != QCBOR_SUCCESS) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_ENCODE_MESSAGE_ERROR, tag, ret);
		goto exit;
	}

	/* Send the framed request synchronously to TME. */
	ret = tmecom_client_send_message_sync(client_ptr,
		tmecom_msg_req,
		tmecom_msg_req_len,
		tmecom_msg_rsp,
		&tmecom_msg_rsp_len,
		timeout_msec);
	if (ret != 0) {
		/* A transport failure communicating with TME is non-recoverable. */
		bl31qtilib_cb_error_fatal(TME_ERR_FATAL_TMECOM_PROTOCOL_FAILURE);
		/*
		 * error_fatal() is not expected to return, but do not rely on that:
		 * falling through with a failed transport would underflow the
		 * tmecom_msg_rsp_len - sizeof(tmecomMsgHdr) subtraction below (rsp len is
		 * 0 or short on failure) and hand decode_message a ~2^64-byte buffer.
		 * Bailing here also preserves the transport's error code, which
		 * decode_message would otherwise overwrite.
		 */
		goto exit;
	}

	/*
	 * Strip the tmecomMsgHdr from the returned length before CBOR decode.
	 * tmecom_msg_rsp_len is an in/out parameter, so re-validate it rather than
	 * trusting the transport not to have enlarged it past the buffer.
	 */
	if ((tmecom_msg_rsp_len < sizeof(tmecomMsgHdr)) ||
			(tmecom_msg_rsp_len > (sizeof(tmecomMsgHdr) + encoded_response_len))) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_ERROR, tag, tmecom_msg_rsp_len);
		ret = E_FAILURE;
		goto exit;
	}

	encoded_response_buf.len = tmecom_msg_rsp_len - sizeof(tmecomMsgHdr);

	/* CBOR-decode the response. */
	ret = decode_message(tag, UsefulBuf_Const(encoded_response_buf), &response_buf);

	if (ret != QCBOR_SUCCESS) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_ERROR, tag, ret);
		goto exit;
	}

	*resp_buf_len = response_buf.len;

exit:
	if (tmecom_msg_req != NULL) tme_msg_free_req(tmecom_msg_req);
	if (tmecom_msg_rsp != NULL) tme_msg_free_rsp(tmecom_msg_rsp);

	return ret;
}

int transceive_message(uint32_t tag,
		void    *req_buf,
		size_t   req_buf_len,
		void    *resp_buf,
		size_t   resp_buf_len,
		size_t  *resp_len)
{
	tmecomClient *client = NULL;
	int           ret;

	ret = tmecom_interface_init(&client);

	if (ret == E_SUCCESS) {
		ret = transceive_message_internal(tag,
		client,
		req_buf,
		req_buf_len,
		resp_buf,
		resp_buf_len,
		resp_len,
		TMECOM_RESPONSE_TIMEOUT_MS);
	}

	if (E_SUCCESS != ret) {
		TFA_LOG_ERR(TZBSP_TME_MESSAGE_TRANSCEIVE_FAIL, tag);
	}

	return ret;
}

uint32_t update_extended_error_info(TmeExtendedErrorInfo *error_info,
		TmeExtendedErrorInfo  result)
{
	uint32_t ret = E_INVALID_ARG;

	if (error_info) {
		error_info->tme_error_status    = result.tme_error_status;
		error_info->seq_error_status    = result.seq_error_status;
		error_info->seq_kp_error_status0 = result.seq_kp_error_status0;
		error_info->seq_kp_error_status1 = result.seq_kp_error_status1;
		error_info->seq_rsp_status      = result.seq_rsp_status;

		uint32_t is_failure = error_info->tme_error_status  ||
		 error_info->seq_error_status  ||
		 error_info->seq_kp_error_status0 ||
		 error_info->seq_kp_error_status1;

		ret = is_failure ? E_FAILURE : E_SUCCESS;
	}

	return ret;
}
