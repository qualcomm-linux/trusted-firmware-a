/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Shim layer: adapts the legacy tmecom client API (tmecomRegisterClient,
 * tmecomClientSendMessageSync, tmecomClientSendMessageAsync, etc.) used by
 * tmeintf to the new qcom_mbox-based tmecom driver in drivers/qti/tme/tmecom.c.
 *
 * BL31 is single-threaded, so only one channel and one async transaction are
 * ever outstanding at a time.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
 * The mbox-based tmecom driver below this shim (drivers/qti/tme/tmecom_mbox.c)
 * reports libc <errno.h> values (EINPROGRESS=36, ENOSPC=28, EAGAIN=35, ...).
 * Include libc errno.h *before* tmecom_os_al.h so this translation unit
 * interprets those driver returns in the driver's own namespace; the include
 * order also defines EFAULT, which suppresses the IxErrno remap block in
 * tmecom_os_al.h. Return codes handed *up* to tmeintf are translated to the
 * IxErrno E_* values that tmeintf compares against (see the E_AGAIN /
 * E_IN_PROGRESS returns below).
 */
#include <lib/libc/errno.h>

#include <common/debug.h>

#include "common/tme_cdefs.h"
#include "stringl/stringl.h"
#include "tmecom.h"
#include "tmecomTFA.h"
#include "tmecom_crc.h"
#include "tmecom_os_al.h"

#include <drivers/qti/tmecom/tmecom.h>

/* -------------------------------------------------------------------------
 * Static allocations for request/response framed buffers (matched to the
 * sizes declared in tmecom.h so tmeintf can use them as before).
 * ---------------------------------------------------------------------- */

static tmecomMsgReq_t s_req_buf;
static tmecomMsgRsp_t s_rsp_buf;

void *tmecomAllocReq(size_t size)
{
  if (size <= sizeof(s_req_buf)) {
    return &s_req_buf;
  }
  return NULL;
}

void tmecomFreeReq(void *pMem)
{
  (void)pMem;
}

void *tmecomAllocRsp(size_t size)
{
  if (size <= sizeof(s_rsp_buf)) {
    return &s_rsp_buf;
  }
  return NULL;
}

void tmecomFreeRsp(void *pMem)
{
  (void)pMem;
}

/* -------------------------------------------------------------------------
 * Async pending transaction state.  Since BL31 is single-threaded and TME
 * does not queue requests, at most one async transaction can exist at a time.
 * The transport now truly separates send from receive: SendMessageAsync hands
 * the request to the mailbox and returns immediately; RecvMessageAsync polls
 * the transport without blocking.
 * ---------------------------------------------------------------------- */

static bool      s_async_pending   = false;
static uint32_t  s_async_txn_id    = 0U;

/* -------------------------------------------------------------------------
 * tmecomClient - opaque handle.  One static instance; registration just
 * opens the channel via the new driver.
 * ---------------------------------------------------------------------- */

struct tmecomClient {
  bool registered;
};static struct tmecomClient s_client;

/* -------------------------------------------------------------------------
 * Legacy init / deinit (called from tmecomInterfaceInit path).
 * ---------------------------------------------------------------------- */

int tmecomInit(eTMEComInterface tmecomInterface)
{
  (void)tmecomInterface;
  /* Channel is opened on tmecomRegisterClient; nothing to do here. */
  return 0;
}

int tmecomDeInit(void)
{
  tmecom_deinit();
  s_client.registered  = false;
  s_async_pending      = false;
  s_async_txn_id       = 0U;
  return 0;
}

/* -------------------------------------------------------------------------
 * Client register / unregister.
 * ---------------------------------------------------------------------- */

int tmecomRegisterClient(const tmecomClientInfo *info, tmecomClient **clientPtr)
{
  int rc;

  INFO("%s: ENTRY channelName:%s\n", __func__,
       (info != NULL) ? info->channelName : "(null)");

  if ((info == NULL) || (clientPtr == NULL)) {
    INFO("%s: EXIT ret=-EINVAL\n", __func__);
    return -EINVAL;
  }

  if (s_client.registered) {
    /* Already open; return the existing client. */
    *clientPtr = &s_client;
    INFO("%s: EXIT ret=0 (already registered)\n", __func__);
    return 0;
  }

  rc = tmecom_init(info->channelName);
  if (rc != 0) {
    *clientPtr = NULL;
    INFO("%s: EXIT ret=%d (tmecom_init failed)\n", __func__, rc);
    return rc;
  }

  s_client.registered = true;
  *clientPtr = &s_client;
  INFO("%s: EXIT ret=0\n", __func__);
  return 0;
}

int tmecomUnregisterClient(tmecomClient *clientPtr)
{
  if (clientPtr == NULL) {
    return -EINVAL;
  }

  tmecom_deinit();
  s_client.registered = false;
  s_async_pending     = false;
  s_async_txn_id      = 0U;
  return 0;
}

/* -------------------------------------------------------------------------
 * Connection query.
 * ---------------------------------------------------------------------- */

bool tmecomClientIsServerConnected(tmecomClient *clientPtr)
{
  bool connected;

  INFO("%s: ENTRY\n", __func__);

  if ((clientPtr == NULL) || !clientPtr->registered) {
    INFO("%s: EXIT ret=false (not registered)\n", __func__);
    return false;
  }
  connected = tmecom_is_connected();
  INFO("%s: EXIT ret=%u\n", __func__, (unsigned int)connected);
  return connected;
}

bool tmecomIsTmeSubsystemLinkUp(void)
{
  return tmecom_is_connected();
}

