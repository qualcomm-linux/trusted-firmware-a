/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stringl.h>

#include <cdefs.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/libc/errno.h>
#include <lib/utils_def.h>

#include <drivers/qti/mbox/qti_mbox.h>

#include <tmecom.h>
#include <tmecom_tfa.h>
#include <tmecom_os_al.h>

/* -------------------------------------------------------------------------
 * Transport constants
 * ---------------------------------------------------------------------- */

#define TMECOM_POLL_MAX			100000U
#define TMECOM_CONNECT_TIMEOUT_US	5000000U
#define TMECOM_INITIAL_TXN_ID		0x00000001U

#define TMECOM_MSG_HDR_SIZE		sizeof(tmecom_msg_hdr_t)

/* Maximum on-wire message size (header + payload). */
#define TMECOM_MAX_MSG_SIZE		2048U

/* Maximum payload the caller may pass to the transport send primitive. */
#define TMECOM_MAX_PAYLOAD_SIZE		(TMECOM_MAX_MSG_SIZE - TMECOM_MSG_HDR_SIZE)

/* -------------------------------------------------------------------------
 * CRC-16/X-25 (a.k.a. reflected CCITT): reflected polynomial 0x8408,
 * initial value 0xFFFF, final XOR 0xFFFF.  This must match the TME firmware
 * implementation (tme_calculate_crc16 in crc16.c), which is applied over the
 * message *payload only* (excluding the tmecom_msg_hdr_t).
 * ---------------------------------------------------------------------- */

static uint16_t crc16_update(uint16_t crc, uint8_t byte)
{
	uint32_t i;

	crc ^= (uint16_t)byte;
	for (i = 0U; i < 8U; i++) {
		if ((crc & 0x0001U) != 0U) {
			crc = (uint16_t)((crc >> 1U) ^ 0x8408U);
		} else {
			crc = (uint16_t)(crc >> 1U);
		}
	}
	return crc;
}

static uint16_t crc16_compute(const uint8_t *buf, size_t len)
{
	uint16_t crc = 0xFFFFU;
	size_t i;

	for (i = 0U; i < len; i++) {
		crc = crc16_update(crc, buf[i]);
	}
	return (uint16_t)(crc ^ 0xFFFFU);
}

/* -------------------------------------------------------------------------
 * Transport state
 * ---------------------------------------------------------------------- */

static struct qti_mbox_chan	*g_chan;
static uint32_t			 g_txn_id;
static bool			 g_connected;
static size_t			 g_mtu;

/*
 * At most one transaction may be outstanding at a time (the remote does not
 * queue requests).  g_pending is set by tmecom_send() and cleared by
 * tmecom_recv() on any terminal outcome; g_pending_txn_id identifies it.
 */
static bool			 g_pending;
static uint32_t			 g_pending_txn_id;

static uint8_t g_send_buf[TMECOM_MAX_MSG_SIZE] __aligned(sizeof(uint32_t));
static uint8_t g_recv_buf[TMECOM_MAX_MSG_SIZE] __aligned(sizeof(uint32_t));

/* Forward declaration: tmecom_init() releases any prior channel via this. */
static void tmecom_deinit(void);

/* -------------------------------------------------------------------------
 * Transport helpers
 * ---------------------------------------------------------------------- */

static size_t tmecom_build_msg(const void *payload, size_t payload_size,
			       uint32_t txn_id)
{
	tmecom_msg_hdr_t *hdr;
	size_t msg_size = TMECOM_MSG_HDR_SIZE + payload_size;

	hdr = (tmecom_msg_hdr_t *)(void *)g_send_buf;
	hdr->version = (uint16_t)TMECOM_VERSION(TMECOM_VERSION_MAJOR,
						TMECOM_VERSION_MINOR);
	hdr->crc     = 0U;
	hdr->txn_id   = txn_id;
	(void)memscpy(g_send_buf + TMECOM_MSG_HDR_SIZE,
		      TMECOM_MAX_PAYLOAD_SIZE, payload, payload_size);
	/* CRC covers the payload only (excluding the header), matching TME. */
	hdr->crc = crc16_compute(g_send_buf + TMECOM_MSG_HDR_SIZE, payload_size);
	return msg_size;
}

