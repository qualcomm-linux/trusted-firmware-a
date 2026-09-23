/*===========================================================================
	Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
	All rights reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef TME_MESSAGE_H_INCLUDED
#define TME_MESSAGE_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#include "IxErrno.h"
#include "tmecom.h"
#include "TmeInterfacesDefs.h"
#include "TmeMessagesTags.h"
#include "UsefulBuf.h"

/* True if [addr, addr+size) is a valid (non-wrapping) address range.
 * NOTE: The original implementation uses region_is_contained_in() to
 * further constrain the range to the 32-bit DDR window [0x80000000, 0x100000000).
 * That API is not available in TF-A, so a portable non-wrapping check is used. */
#define TME_ADDR_OK(addr, size) \
	((size_t)(size) == 0U || \
	 ((uintptr_t)(addr) + (size_t)(size)) > (uintptr_t)(addr))

/* Max CBOR payload that fits inside the tmecom request / response mailbox. */
#define MAX_CBOR_REQ_LENGTH  (TMECOM_MAX_REQUEST_SIZE  - sizeof(tmecomMsgHdr))
#define MAX_CBOR_RSP_LENGTH  (TMECOM_MAX_RESPONSE_SIZE - sizeof(tmecomMsgHdr))

/*
 * get_encoded_number_size() - Return the number of bytes CBOR needs to encode @number.
 */
size_t get_encoded_number_size(uint32_t number);

/*
 * get_max_request_payload() - Return the maximum raw payload that fits in a CBOR request to TME.
 *
 * Accounts for the CBOR tag and length overhead around the payload.
 */
size_t get_max_request_payload(void);

/*
 * get_max_response_payload() - Return the maximum raw payload that fits in a CBOR response from TME.
 *
 * Accounts for the CBOR tag and length overhead around the payload.
 */
size_t get_max_response_payload(void);

/*
 * encode_message() - CBOR-encode @message_buf into @encoded_buf, tagged with @tag.
 *
 * @param [in]     tag         CBOR tag identifying the TME operation.
 * @param [in]     message_buf  Raw message bytes to encode.
 * @param [in/out] encoded_buf  Output buffer; on return, len holds encoded size.
 *
 * @return QCBOR_SUCCESS (0) on success, QCBOR error code otherwise.
 */
int encode_message(uint32_t tag, const UsefulBufC message_buf, UsefulBuf *encoded_buf);

/*
 * decode_message() - CBOR-decode @encoded_buf into @message_buf, verifying @tag.
 *
 * @param [in]     tag         Expected CBOR tag.
 * @param [in]     encoded_buf  CBOR-encoded input.
 * @param [in/out] message_buf  Output buffer; on return, len holds decoded size.
 *
 * @return QCBOR_SUCCESS (0) on success, QCBOR/TME error code otherwise.
 */
int decode_message(uint32_t tag, const UsefulBufC encoded_buf, UsefulBuf *message_buf);

/*
 * transceive_message() - CBOR-encode a request, send it to TME, then CBOR-decode
 * the response.  Uses a fixed timeout of TMECOM_RESPONSE_TIMEOUT_MS.
 *
 * A transport-layer failure (tmecom_client_send_message_sync returns non-zero) is
 * treated as fatal and calls tzbsp_err_fatal().
 *
 * @param [in]     tag          CBOR tag identifying the TME operation.
 * @param [in]     req_buf       Pointer to the raw (pre-CBOR) request struct.
 * @param [in]     req_buf_len    Size of the request struct in bytes.
 * @param [out]    resp_buf      Pointer to the buffer for the decoded response.
 * @param [in]     resp_buf_len   Size of resp_buf in bytes.
 * @param [out]    resp_len      Number of decoded response bytes written.
 *
 * @return E_SUCCESS on success, error code otherwise.
 */
int transceive_message(uint32_t tag,
		void    *req_buf,
		size_t   req_buf_len,
		void    *resp_buf,
		size_t   resp_buf_len,
		size_t  *resp_len);

/**
 * Copy extended error information from @c result into @c error_info and
 * return whether any error field is non-zero.
 *
 * @param [out] error_info  Destination for the error information (must not be NULL).
 * @param [in]  result     Source error information to copy.
 *
 * @return @c E_SUCCESS if all error fields are zero, @c E_FAILURE otherwise.
 *         Returns @c E_INVALID_ARG if @c error_info is NULL.
 */
uint32_t update_extended_error_info(TmeExtendedErrorInfo *error_info,
		TmeExtendedErrorInfo  result);

#endif /* TME_MESSAGE_H_INCLUDED */