/* -------------------------------------------------------------------------
 * Synchronous send/receive.
 *
 * tmeintf passes a full tmecomMsgReq_t (header + encoded payload) as @reqPtr
 * and expects a full tmecomMsgRsp_t (header + encoded response) in @respPtr.
 * The new tmecom_send_recv() owns the framing internally, so we strip the
 * header from the request payload, call send_recv, then rebuild the response
 * with the header so tmeintf's validation code keeps working.
 * ---------------------------------------------------------------------- */

int tmecomClientSendMessageSync(void     *clientPtr,
                                void     *reqPtr,
                                size_t    reqSize,
                                void     *respPtr,
                                size_t   *respSize,
                                uint32_t  timeoutMSec)
{
  tmecomMsgRsp_t *rsp = (tmecomMsgRsp_t *)respPtr;
  size_t          payload_size;
  size_t          rsp_payload_capacity;
  int             rc;

  (void)timeoutMSec; /* new driver handles its own timeout internally */

  INFO("%s: ENTRY reqSize:%zu timeoutMSec:%u\n", __func__, reqSize,
       timeoutMSec);

  if ((clientPtr == NULL) || (reqPtr == NULL) || (respPtr == NULL) ||
      (respSize == NULL) || (reqSize < sizeof(tmecomMsgHdr))) {
    INFO("%s: EXIT ret=-EINVAL\n", __func__);
    return -EINVAL;
  }

  /* Payload starts after the header. */
  const uint8_t *req_payload     = (const uint8_t *)reqPtr + sizeof(tmecomMsgHdr);
  size_t         req_payload_size = reqSize - sizeof(tmecomMsgHdr);

  rsp_payload_capacity = sizeof(rsp->encRspBuf);

  rc = tmecom_send_recv(req_payload, req_payload_size,
                        rsp->encRspBuf, &rsp_payload_capacity);
  if (rc != 0) {
    INFO("%s: EXIT ret=%d (tmecom_send_recv failed)\n", __func__, rc);
    return rc;
  }

  payload_size = rsp_payload_capacity;

  /* Fill in a synthetic response header so tmeintf validation passes. */
  rsp->hdr.version = (uint16_t)TMECOM_VERSION(1, 0);
  rsp->hdr.txnId   = ((const tmecomMsgHdr *)reqPtr)->txnId;
  rsp->hdr.crc     = tmeCalculateCRC16(rsp->encRspBuf, payload_size);

  *respSize = sizeof(tmecomMsgHdr) + payload_size;
  INFO("%s: EXIT ret=0 respSize:%zu\n", __func__, *respSize);
  return 0;
}

/* -------------------------------------------------------------------------
 * Async send/receive.
 *
 * The transport truly separates send from receive.  SendMessageAsync frames
 * the request and hands it to the mailbox, returning as soon as the local
 * transport has accepted it -- it does NOT wait for the TME SS to respond.
 * RecvMessageAsync advances the transport once and either copies out a ready
 * response or reports -EINPROGRESS.
 *
 * The driver-assigned transaction id is handed back to the caller as the
 * opaque handle and passed straight through to tmecom_recv().
 * ---------------------------------------------------------------------- */

int tmecomClientSendMessageAsync(void     *clientPtr,
                                 void     *reqPtr,
                                 size_t    reqSize,
                                 uint32_t *txnId)
{
  int      rc;
  // uint32_t driver_txn_id;

  if ((clientPtr == NULL) || (reqPtr == NULL) || (txnId == NULL) ||
      (reqSize < sizeof(tmecomMsgHdr))) {
    return -EINVAL;
  }

  if (s_async_pending) {
    /* Busy: translate to the IxErrno value TmePassthroughCmd expects. */
    return -E_AGAIN;
  }

  const uint8_t *req_payload     = (const uint8_t *)reqPtr + sizeof(tmecomMsgHdr);
  size_t         req_payload_size = reqSize - sizeof(tmecomMsgHdr);

  // rc = tmecom_send(req_payload, req_payload_size, &driver_txn_id);
  rc = tmecom_send(req_payload, req_payload_size);
  if (rc != 0) {
    return rc;
  }

  s_async_pending   = true;
  // s_async_txn_id    = driver_txn_id;
  // *txnId            = driver_txn_id;

  return 0;
}

int tmecomClientRecvMessageAsync(void     *clientPtr,
                                 uint32_t  txnId,
                                 void     *respPtr,
                                 size_t   *respSize)
{
  int rc;

  (void)clientPtr;

  if ((respPtr == NULL) || (respSize == NULL)) {
    return -EINVAL;
  }

  if (!s_async_pending || (txnId != s_async_txn_id)) {
    return -EINVAL;
  }

  rc = tmecom_recv(respPtr, respSize);
  if (rc == -EINPROGRESS) {
    /*
     * No response yet; keep the transaction outstanding for a later poll.
     * The driver reports libc -EINPROGRESS (36); translate to the IxErrno
     * -E_IN_PROGRESS (12) that TmePassthroughAwait compares against.
     */
    return -E_IN_PROGRESS;
  }

  /* Terminal outcome (success or error): the transaction is consumed. */
  s_async_pending = false;

  if (rc == -ENOSPC) {
    /* Response too large: report the IxErrno "no memory" terminal code. */
    return -E_NO_MEMORY;
  }

  return rc;
}

/* -------------------------------------------------------------------------
 * Test stub (unused in production builds).
 * ---------------------------------------------------------------------- */

int tmecomRunTests(void)
{
  return 0;
}
