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
 * GetEncodedNumberSize() - Return the number of bytes CBOR needs to encode @number.
 */
size_t GetEncodedNumberSize(uint32_t number);

/*
 * GetMaxRequestPayload() - Return the maximum raw payload that fits in a CBOR request to TME.
 *
 * Accounts for the CBOR tag and length overhead around the payload.
 */
size_t GetMaxRequestPayload(void);

/*
 * GetMaxResponsePayload() - Return the maximum raw payload that fits in a CBOR response from TME.
 *
 * Accounts for the CBOR tag and length overhead around the payload.
 */
size_t GetMaxResponsePayload(void);

/*
 * EncodeMessage() - CBOR-encode @messageBuf into @encodedBuf, tagged with @tag.
 *
 * @param [in]     tag         CBOR tag identifying the TME operation.
 * @param [in]     messageBuf  Raw message bytes to encode.
 * @param [in/out] encodedBuf  Output buffer; on return, len holds encoded size.
 *
 * @return QCBOR_SUCCESS (0) on success, QCBOR error code otherwise.
 */
int EncodeMessage(uint32_t tag, const UsefulBufC messageBuf, UsefulBuf *encodedBuf);

/*
 * DecodeMessage() - CBOR-decode @encodedBuf into @messageBuf, verifying @tag.
 *
 * @param [in]     tag         Expected CBOR tag.
 * @param [in]     encodedBuf  CBOR-encoded input.
 * @param [in/out] messageBuf  Output buffer; on return, len holds decoded size.
 *
 * @return QCBOR_SUCCESS (0) on success, QCBOR/TME error code otherwise.
 */
int DecodeMessage(uint32_t tag, const UsefulBufC encodedBuf, UsefulBuf *messageBuf);

/*
 * TransceiveMessage() - CBOR-encode a request, send it to TME, then CBOR-decode
 * the response.  Uses a fixed timeout of TMECOM_RESPONSE_TIMEOUT_MS.
 *
 * A transport-layer failure (tmecomClientSendMessageSync returns non-zero) is
 * treated as fatal and calls tzbsp_err_fatal().
 *
 * @param [in]     tag          CBOR tag identifying the TME operation.
 * @param [in]     reqBuf       Pointer to the raw (pre-CBOR) request struct.
 * @param [in]     reqBufLen    Size of the request struct in bytes.
 * @param [out]    respBuf      Pointer to the buffer for the decoded response.
 * @param [in]     respBufLen   Size of respBuf in bytes.
 * @param [out]    respLen      Number of decoded response bytes written.
 *
 * @return E_SUCCESS on success, error code otherwise.
 */
int TransceiveMessage(uint32_t tag,
                      void    *reqBuf,
                      size_t   reqBufLen,
                      void    *respBuf,
                      size_t   respBufLen,
                      size_t  *respLen);

/**
 * Copy extended error information from @c result into @c errorInfo and
 * return whether any error field is non-zero.
 *
 * @param [out] errorInfo  Destination for the error information (must not be NULL).
 * @param [in]  result     Source error information to copy.
 *
 * @return @c E_SUCCESS if all error fields are zero, @c E_FAILURE otherwise.
 *         Returns @c E_INVALID_ARG if @c errorInfo is NULL.
 */
uint32_t UpdatedExtendedErrorInfo(TmeExtendedErrorInfo *errorInfo,
                                  TmeExtendedErrorInfo  result);

#endif /* TME_MESSAGE_H_INCLUDED */
