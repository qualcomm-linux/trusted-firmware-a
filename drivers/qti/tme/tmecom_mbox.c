/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TMECOM - Trusted Management Engine Communication.
 *
 * Uses the QTI_mbox polling mailbox API.  Does not depend on GLink.
 *
 * Every message - request and response - is framed on the wire as:
 *
 *     [ struct tmecom_msg_hdr (8 bytes) | payload ]
 *
 * The CRC is computed by tme_calculate_crc16() (CRC-16/X-25: reflected poly
 * 0x8408, init 0xFFFF, final XOR 0xFFFF) over the PAYLOAD ONLY - the header
 * itself is not covered.  That helper is the protocol's own definition; do not
 * substitute a local CRC-16 variant and do not widen the range to include the
 * header, or the remote will reject the frame.
 * The remote echoes the request txn_id in the response.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <cdefs.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/libc/errno.h>
#include <lib/utils_def.h>

#include <drivers/qti/mbox/qti_mbox.h>
#include <drivers/qti/tmecom/tmecom.h>

#include <tmecom_crc.h>

#define TMECOM_POLL_MAX			100000U
#define TMECOM_CONNECT_TIMEOUT_US	5000000U
#define TMECOM_INITIAL_TXN_ID		0x00000001U

static struct qti_mbox_chan	*g_chan;
static bool			 g_connected;
static size_t			 g_mtu;

/* Transaction id handed to the next tmecom_send(); never 0 (see wrap below). */
static uint32_t			 g_txn_id;

/*
 * At most one transaction may be outstanding at a time (the remote does not
 * queue requests).  g_pending is set by tmecom_send() and cleared by
 * tmecom_recv() on any terminal outcome; g_pending_txn_id identifies it.
 */
static bool			 g_pending;
static uint32_t			 g_pending_txn_id;

/*
 * Framing bounce buffers.  Static rather than on-stack: BL31's per-CPU stack
 * is far too small for a ~2KB frame (same rationale as the tmecomMsgReq_t /
 * tmecomMsgRsp_t buffers in TmeMessage.c).
 *
 * g_recv_buf exists so a caller's response buffer only ever has to be large
 * enough for the PAYLOAD.  The framed message (header + payload) lands here
 * first and only the payload is copied out.  Receiving straight into the
 * caller's buffer would silently require it to be TMECOM_MSG_HDR_SIZE bytes
 * larger than the public API promises.
 */
static uint8_t g_send_buf[TMECOM_MAX_MSG_SIZE] __aligned(sizeof(uint32_t));
static uint8_t g_recv_buf[TMECOM_MAX_MSG_SIZE] __aligned(sizeof(uint32_t));

/*
 * tmecom_build_msg() - frame a request into g_send_buf.
 *
 * The header is assembled locally and memcpy'd in, so no alignment or
 * strict-aliasing assumptions are made about the buffer.
 *
 * Return: total framed size (header + payload) in bytes.
 */
static size_t tmecom_build_msg(const void *payload, size_t payload_size,
			       uint32_t txn_id)
{
	struct tmecom_msg_hdr hdr;

	hdr.version = (uint16_t)TMECOM_WIRE_VERSION;
	hdr.txn_id  = txn_id;
	/* CRC covers the payload only - see the file header comment. */
	hdr.crc     = tme_calculate_crc16(payload, payload_size);

	(void)memcpy(g_send_buf, &hdr, TMECOM_MSG_HDR_SIZE);
	(void)memcpy(g_send_buf + TMECOM_MSG_HDR_SIZE, payload, payload_size);

	return TMECOM_MSG_HDR_SIZE + payload_size;
}

/*
 * tmecom_validate_response() - validate the framing of the message currently
 * in g_recv_buf.
 *
 * Return: 0 if the frame is well-formed and matches @expected_txn_id,
 *         -EBADMSG otherwise.
 */
