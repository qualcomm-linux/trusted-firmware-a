/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef TME_INTERFACES_H_INCLUDED
#define TME_INTERFACES_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "IxErrno.h"
#include "TmeInterfacesDefs.h"

/*
 * TmeForwardRequest() - forward a pre-encoded (CBOR/QBOR) request to
 * TME and return the raw response.
 *
 * The request buffer is forwarded as-is; no serialisation is performed here.
 * A hard-coded communication timeout is applied internally — callers do not
 * supply or influence the timeout value.
 *
 * @param [in]  reqBuf       Pointer to the pre-encoded request buffer.
 * @param [in]  reqSize      Size of the request buffer in bytes.
 * @param [out] rspBuf       Pointer to the response buffer.
 * @param [in]  rspBufSize   Size of the response buffer in bytes.
 *
 * @return E_SUCCESS on success, error code otherwise.
 */
int TmeForwardRequest(void *reqBuf,
                      size_t      reqSize,
                      void       *rspBuf,
                      size_t     *rspBufSize);

/*
 * TmePassthroughCmd() - asynchronously forward a pre-encoded (CBOR/QBOR)
 * request from to TME.
 *
 * Returns as soon as the request has been handed to the transport; does not
 * wait for TME to finish processing it. The TME CPU processes one request
 * at a time and does not support queuing, so only one request -- submitted
 * via this function or TmeForwardRequest() -- may be outstanding at a time.
 *
 * Use TmePassthroughAwait() with the returned handle to poll for the
 * response.
 *
 * @param [in]  reqBuf    Pointer to the pre-encoded request buffer.
 * @param [in]  reqSize   Size of the request buffer in bytes.
 * @param [out] handle    Opaque handle identifying this request, to be
 *                        passed to TmePassthroughAwait().
 *
 * @return E_SUCCESS and *handle set if the request was submitted.
 *         E_AGAIN if TME is currently processing another request; the
 *         caller should retry later.
 *         Other error code on failure to submit the request.
 */
int TmePassthroughCmd(void *reqBuf, size_t reqSize, uint32_t *handle);

/*
 * TmePassthroughAwait() - poll, without blocking, for the response to a
 * request previously submitted via TmePassthroughCmd().
 *
 * @param [in]     handle      Handle returned by TmePassthroughCmd().
 * @param [out]    rspBuf      Pointer to the response buffer.
 * @param [in/out] rspBufSize  On input: capacity of rspBuf in bytes.
 *                             On output: actual response size in bytes.
 *
 * @return E_SUCCESS and response copied into rspBuf if TME has finished
 *         processing the request.
 *         E_IN_PROGRESS if TME is still processing the request; call again
 *         later.
 *         Other error code if handle is invalid/stale.
 */
int TmePassthroughAwait(uint32_t handle, void *rspBuf, size_t *rspBufSize);

/*
 * TmeSHADigest() - compute a SHA digest over a message via TME.
 *
 * @param [in]  inHashAlgorithm  Hash algorithm to use (TME_HA_SHA256/384/512).
 * @param [in]  inMsg            Pointer to the input message.
 * @param [in]  inMsgLen         Length of the input message in bytes.
 * @param [out] outDigest        Buffer to receive the computed digest.
 * @param [out] outDigestLen     On input: capacity of outDigest in bytes.
 *                               On output: actual digest length written.
 * @param [out] errorInfo        Extended error information from TME.
 *
 * @return 0 if successful, non-zero value otherwise.
 */
int TmeSHADigest(TMEHashAlgID_t        inHashAlgorithm,
                 const uint8_t        *inMsg,
                 size_t                inMsgLen,
                 uint8_t              *outDigest,
                 size_t               *outDigestLen,
                 TmeExtendedErrorInfo *errorInfo);

#endif /* TME_INTERFACES_H_INCLUDED */
