/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stdint.h>
#include <stddef.h>
#include <stringl/stringl.h>

#include <common/debug.h>

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
static tmecomMsgReq_t gTmecomMsgReq = { 0 };
static tmecomMsgRsp_t gTmecomMsgRsp = { 0 };

static void *tmeMsgAllocReq(size_t size) {
  if (size <= sizeof(gTmecomMsgReq)) return (void *)&gTmecomMsgReq;
  return NULL;
}
static void tmeMsgFreeReq(void *pMem) { (void)pMem; }
static void *tmeMsgAllocRsp(size_t size) {
  if (size <= sizeof(gTmecomMsgRsp)) return (void *)&gTmecomMsgRsp;
  return NULL;
}
static void tmeMsgFreeRsp(void *pMem) { (void)pMem; }

size_t GetEncodedNumberSize(uint32_t number)
{
  size_t encodedSize = sizeof(uint8_t);

  /* Unreachable for uint32_t argument. Mirrors original implementation. */
  if (number > 0xffffffff) {
    encodedSize += sizeof(uint64_t);
  } else if (number > 0xffff) {
    encodedSize += sizeof(uint32_t);
  } else if (number > 0xff) {
    encodedSize += sizeof(uint16_t);
  } else if (number >= 24) {
    encodedSize += sizeof(uint8_t);
  }
  return encodedSize;
}

size_t GetMaxRequestPayload(void)
{
  return MAX_CBOR_REQ_LENGTH -
         GetEncodedNumberSize(TME_MSG_CBOR_TAG_MAX) -
         GetEncodedNumberSize(MAX_CBOR_REQ_LENGTH);
}

size_t GetMaxResponsePayload(void)
{
  return MAX_CBOR_RSP_LENGTH -
         GetEncodedNumberSize(TME_MSG_CBOR_TAG_MAX) -
         GetEncodedNumberSize(MAX_CBOR_RSP_LENGTH);
}

int EncodeMessage(uint32_t tag, const UsefulBufC messageBuf, UsefulBuf *encodedBuf)
{
  int                ret          = E_FAILURE;
  size_t             encodedLen   = 0;
  QCBOREncodeContext encodeContext = {0};

  INFO("%s: ENTRY tag:0x%08X messageLen:%zu bufCapacity:%zu\n", __func__,
       tag, messageBuf.len, encodedBuf->len);

  QCBOREncode_Init(&encodeContext, *encodedBuf);

  QCBOREncode_AddTag(&encodeContext, tag);
  QCBOREncode_AddBytes(&encodeContext, messageBuf);

  ret = QCBOREncode_FinishGetSize(&encodeContext, &encodedLen);

  if (ret != QCBOR_SUCCESS)
  {
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_ENCODE_MESSAGE_QCBOR_ERROR, tag);
    INFO("%s: EXIT ret=%d (QCBOR encode error)\n", __func__, ret);
    return ret;
  }

  if (encodedLen <= encodedBuf->len)
  {
    encodedBuf->len = encodedLen;
  }
  else
  {
    encodedBuf->len = 0;
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_ENCODE_MESSAGE_LENGTH_ERROR, tag, encodedLen, encodedBuf->len);
    INFO("%s: EXIT ret=%d (encoded length %zu exceeds capacity)\n",
         __func__, E_NO_MEMORY, encodedLen);
    return E_NO_MEMORY;
  }

  INFO("%s: EXIT ret=%d encodedLen:%zu\n", __func__, E_SUCCESS,
       encodedBuf->len);
  return E_SUCCESS;
}

