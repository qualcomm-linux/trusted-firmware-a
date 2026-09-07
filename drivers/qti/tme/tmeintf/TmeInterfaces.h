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

/*
 * TmeFuseRead() - read one QFPROM row via TME.
 *
 * @param [in]  addrType         Fuse address space (TME_QFPROM_ADDR_SPACE_*).
 * @param [in]  fuseAddr         SoC address of the QFPROM row to read.
 * @param [out] fuseData         Receives the row contents, low word first.
 *                               Must point to space for at least
 *                               TME_QFPROM_FUSE_DATA_WORDS uint32_t values -
 *                               a QFPROM row is always read two words at a
 *                               time, regardless of the width of interest.
 * @param [out] qfpromApiStatus  Status reported by TME's qfprom driver;
 *                               TME_QFPROM_NO_ERR on a clean read.  Written
 *                               only when the call returns E_SUCCESS.
 *
 * @return E_SUCCESS if the exchange completed and the response was the
 *         expected size, error code otherwise.
 *
 * NOTE: E_SUCCESS only means the request/response exchange itself succeeded.
 * The caller MUST also check @p qfpromApiStatus - TME reports a rejected or
 * failed fuse read there, not in the return value.
 */
int TmeFuseRead(TmeQfpromAddrSpace_t addrType,
                uint32_t             fuseAddr,
                uint32_t *const      fuseData,
                uint32_t *const      qfpromApiStatus);

/*
 * TmeFuseWriteMultiple() - blow up to TME_MAX_FUSE_WRITE_REQ QFPROM rows in a
 * single request via TME.
 *
 * ###########################################################################
 * # DESTRUCTIVE AND IRREVERSIBLE.  QFPROM fuses are one-time-programmable:   #
 * # any bit set in fuseArray[].data[] is blown permanently on real silicon    #
 * # and can never be cleared.  Blowing the wrong row can brick the part or    #
 * # lock it out of secure boot.                                              #
 * #                                                                         #
 * # An all-zero data[] blows nothing, which is what makes it safe to use for  #
 * # exercising this path without altering chip state.                        #
 * ###########################################################################
 *
 * @param [in]  fuseArray        Rows to write.  Not modified.
 * @param [in]  fuseArrayLen     Number of entries in fuseArray; must be in
 *                               1..TME_MAX_FUSE_WRITE_REQ.
 * @param [out] qfpromApiStatus  Status reported by TME for the write;
 *                               TME_QFPROM_NO_ERR on success.  Always written
 *                               once the arguments validate - set to
 *                               TME_QFPROM_STATUS_UNSET before the exchange.
 *
 * @return E_SUCCESS only if the exchange completed, the response was the
 *         expected size, AND TME reported TME_QFPROM_NO_ERR.
 *         E_BAD_ADDRESS / E_NO_DATA / E_DATA_TOO_LARGE on bad arguments,
 *         other error code on a failed exchange or a rejected write.
 *
 * NOTE: unlike TmeFuseRead(), a nonzero status is folded into the return value
 * here, so E_SUCCESS does mean the write itself was accepted.
 */
int TmeFuseWriteMultiple(TMEFuse_t      *fuseArray,
                         size_t          fuseArrayLen,
                         uint32_t *const qfpromApiStatus);

/*
 * TmeWriteConfigRegister() - write a QFPROM configuration register via TME.
 *
 * @param [in]  registerId  Register to write (QFPROM_BIST_CTRL,
 *                           QFPROM_WRITE_DISABLE_STICKY_BIT0/1).
 * @param [in]  value       Value to write into the register.
 *
 * @return E_SUCCESS if the exchange completed, the response was the expected
 *         size, AND TME reported a zero status.  Error code otherwise -
 *         including when TME rejects registerId itself.
 */
int TmeWriteConfigRegister(tmeConfigRegisterId_e registerId, uint32_t value);

#endif /* TME_INTERFACES_H_INCLUDED */