static int tmecom_validate_response(size_t msg_size, uint32_t expected_txn_id)
{
	const tmecom_msg_hdr_t *hdr;
	uint16_t received_crc;
	uint16_t computed_crc;
	int ret = -EBADMSG;

	do {
		if (msg_size < TMECOM_MSG_HDR_SIZE) {
			break;
		}

		hdr = (const tmecom_msg_hdr_t *)(const void *)g_recv_buf;
		if (hdr->version != (uint16_t)TMECOM_VERSION(TMECOM_VERSION_MAJOR,
								TMECOM_VERSION_MINOR)) {
			break;
		}

		received_crc = hdr->crc;
		/* CRC covers the payload only (excluding the header), matching TME. */
		computed_crc = crc16_compute(g_recv_buf + TMECOM_MSG_HDR_SIZE,
					     msg_size - TMECOM_MSG_HDR_SIZE);
		if (computed_crc != received_crc) {
			break;
		}

		if (hdr->txn_id != expected_txn_id) {
			break;
		}

		ret = 0;
	} while (0);

	return ret;
}

/*
 * tmecom_probe_rx() - advance the transport once and report whether a
 * response is ready.
 *
 * Return: 1 if a message is ready, 0 if not yet ready, negative errno on a
 *         transport error / disconnect.
 */
static int tmecom_probe_rx(void)
{
	uint32_t events = 0U;
	int rc = -EIO;

	do {
		rc = qti_mbox_process(g_chan, &events);
		if (rc != 0) {
			break;
		}
		if ((events & QTI_MBOX_EVT_ERROR) != 0U) {
			rc = -EIO;
			break;
		}
		if ((events & (QTI_MBOX_EVT_DISCONNECTED |
			       QTI_MBOX_EVT_REMOTE_RESET)) != 0U) {
			g_connected = false;
			rc = -ENODEV;
			break;
		}
		if ((events & QTI_MBOX_EVT_RX_READY) != 0U) {
			rc = 1;
		}
	} while (0);

	return rc;
}

/* -------------------------------------------------------------------------
 * Transport primitives (static; callers use the tmecom_client_t API below)
 * ---------------------------------------------------------------------- */

static int tmecom_init(const char *channel_name)
{
	uint64_t deadline = 0U;
	uint32_t events = 0U;
	int rc = -EINVAL;

	do {
		if ((channel_name == NULL) || (channel_name[0] == '\0')) {
			break;
		}

		tmecom_deinit();
		rc = qti_mbox_request(channel_name, &g_chan);
		if (rc != 0) {
			break;
		}

		deadline = timeout_init_us(TMECOM_CONNECT_TIMEOUT_US);
		do {
			rc = qti_mbox_process(g_chan, &events);
			if (rc != 0) {
				break;
			}
			if ((events & QTI_MBOX_EVT_ERROR) != 0U) {
				rc = -EIO;
				break;
			}
			if ((events & QTI_MBOX_EVT_CONNECTED) != 0U) {
				rc = 0;
				break;
			}
		} while (!timeout_elapsed(deadline));
		if (rc != 0) {
			break;
		}
		if ((events & QTI_MBOX_EVT_CONNECTED) == 0U) {
			rc = -ETIMEDOUT;
			break;
		}

		rc = qti_mbox_get_mtu(g_chan, &g_mtu);
		if (rc != 0) {
			break;
		}
		if (g_mtu <= TMECOM_MSG_HDR_SIZE) {
			rc = -ENODEV;
			break;
		}

		g_connected = true;
		g_txn_id = TMECOM_INITIAL_TXN_ID;
	} while (0);

	if (rc != 0) {
		tmecom_deinit();
	}
	return rc;
}