int DecodeMessage(uint32_t tag, const UsefulBufC encodedBuf, UsefulBuf *messageBuf)
{
  int                ret           = E_FAILURE;
  size_t             decodedLen    = 0;
  QCBORDecodeContext decodeContext = {0};
  QCBORItem          item          = {0};
  size_t             dump_len;
  size_t             i;

  INFO("%s: ENTRY tag:0x%08X encodedLen:%zu bufCapacity:%zu\n", __func__,
       tag, encodedBuf.len, messageBuf->len);

  /*
   * Dump the raw bytes unconditionally, before any parsing is attempted, so
   * a malformed/unexpected response is still visible even when every
   * decode step below fails.
   */
  dump_len = (encodedBuf.len < 64U) ? encodedBuf.len : 64U;
  printf("INFO:    %s: raw encodedBuf (%zu bytes):", __func__, encodedBuf.len);
  for (i = 0U; i < dump_len; i++) {
    printf(" %02X", ((const uint8_t *)encodedBuf.ptr)[i]);
  }
  if (encodedBuf.len > dump_len) {
    printf(" ... (%zu more bytes)", encodedBuf.len - dump_len);
  }
  printf("\n");

  QCBORDecode_Init(&decodeContext, encodedBuf, QCBOR_DECODE_MODE_NORMAL);

  ret = QCBORDecode_GetNext(&decodeContext, &item);
  if (ret != QCBOR_SUCCESS)
  {
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_QCBOR_ERROR, tag);
    INFO("%s: EXIT ret=%d (QCBOR decode error)\n", __func__, ret);
    return ret;
  }

  /* v1.2 QCBOR no longer exposes a single item.uTag field; fetch the
   * outermost tag (index 0) of the decoded item via the accessor. */
  uint64_t itemTag = QCBORDecode_GetNthTag(&decodeContext, &item, 0);

  /* Either tag matches the message, or TME responded with an error tag. */
  if ((itemTag != tag) && (itemTag != TME_MSG_CBOR_TAG_ERROR))
  {
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_TAG_ERROR, itemTag, tag);
    INFO("%s: EXIT ret=%d (tag mismatch got:0x%llx want:0x%08X)\n",
         __func__, E_FAILURE, (unsigned long long)itemTag, tag);
    return E_FAILURE;
  }

  /*
   * Generic errors are sent as integers.  A valid bstr message is handled
   * below.  For example, the message may have failed some integrity check
   * due to corruption in flight.
   */
  if ((item.uDataType == QCBOR_TYPE_UINT64) || (item.uDataType == QCBOR_TYPE_INT64))
  {
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_DATA_ERROR, item.val.uint64);
    INFO("%s: EXIT ret=%d (generic error response val:0x%llx)\n", __func__,
         E_FAILURE, (unsigned long long)item.val.uint64);
    return E_FAILURE;
  }

  /* A valid, handled message has a bstr-formatted response. */
  if (item.uDataType != QCBOR_TYPE_BYTE_STRING)
  {
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_TYPE_ERROR, item.uDataType);
    INFO("%s: EXIT ret=%d (unexpected item type:%d)\n", __func__,
         E_FAILURE, item.uDataType);
    return E_FAILURE;
  }

  decodedLen = memscpy(messageBuf->ptr,
                       messageBuf->len,
                       item.val.string.ptr,
                       item.val.string.len);

  if (decodedLen <= messageBuf->len)
  {
    messageBuf->len = decodedLen;
  }
  else
  {
    messageBuf->len = 0;
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_BUF_COPY_ERROR, tag);
    INFO("%s: EXIT ret=%d (decoded length %zu exceeds capacity)\n",
         __func__, E_NO_MEMORY, decodedLen);
    return E_NO_MEMORY;
  }

  INFO("%s: EXIT ret=%d decodedLen:%zu\n", __func__, E_SUCCESS,
       messageBuf->len);
  return E_SUCCESS;
}