static int tmecom_validate_response(size_t msg_size, uint32_t expected_txn_id)
{
	struct tmecom_msg_hdr hdr;

	if (msg_size < TMECOM_MSG_HDR_SIZE) {
		ERROR("tmecom_recv: short frame:%zu\n", msg_size);
		return -EBADMSG;
	}

	(void)memcpy(&hdr, g_recv_buf, TMECOM_MSG_HDR_SIZE);

	if (hdr.version != (uint16_t)TMECOM_WIRE_VERSION) {
		ERROR("tmecom_recv: bad version:0x%04X\n", hdr.version);
		return -EBADMSG;
	}
	if (hdr.txn_id != expected_txn_id) {
		ERROR("tmecom_recv: txn_id got:0x%08X want:0x%08X\n",
		      hdr.txn_id, expected_txn_id);
		return -EBADMSG;
	}
	if (!tme_does_crc16_match(hdr.crc, g_recv_buf + TMECOM_MSG_HDR_SIZE,
			       msg_size - TMECOM_MSG_HDR_SIZE)) {
		ERROR("tmecom_recv: CRC mismatch hdr.crc:0x%04X\n", hdr.crc);
		return -EBADMSG;
	}

	return 0;
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
	uint32_t events;
	int rc;

	rc = qti_mbox_process(g_chan, &events);
	if (rc != 0) {
		return rc;
	}
	if ((events & QTI_MBOX_EVT_ERROR) != 0U) {
		return -EIO;
	}
	if ((events & (QTI_MBOX_EVT_DISCONNECTED |
		       QTI_MBOX_EVT_REMOTE_RESET)) != 0U) {
		g_connected = false;
		return -ENODEV;
	}
	if ((events & QTI_MBOX_EVT_RX_READY) != 0U) {
		return 1;
	}
	return 0;
}

/*
 * tmecom_fetch_mtu() - query and validate the channel MTU.
 *
 * Must run only after the transport reports CONNECTED: the QMP transport
 * returns -EAGAIN until the remote has published its shared-memory layout.
 *
 * On failure the channel is released and g_chan cleared, so the caller can
 * simply propagate the return value.
 *
 * Return: 0 on success, negative errno on failure.
 */
static int tmecom_fetch_mtu(void)
{
	int rc;

	rc = qti_mbox_get_mtu(g_chan, &g_mtu);
	if (rc != 0) {
		qti_mbox_release(g_chan);
		g_chan = NULL;
		ERROR("QTI_mbox_get_mtu err:%d\n", rc);
		return rc;
	}

	if (g_mtu <= TMECOM_MSG_HDR_SIZE) {
		qti_mbox_release(g_chan);
		g_chan = NULL;
		ERROR("QTI_mbox_get_mtu bad mtu:%zu\n", g_mtu);
		return -ENODEV;
	}

	return 0;
}

int tmecom_init(const char *channel_name)
{
	uint64_t deadline;
	uint32_t events;
	bool connected = false;
	int rc;

	if ((channel_name == NULL) || (channel_name[0] == '\0')) {
		return -EINVAL;
	}

	tmecom_deinit();

	rc = qti_mbox_request(channel_name, &g_chan);
	if (rc != 0) {
		ERROR("QTI_mbox_request failed err:%d\n", rc);
		return rc;
	}

	/*
	 * The transport connects during QTI_mbox_request().  Poll for
	 * the CONNECTED event; if not observed, check for ERROR only.
	 */
	deadline = timeout_init_us(TMECOM_CONNECT_TIMEOUT_US);
	do {
		rc = qti_mbox_process(g_chan, &events);
		if (rc != 0) {
			qti_mbox_release(g_chan);
			g_chan = NULL;
			ERROR("qti_mbox_process err:%d\n", rc);
			return rc;
		}
		if ((events & QTI_MBOX_EVT_ERROR) != 0U) {
			qti_mbox_release(g_chan);
			g_chan = NULL;
			ERROR("QTI_mbox_process QTI_MBOX_EVT_ERROR\n");
			return -EIO;
		}
		if ((events & QTI_MBOX_EVT_CONNECTED) != 0U) {
			rc = tmecom_fetch_mtu();
			if (rc != 0) {
				return rc;
			}
			connected = true;
			break;
		}
	} while (!timeout_elapsed(deadline));

	/*
	 * The poll expired without a CONNECTED event.  The remote may have
	 * connected before QTI_mbox_process() was first called, in which case
	 * the event was never observable; proceed, but fetch the MTU here since
	 * the connected path above did not run.  A successful MTU fetch implies
	 * the remote did publish a valid layout.
	 */
	if (!connected) {
		rc = tmecom_fetch_mtu();
		if (rc != 0) {
			return rc;
		}
	}

	g_connected = true;
	g_txn_id = TMECOM_INITIAL_TXN_ID;
	return 0;
}

void tmecom_deinit(void)
{
	if (g_chan != NULL) {
		qti_mbox_release(g_chan);
		g_chan = NULL;
	}
	g_connected = false;
	g_mtu = 0U;
	g_txn_id = 0U;
	g_pending = false;
	g_pending_txn_id = 0U;
	(void)memset(g_send_buf, 0, sizeof(g_send_buf));
	(void)memset(g_recv_buf, 0, sizeof(g_recv_buf));
}