static void tmecom_deinit(void)
{
	if (g_chan != NULL) {
		qti_mbox_release(g_chan);
		g_chan = NULL;
	}
	g_connected = false;
	g_txn_id = 0U;
	g_mtu = 0U;
	g_pending = false;
	g_pending_txn_id = 0U;
	(void)memset(g_send_buf, 0, sizeof(g_send_buf));
	(void)memset(g_recv_buf, 0, sizeof(g_recv_buf));
}

static int tmecom_send(const void *req, size_t req_size, uint32_t *txn_id)
{
	size_t msg_size = 0U;
	uint32_t current_txn_id = 0U;
	int rc = -EINVAL;

	do {
		if ((req == NULL) || (txn_id == NULL) || (req_size == 0U)) {
			break;
		}
		if ((g_chan == NULL) || !g_connected) {
			rc = -ENODEV;
			break;
		}
		if ((req_size > TMECOM_MAX_PAYLOAD_SIZE) ||
			(req_size + TMECOM_MSG_HDR_SIZE > g_mtu)) {
			rc = -EMSGSIZE;
			break;
		}
		if (g_pending) {
			rc = -EAGAIN;
			break;
		}

		current_txn_id = g_txn_id;
		g_txn_id++;
		if (g_txn_id == 0U) {
			g_txn_id = TMECOM_INITIAL_TXN_ID;
		}

		msg_size = tmecom_build_msg(req, req_size, current_txn_id);
		rc = qti_mbox_send(g_chan, g_send_buf, msg_size);
		if (rc != 0) {
			break;
		}

		g_pending = true;
		g_pending_txn_id = current_txn_id;
		*txn_id = current_txn_id;
	} while (0);

	return rc;
}

static int tmecom_recv(uint32_t txn_id, void *rsp, size_t *rsp_size)
{
	size_t recv_size = sizeof(g_recv_buf);
	size_t payload_size = 0U;
	int rc = -EINVAL;

	do {
		if ((rsp == NULL) || (rsp_size == NULL) ||
			(!g_pending || (txn_id != g_pending_txn_id))) {
			break;
		}
		if ((g_chan == NULL) || !g_connected) {
			g_pending = false;
			rc = -ENODEV;
			break;
		}

		rc = tmecom_probe_rx();
		if (rc == 0) {
			rc = -EINPROGRESS;
			break;
		}
		if (rc < 0) {
			g_pending = false;
			break;
		}

		g_pending = false;
		rc = qti_mbox_recv(g_chan, g_recv_buf, &recv_size);
		if (rc != 0) {
			break;
		}

		rc = tmecom_validate_response(recv_size, txn_id);
		if (rc != 0) {
			break;
		}

		payload_size = recv_size - TMECOM_MSG_HDR_SIZE;
		if (payload_size > *rsp_size) {
			*rsp_size = payload_size;
			rc = -ENOSPC;
			break;
		}

		(void)memscpy(rsp, *rsp_size,
			      g_recv_buf + TMECOM_MSG_HDR_SIZE, payload_size);
		*rsp_size = payload_size;
		rc = 0;
	} while (0);

	return rc;
}

static int tmecom_send_recv(const void *req, size_t req_size,
			    void *rsp, size_t *rsp_size)
{
	uint32_t txn_id;
	uint32_t poll;
	int rc;

	do {
		if ((rsp == NULL) || (rsp_size == NULL)) {
			rc = -EINVAL;
			break;
		}

		rc = tmecom_send(req, req_size, &txn_id);
		if (rc != 0) {
			break;
		}

		for (poll = 0U; poll < TMECOM_POLL_MAX; poll++) {
			size_t rsp_capacity = *rsp_size;

			rc = tmecom_recv(txn_id, rsp, &rsp_capacity);
			if (rc != -EINPROGRESS) {
				*rsp_size = rsp_capacity;
				break;
			}
		}
		if (poll == TMECOM_POLL_MAX) {
			/* Timed out waiting for the remote; drop the stale transaction. */
			ERROR("tmecom_send_recv: TIMEOUT after %u polls\n", TMECOM_POLL_MAX);
			g_pending = false;
			rc = -ETIMEDOUT;
		}
	} while (0);

	return rc;
}

