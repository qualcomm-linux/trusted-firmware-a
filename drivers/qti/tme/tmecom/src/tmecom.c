/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Shim layer: adapts the legacy tmecom client API (tmecom_register_client,
 * tmecom_client_send_message_sync, tmecom_client_send_message_async, etc.) used by
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

void *tmecom_alloc_req(size_t size)
{
	if (size <= sizeof(s_req_buf)) {
		return &s_req_buf;
	}
	return NULL;
}

void tmecom_free_req(void *p_mem)
{
	(void)p_mem;
}

void *tmecom_alloc_rsp(size_t size)
{
	if (size <= sizeof(s_rsp_buf)) {
		return &s_rsp_buf;
	}
	return NULL;
}

void tmecom_free_rsp(void *p_mem)
{
	(void)p_mem;
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
};

static struct tmecomClient s_client;

/* -------------------------------------------------------------------------
 * Legacy init / deinit (called from tmecom_interface_init path).
 * ---------------------------------------------------------------------- */

int tmecom_init_legacy(eTMEComInterface tmecomInterface)
{
	(void)tmecomInterface;
	/* Channel is opened on tmecom_register_client; nothing to do here. */
	return 0;
}

int tmecom_deinit_legacy(void)
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

int tmecom_register_client(const tmecomClientInfo *info, tmecomClient **client_ptr)
{
	int rc;

	if ((info == NULL) || (client_ptr == NULL)) {
		return -EINVAL;
	}

	if (s_client.registered) {
		/* Already open; return the existing client. */
		*client_ptr = &s_client;
		return 0;
	}

	rc = tmecom_init(info->channel_name);
	if (rc != 0) {
		*client_ptr = NULL;
		return rc;
	}

	s_client.registered = true;
	*client_ptr = &s_client;
	return 0;
}

int tmecom_unregister_client(tmecomClient *client_ptr)
{
	if (client_ptr == NULL) {
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

bool tmecom_client_is_server_connected(tmecomClient *client_ptr)
{
	bool connected;

	if ((client_ptr == NULL) || !client_ptr->registered) {
		return false;
	}
	connected = tmecom_is_connected();
	return connected;
}

bool tmecom_is_tme_subsystem_link_up(void)
{
	return tmecom_is_connected();
}

/* -------------------------------------------------------------------------
 * Synchronous send/receive.
 *
 * tmeintf passes a full tmecomMsgReq_t (header + encoded payload) as @req_ptr
 * and expects a full tmecomMsgRsp_t (header + encoded response) in @resp_ptr.
 * The new tmecom_send_recv() owns the framing internally, so we strip the
 * header from the request payload, call send_recv, then rebuild the response
 * with the header so tmeintf's validation code keeps working.
 * ---------------------------------------------------------------------- */

int tmecom_client_send_message_sync(void     *client_ptr,
		void     *req_ptr,
		size_t    req_size,
		void     *resp_ptr,
		size_t   *resp_size,
		uint32_t  timeout_msec)
{
	tmecomMsgRsp_t *rsp = (tmecomMsgRsp_t *)resp_ptr;
	size_t          payload_size;
	size_t          rsp_payload_capacity;
	int             rc;

	(void)timeout_msec; /* new driver handles its own timeout internally */

	if ((client_ptr == NULL) || (req_ptr == NULL) || (resp_ptr == NULL) ||
			(resp_size == NULL) || (req_size < sizeof(tmecomMsgHdr))) {
		return -EINVAL;
	}

	/* Payload starts after the header. */
	const uint8_t *req_payload     = (const uint8_t *)req_ptr + sizeof(tmecomMsgHdr);
	size_t         req_payload_size = req_size - sizeof(tmecomMsgHdr);

	rsp_payload_capacity = sizeof(rsp->enc_rsp_buf);

	rc = tmecom_send_recv(req_payload, req_payload_size,
		rsp->enc_rsp_buf, &rsp_payload_capacity);
	if (rc != 0) {
		return rc;
	}

	payload_size = rsp_payload_capacity;

	/* Fill in a synthetic response header so tmeintf validation passes. */
	rsp->hdr.version = (uint16_t)TMECOM_VERSION(1, 0);
	rsp->hdr.txn_id   = ((const tmecomMsgHdr *)req_ptr)->txn_id;
	rsp->hdr.crc     = tme_calculate_crc16(rsp->enc_rsp_buf, payload_size);

	*resp_size = sizeof(tmecomMsgHdr) + payload_size;
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

int tmecom_client_send_message_async(void     *client_ptr,
		 void     *req_ptr,
		 size_t    req_size,
		 uint32_t *txn_id)
{
	int      rc;
	uint32_t driver_txn_id = 0U;

	if ((client_ptr == NULL) || (req_ptr == NULL) || (txn_id == NULL) ||
			(req_size < sizeof(tmecomMsgHdr))) {
		return -EINVAL;
	}

	if (s_async_pending) {
		/* Busy: translate to the IxErrno value tme_passthrough_cmd expects. */
		return -E_AGAIN;
	}

	const uint8_t *req_payload     = (const uint8_t *)req_ptr + sizeof(tmecomMsgHdr);
	size_t         req_payload_size = req_size - sizeof(tmecomMsgHdr);

	rc = tmecom_send(req_payload, req_payload_size, &driver_txn_id);
	if (rc != 0) {
		return rc;
	}

	s_async_pending   = true;
	s_async_txn_id    = driver_txn_id;
	*txn_id            = driver_txn_id;

	return 0;
}

int tmecom_client_recv_message_async(void     *client_ptr,
		 uint32_t  txn_id,
		 void     *resp_ptr,
		 size_t   *resp_size)
{
	int rc;

	(void)client_ptr;

	if ((resp_ptr == NULL) || (resp_size == NULL)) {
		return -EINVAL;
	}

	if (!s_async_pending || (txn_id != s_async_txn_id)) {
		return -EINVAL;
	}

	rc = tmecom_recv(txn_id, resp_ptr, resp_size);
	if (rc == -EINPROGRESS) {
		/*
		 * No response yet; keep the transaction outstanding for a later poll.
		 * The driver reports libc -EINPROGRESS (36); translate to the IxErrno
		 * -E_IN_PROGRESS (12) that tme_passthrough_await compares against.
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

int tmecom_run_tests(void)
{
	return 0;
}