int tmecom_send(const void *req, size_t req_size, uint32_t *txn_id)
{
	size_t   msg_size;
	uint32_t current_txn_id;
	int rc;

	if ((req == NULL) || (txn_id == NULL) || (req_size == 0U)) {
		return -EINVAL;
	}
	if ((g_chan == NULL) || !g_connected) {
		return -ENODEV;
	}
	if (req_size > TMECOM_MAX_PAYLOAD_SIZE) {
		return -EMSGSIZE;
	}
	/*
	 * TMECOM_MAX_PAYLOAD_SIZE is the buffer bound; the negotiated MTU is
	 * usually smaller, so the framed size must be checked against it too.
	 */
	if ((req_size + TMECOM_MSG_HDR_SIZE) > g_mtu) {
		return -EMSGSIZE;
	}
	if (g_pending) {
		return -EAGAIN;
	}

	current_txn_id = g_txn_id;
	g_txn_id++;
	if (g_txn_id == 0U) {
		g_txn_id = TMECOM_INITIAL_TXN_ID;
	}

	msg_size = tmecom_build_msg(req, req_size, current_txn_id);

	rc = qti_mbox_send(g_chan, g_send_buf, msg_size);
	if (rc != 0) {
		return rc;
	}

	g_pending = true;
	g_pending_txn_id = current_txn_id;
	*txn_id = current_txn_id;
	return 0;
}

int tmecom_recv(uint32_t txn_id, void *rsp, size_t *rsp_size)
{
	size_t recv_size;
	size_t payload_size;
	int rc;

	if ((rsp == NULL) || (rsp_size == NULL)) {
		return -EINVAL;
	}
	/*
	 * Refuse to consume a message unless it belongs to the outstanding
	 * transaction; otherwise a stray/unsolicited frame could be accepted
	 * and validated against a stale txn id.
	 */
	if (!g_pending || (txn_id != g_pending_txn_id)) {
		return -EINVAL;
	}
	if ((g_chan == NULL) || !g_connected) {
		g_pending = false;
		return -ENODEV;
	}

	rc = tmecom_probe_rx();
	if (rc == 0) {
		/* No response yet; keep the transaction outstanding. */
		return -EINPROGRESS;
	}
	if (rc < 0) {
		g_pending = false;
		return rc;
	}

	/* From here on the transaction is consumed regardless of outcome. */
	g_pending = false;

	recv_size = sizeof(g_recv_buf);
	rc = qti_mbox_recv(g_chan, g_recv_buf, &recv_size);
	if (rc == -ENOSPC) {
		ERROR("QTI_mbox_recv ENOSPC need:%zu\n", recv_size);
		return -EIO;
	}
	if (rc != 0) {
		ERROR("QTI_mbox_recv err:%d\n", rc);
		return rc;
	}

	rc = tmecom_validate_response(recv_size, txn_id);
	if (rc != 0) {
		return rc;
	}

	payload_size = recv_size - TMECOM_MSG_HDR_SIZE;
	if (payload_size > *rsp_size) {
		/* Report the required capacity so the caller can retry. */
		*rsp_size = payload_size;
		return -ENOSPC;
	}

	(void)memcpy(rsp, g_recv_buf + TMECOM_MSG_HDR_SIZE, payload_size);
	*rsp_size = payload_size;

	return 0;
}

int tmecom_send_recv(const void *req, size_t req_size,
		     void *rsp, size_t *rsp_size)
{
	uint32_t txn_id = 0U;
	uint32_t poll;
	int rc;

	if ((rsp == NULL) || (rsp_size == NULL)) {
		return -EINVAL;
	}

	rc = tmecom_send(req, req_size, &txn_id);
	if (rc != 0) {
		return rc;
	}

	for (poll = 0U; poll < TMECOM_POLL_MAX; poll++) {
		size_t rsp_capacity = *rsp_size;

		rc = tmecom_recv(txn_id, rsp, &rsp_capacity);
		if (rc != -EINPROGRESS) {
			*rsp_size = rsp_capacity;
			return rc;
		}
	}

	/* Timed out waiting for the remote; drop the stale transaction. */
	g_pending = false;
	return -ETIMEDOUT;
}

bool tmecom_is_connected(void)
{
	uint32_t events;
	int rc;

	if (g_chan == NULL) {
		return false;
	}

	rc = qti_mbox_process(g_chan, &events);
	if (rc != 0) {
		g_connected = false;
		return g_connected;
	}
	if ((events & (QTI_MBOX_EVT_DISCONNECTED |
		       QTI_MBOX_EVT_REMOTE_RESET |
		       QTI_MBOX_EVT_ERROR)) != 0U) {
		g_connected = false;
	}
	if ((events & QTI_MBOX_EVT_CONNECTED) != 0U) {
		g_connected = true;
	}
	return g_connected;
}