static bool tmecom_is_connected(void)
{
	uint32_t events = 0U;
	bool connected = false;

	do {
		if (g_chan == NULL) {
			break;
		}

		if (qti_mbox_process(g_chan, &events) != 0) {
			g_connected = false;
			break;
		}
		if ((events & (QTI_MBOX_EVT_DISCONNECTED |
			       QTI_MBOX_EVT_REMOTE_RESET |
			       QTI_MBOX_EVT_ERROR)) != 0U) {
			g_connected = false;
		}
		if ((events & QTI_MBOX_EVT_CONNECTED) != 0U) {
			g_connected = true;
		}
		connected = g_connected;
	} while (0);

	return connected;
}

/* -------------------------------------------------------------------------
 * Static allocations for request/response framed buffers (matched to the
 * sizes declared in tmecom.h so tmeintf can use them as before).
 * ---------------------------------------------------------------------- */

static tmecom_msg_req_t s_req_buf;
static tmecom_msg_rsp_t s_rsp_buf;

void *tmecom_alloc_req(size_t size)
{
	if (size <= sizeof(s_req_buf)) {
		return &s_req_buf;
	}
	return NULL;
}

void tmecom_free_req(void *p_mem)
{
	/* No-op because the tmecom buffer is statically allocated. */
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
	/* No-op because the tmecom buffer is statically allocated. */
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
 * tmecom_client_t - opaque handle.  One static instance; registration just
 * opens the channel via the transport layer.
 * ---------------------------------------------------------------------- */

struct tmecom_client_t {
	bool registered;
};

static tmecom_client_t s_client;

/* -------------------------------------------------------------------------
 * Client register / unregister.
 * ---------------------------------------------------------------------- */

int tmecom_register_client(const tmecom_client_info_t *info,
			   tmecom_client_t **client_ptr)
{
	int rc = -EINVAL;

	do {
		if ((info == NULL) || (client_ptr == NULL)) {
			break;
		}

		if (s_client.registered) {
			/* Already open; return the existing client. */
			*client_ptr = &s_client;
			rc = 0;
			break;
		}

		rc = tmecom_init(info->channel_name);
		if (rc != 0) {
			*client_ptr = NULL;
			break;
		}

		s_client.registered = true;
		*client_ptr = &s_client;
	} while (0);

	return rc;
}

int tmecom_unregister_client(tmecom_client_t *client_ptr)
{
	int ret = -EINVAL;

	do {
		if (client_ptr == NULL) {
			break;
		}

		tmecom_deinit();
		s_client.registered = false;
		s_async_pending     = false;
		s_async_txn_id      = 0U;
		ret = 0;
	} while (0);

	return ret;
}

/* -------------------------------------------------------------------------
 * Connection query.
 * ---------------------------------------------------------------------- */

bool tmecom_client_is_server_connected(tmecom_client_t *client_ptr)
{
	if ((client_ptr == NULL) || !client_ptr->registered) {
		return false;
	}
	return tmecom_is_connected();
}

bool tmecom_is_tme_subsystem_link_up(void)
{
	return tmecom_is_connected();
}

/* -------------------------------------------------------------------------
 * Synchronous send/receive.
 *
 * tmeintf passes a full tmecom_msg_req_t (header + encoded payload) as @req_ptr
 * and expects a full tmecom_msg_rsp_t (header + encoded response) in @resp_ptr.
 * tmecom_send_recv() owns the framing internally, so we strip the header from
 * the request payload, call send_recv, then rebuild the response with the
 * header so tmeintf's validation code keeps working.
 * ---------------------------------------------------------------------- */

int tmecom_client_send_message_sync(void     *client_ptr,
				void     *req_ptr,
				size_t    req_size,
				void     *resp_ptr,
				size_t   *resp_size,
				uint32_t  timeout_msec)
{
	tmecom_msg_rsp_t *rsp = (tmecom_msg_rsp_t *)resp_ptr;
	size_t          payload_size;
	size_t          rsp_payload_capacity;
	int             rc;

	(void)timeout_msec; /* transport handles its own timeout internally */

	rc = -EINVAL;
	do {
		if ((client_ptr != &s_client) || !s_client.registered ||
			(req_ptr == NULL) || (resp_ptr == NULL) ||
			(resp_size == NULL) || (req_size < sizeof(tmecom_msg_hdr_t))) {
			break;
		}

		/* Payload starts after the header. */
		const uint8_t *req_payload = (const uint8_t *)req_ptr +
			sizeof(tmecom_msg_hdr_t);
		size_t         req_payload_size = req_size - sizeof(tmecom_msg_hdr_t);

		rsp_payload_capacity = sizeof(rsp->enc_rsp_buf);

		rc = tmecom_send_recv(req_payload, req_payload_size,
						rsp->enc_rsp_buf, &rsp_payload_capacity);
		if (rc != 0) {
			break;
		}

		payload_size = rsp_payload_capacity;

		/* Fill in a synthetic response header so tmeintf validation passes. */
		rsp->hdr.version = (uint16_t)TMECOM_VERSION(TMECOM_VERSION_MAJOR,
						  TMECOM_VERSION_MINOR);
		rsp->hdr.txn_id   = ((const tmecom_msg_hdr_t *)req_ptr)->txn_id;
		rsp->hdr.crc     = crc16_compute(rsp->enc_rsp_buf, payload_size);

		*resp_size = sizeof(tmecom_msg_hdr_t) + payload_size;
		rc = 0;
	} while (0);

	return rc;
}

/* -------------------------------------------------------------------------
 * Async send/receive.
 *
 * SendMessageAsync frames the request and hands it to the mailbox, returning
 * as soon as the local transport has accepted it -- it does not wait for the
 * TME SS to respond.  RecvMessageAsync advances the transport once and either
 * copies out a ready response or reports -EINPROGRESS.
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
	uint32_t driver_txn_id;

	rc = -EINVAL;
	do {
		if ((client_ptr == NULL) || (client_ptr != &s_client) ||
		    !s_client.registered || (req_ptr == NULL) || (txn_id == NULL) ||
			(req_size < sizeof(tmecom_msg_hdr_t))) {
			break;
		}

		if (s_async_pending) {
			rc = -EAGAIN;
			break;
		}

		const uint8_t *req_payload = (const uint8_t *)req_ptr +
			sizeof(tmecom_msg_hdr_t);
		size_t         req_payload_size = req_size - sizeof(tmecom_msg_hdr_t);

		rc = tmecom_send(req_payload, req_payload_size, &driver_txn_id);
		if (rc != 0) {
			break;
		}

		s_async_pending   = true;
		s_async_txn_id    = driver_txn_id;
		*txn_id           = driver_txn_id;

		rc = 0;
	} while (0);

	return rc;
}

int tmecom_client_recv_message_async(void     *client_ptr,
				 uint32_t  txn_id,
				 void     *resp_ptr,
				 size_t   *resp_size)
{
	int rc;

	rc = -EINVAL;
	do {
		if ((client_ptr == NULL) || (client_ptr != &s_client) ||
		    !s_client.registered || (resp_ptr == NULL) || (resp_size == NULL)) {
			break;
		}

		if (!s_async_pending || (txn_id != s_async_txn_id)) {
			break;
		}

		rc = tmecom_recv(txn_id, resp_ptr, resp_size);
		if (rc == -EINPROGRESS) {
			break;
		}

		/* Terminal outcome (success or error): the transaction is consumed. */
		s_async_pending = false;

	} while (0);

	return rc;
}