static int TransceiveMessageInternal(uint32_t  tag,
                                     void     *clientPtr,
                                     void     *reqBuf,
                                     size_t    reqBufLen,
                                     void     *respBuf,
                                     size_t    respBufCapacity,
                                     size_t   *respBufLen,
                                     uint32_t  timeoutMSec)
{
  int ret = 0;

  tmecomMsgReq_t *tmecomMsgReq      = NULL;
  size_t          tmecomMsgReqLen    = 0;
  UsefulBufC      requestBuf         = {reqBuf, reqBufLen};
  UsefulBuf       encodedRequestBuf  = {0};
  size_t          encodedRequestLen  = 0;

  tmecomMsgRsp_t *tmecomMsgRsp       = NULL;
  size_t          tmecomMsgRspLen    = 0;
  UsefulBuf       responseBuf        = {respBuf, respBufCapacity};
  UsefulBuf       encodedResponseBuf = {0};
  size_t          encodedResponseLen = 0;

  INFO("%s: ENTRY tag:0x%08X reqBufLen:%zu respBufCapacity:%zu timeoutMSec:%u\n",
       __func__, tag, reqBufLen, respBufCapacity, timeoutMSec);

  if (clientPtr == NULL)
  {
    ret = E_INVALID_ARG;
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_CLIENT_INVALID, tag);
    goto exit;
  }

  /* Calculate the CBOR-encoded request size and allocate a framed buffer. */
  encodedRequestLen = GetEncodedNumberSize(tag) +
                      GetEncodedNumberSize(reqBufLen) +
                      reqBufLen;

  if (encodedRequestLen > MAX_CBOR_REQ_LENGTH)
  {
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_ALLOCATE_FAILED, tag, encodedRequestLen);
    ret = E_DATA_TOO_LARGE;
    goto exit;
  }

  tmecomMsgReqLen = sizeof(tmecomMsgHdr) + encodedRequestLen;
  tmecomMsgReq    = tmeMsgAllocReq(tmecomMsgReqLen);

  if (tmecomMsgReq == NULL)
  {
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_ALLOCATE_FAILED, tag, encodedRequestLen);
    ret = E_NO_MEMORY;
    goto exit;
  }

  encodedRequestBuf.ptr = tmecomMsgReq->encReqBuf;
  encodedRequestBuf.len = encodedRequestLen;

  /* Calculate the CBOR-encoded response size and allocate a framed buffer. */
  encodedResponseLen = GetEncodedNumberSize(tag) +
                       GetEncodedNumberSize(respBufCapacity) +
                       respBufCapacity;

  if (encodedResponseLen > MAX_CBOR_RSP_LENGTH)
  {
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_ALLOCATE_FAILED, tag, encodedResponseLen);
    ret = E_DATA_TOO_LARGE;
    goto exit;
  }

  tmecomMsgRspLen = sizeof(tmecomMsgHdr) + encodedResponseLen;
  tmecomMsgRsp    = tmeMsgAllocRsp(tmecomMsgRspLen);

  if (tmecomMsgRsp == NULL)
  {
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_ALLOCATE_FAILED, tag, encodedResponseLen);
    ret = E_NO_MEMORY;
    goto exit;
  }

  encodedResponseBuf.ptr = tmecomMsgRsp->encRspBuf;
  encodedResponseBuf.len = encodedResponseLen;

  /* CBOR-encode the raw request struct. */
  ret = EncodeMessage(tag, requestBuf, &encodedRequestBuf);
  if (ret != QCBOR_SUCCESS)
  {
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_ENCODE_MESSAGE_ERROR, tag, ret);
    goto exit;
  }

  /* Send the framed request synchronously to TME. */
  ret = tmecomClientSendMessageSync(clientPtr,
                                    tmecomMsgReq,
                                    tmecomMsgReqLen,
                                    tmecomMsgRsp,
                                    &tmecomMsgRspLen,
                                    timeoutMSec);
  if (ret != 0)
  {
    /* A transport failure communicating with TME is non-recoverable. */
    bl31qtilib_cb_error_fatal(TME_ERR_FATAL_TMECOM_PROTOCOL_FAILURE);
  }

  /* Strip the tmecomMsgHdr from the returned length before CBOR decode. */
  encodedResponseBuf.len = tmecomMsgRspLen - sizeof(tmecomMsgHdr);

  /* CBOR-decode the response. */
  ret = DecodeMessage(tag, UsefulBuf_Const(encodedResponseBuf), &responseBuf);

  if (ret != QCBOR_SUCCESS)
  {
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_DECODE_MESSAGE_ERROR, tag, ret);
    goto exit;
  }

  *respBufLen = responseBuf.len;

exit:
  INFO("%s: EXIT ret=%d\n", __func__, ret);
  if (tmecomMsgReq != NULL) tmeMsgFreeReq(tmecomMsgReq);
  if (tmecomMsgRsp != NULL) tmeMsgFreeRsp(tmecomMsgRsp);

  return ret;
}

int TransceiveMessage(uint32_t tag,
                      void    *reqBuf,
                      size_t   reqBufLen,
                      void    *respBuf,
                      size_t   respBufLen,
                      size_t  *respLen)
{
  tmecomClient *client = NULL;
  int           ret;

  INFO("%s: ENTRY tag:0x%08X reqBufLen:%zu respBufLen:%zu\n", __func__, tag,
       reqBufLen, respBufLen);

  ret = tmecomInterfaceInit(&client);

  if (ret == E_SUCCESS)
  {
    ret = TransceiveMessageInternal(tag,
                                    client,
                                    reqBuf,
                                    reqBufLen,
                                    respBuf,
                                    respBufLen,
                                    respLen,
                                    TMECOM_RESPONSE_TIMEOUT_MS);
  }

  if (E_SUCCESS != ret)
  {
    TFA_LOG_ERR(TZBSP_TME_MESSAGE_TRANSCEIVE_FAIL, tag);
  }

  INFO("%s: EXIT ret=%d respLen:%zu\n", __func__, ret,
       (respLen != NULL) ? *respLen : 0U);
  return ret;
}

uint32_t UpdatedExtendedErrorInfo(TmeExtendedErrorInfo *errorInfo,
                                  TmeExtendedErrorInfo  result)
{
  uint32_t ret = E_INVALID_ARG;

  if (errorInfo)
  {
    errorInfo->tmeErrorStatus    = result.tmeErrorStatus;
    errorInfo->seqErrorStatus    = result.seqErrorStatus;
    errorInfo->seqKPErrorStatus0 = result.seqKPErrorStatus0;
    errorInfo->seqKPErrorStatus1 = result.seqKPErrorStatus1;
    errorInfo->seqRspStatus      = result.seqRspStatus;

    uint32_t isFailure = errorInfo->tmeErrorStatus  ||
                         errorInfo->seqErrorStatus  ||
                         errorInfo->seqKPErrorStatus0 ||
                         errorInfo->seqKPErrorStatus1;

    ret = isFailure ? E_FAILURE : E_SUCCESS;
  }

  return ret;
}
