/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TMECOM - Trusted Management Engine Communication.
 *
 * Uses the qcom_mbox polling mailbox API.  Does not depend on GLink.
 *
 * Every message - request and response - is framed on the wire as:
 *
 *     [ struct tmecom_msg_hdr (8 bytes) | payload ]
 *
 * The CRC is computed by tmeCalculateCRC16() (CRC-16/X-25: reflected poly
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

#include <drivers/qti/mbox/qcom_mbox.h>
#include <drivers/qti/tmecom/tmecom.h>

#include <tmecom_crc.h>

#define TMECOM_POLL_MAX			100000U
#define TMECOM_CONNECT_TIMEOUT_US	5000000U
#define TMECOM_INITIAL_TXN_ID		0x00000001U

static struct qcom_mbox_chan	*g_chan;
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
	hdr.crc     = tmeCalculateCRC16(payload, payload_size);

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
	if (!tmeDoesCRC16Match(hdr.crc, g_recv_buf + TMECOM_MSG_HDR_SIZE,
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

	VERBOSE("%s: ENTRY\n", __func__);

	events = qcom_mbox_process(g_chan);
	if ((events & QCOM_MBOX_EVT_ERROR) != 0U) {
		VERBOSE("%s: EXIT ret=-EIO (transport error)\n", __func__);
		return -EIO;
	}
	if ((events & (QCOM_MBOX_EVT_DISCONNECTED |
		       QCOM_MBOX_EVT_REMOTE_RESET)) != 0U) {
		g_connected = false;
		VERBOSE("%s: EXIT ret=-ENODEV (disconnected)\n", __func__);
		return -ENODEV;
	}
	if ((events & QCOM_MBOX_EVT_RX_READY) != 0U) {
		VERBOSE("%s: EXIT ret=1 (rx ready)\n", __func__);
		return 1;
	}
	VERBOSE("%s: EXIT ret=0 events=0x%08X\n", __func__, events);
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

	INFO("%s: ENTRY\n", __func__);

	rc = qcom_mbox_get_mtu(g_chan, &g_mtu);
	if (rc != 0) {
		qcom_mbox_release(g_chan);
		g_chan = NULL;
		ERROR("qcom_mbox_get_mtu err:%d\n", rc);
		INFO("%s: EXIT ret=%d (qcom_mbox_get_mtu failed)\n", __func__,
		     rc);
		return rc;
	}

	if (g_mtu <= TMECOM_MSG_HDR_SIZE) {
		qcom_mbox_release(g_chan);
		g_chan = NULL;
		ERROR("qcom_mbox_get_mtu bad mtu:%zu\n", g_mtu);
		INFO("%s: EXIT ret=-ENODEV (mtu too small)\n", __func__);
		return -ENODEV;
	}

	INFO("%s: EXIT ret=0 mtu=%zu\n", __func__, g_mtu);
	return 0;
}

int tmecom_init(const char *channel_name)
{
	uint64_t deadline;
	uint32_t events;
	bool connected = false;
	int rc;

	INFO("%s: ENTRY channel_name:%s\n", __func__,
	     (channel_name != NULL) ? channel_name : "(null)");

	if ((channel_name == NULL) || (channel_name[0] == '\0')) {
		INFO("%s: EXIT ret=-EINVAL\n", __func__);
		return -EINVAL;
	}

	tmecom_deinit();

	INFO("qcom_mbox_request ch:%s\n", channel_name);
	rc = qcom_mbox_request(channel_name, &g_chan);
	if (rc != 0) {
		ERROR("qcom_mbox_request failed err:%d\n", rc);
		INFO("%s: EXIT ret=%d (qcom_mbox_request failed)\n", __func__,
		     rc);
		return rc;
	}
	INFO("qcom_mbox_request ch:%s success\n", channel_name);

	/*
	 * The transport connects during qcom_mbox_request().  Poll for
	 * the CONNECTED event; if not observed, check for ERROR only.
	 */
	deadline = timeout_init_us(TMECOM_CONNECT_TIMEOUT_US);
	do {
		events = qcom_mbox_process(g_chan);
		INFO("qcom_mbox_process events:0x%08X\n", events);
		if ((events & QCOM_MBOX_EVT_ERROR) != 0U) {
			qcom_mbox_release(g_chan);
			g_chan = NULL;
			ERROR("qcom_mbox_process QCOM_MBOX_EVT_ERROR\n");
			INFO("%s: EXIT ret=-EIO (QCOM_MBOX_EVT_ERROR)\n",
			     __func__);
			return -EIO;
		}
		if ((events & QCOM_MBOX_EVT_CONNECTED) != 0U) {
			INFO("qcom_mbox_process QCOM_MBOX_EVT_CONNECTED\n");
			rc = tmecom_fetch_mtu();
			if (rc != 0) {
				INFO("%s: EXIT ret=%d (tmecom_fetch_mtu failed)\n",
				     __func__, rc);
				return rc;
			}
			connected = true;
			break;
		}
	} while (!timeout_elapsed(deadline));

	/*
	 * The poll expired without a CONNECTED event.  The remote may have
	 * connected before qcom_mbox_process() was first called, in which case
	 * the event was never observable; proceed, but fetch the MTU here since
	 * the connected path above did not run.  A successful MTU fetch implies
	 * the remote did publish a valid layout.
	 */
	if (!connected) {
		rc = tmecom_fetch_mtu();
		if (rc != 0) {
			INFO("%s: EXIT ret=%d (tmecom_fetch_mtu failed, late path)\n",
			     __func__, rc);
			return rc;
		}
	}

	INFO("qcom_mbox_request ch:%s connected\n", channel_name);
	g_connected = true;
	g_txn_id = TMECOM_INITIAL_TXN_ID;
	INFO("%s: EXIT ret=0\n", __func__);
	return 0;
}

void tmecom_deinit(void)
{
	if (g_chan != NULL) {
		qcom_mbox_release(g_chan);
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

	INFO("%s: ENTRY req_size:%zu\n", __func__, req_size);

	if ((req == NULL) || (txn_id == NULL) || (req_size == 0U)) {
		INFO("%s: EXIT ret=-EINVAL\n", __func__);
		return -EINVAL;
	}
	if ((g_chan == NULL) || !g_connected) {
		INFO("%s: EXIT ret=-ENODEV\n", __func__);
		return -ENODEV;
	}
	if (req_size > TMECOM_MAX_PAYLOAD_SIZE) {
		INFO("%s: EXIT ret=-EMSGSIZE (req_size:%zu > max:%zu)\n",
		     __func__, req_size, (size_t)TMECOM_MAX_PAYLOAD_SIZE);
		return -EMSGSIZE;
	}
	/*
	 * TMECOM_MAX_PAYLOAD_SIZE is the buffer bound; the negotiated MTU is
	 * usually smaller, so the framed size must be checked against it too.
	 */
	if ((req_size + TMECOM_MSG_HDR_SIZE) > g_mtu) {
		INFO("%s: EXIT ret=-EMSGSIZE (framed:%zu > mtu:%zu)\n",
		     __func__, req_size + TMECOM_MSG_HDR_SIZE, g_mtu);
		return -EMSGSIZE;
	}
	if (g_pending) {
		INFO("%s: EXIT ret=-EAGAIN (txn already pending)\n", __func__);
		return -EAGAIN;
	}

	current_txn_id = g_txn_id;
	g_txn_id++;
	if (g_txn_id == 0U) {
		g_txn_id = TMECOM_INITIAL_TXN_ID;
	}

	msg_size = tmecom_build_msg(req, req_size, current_txn_id);

	rc = qcom_mbox_send(g_chan, g_send_buf, msg_size);
	if (rc != 0) {
		INFO("%s: EXIT ret=%d (qcom_mbox_send failed)\n", __func__,
		     rc);
		return rc;
	}

	g_pending = true;
	g_pending_txn_id = current_txn_id;
	*txn_id = current_txn_id;
	INFO("%s: EXIT ret=0 txn_id:0x%08X framed_size:%zu\n", __func__,
	     current_txn_id, msg_size);
	return 0;
}

int tmecom_recv(uint32_t txn_id, void *rsp, size_t *rsp_size)
{
	size_t recv_size;
	size_t payload_size;
	int rc;

	/* Called on every poll iteration (up to TMECOM_POLL_MAX times per
	 * transaction) - keep entry/in-progress tracing at VERBOSE so a
	 * default build does not flood the log. */
	VERBOSE("%s: ENTRY txn_id:0x%08X\n", __func__, txn_id);

	if ((rsp == NULL) || (rsp_size == NULL)) {
		INFO("%s: EXIT ret=-EINVAL\n", __func__);
		return -EINVAL;
	}
	/*
	 * Refuse to consume a message unless it belongs to the outstanding
	 * transaction; otherwise a stray/unsolicited frame could be accepted
	 * and validated against a stale txn id.
	 */
	if (!g_pending || (txn_id != g_pending_txn_id)) {
		INFO("%s: EXIT ret=-EINVAL (no matching txn; pending:%u id:0x%08X)\n",
		     __func__, (unsigned int)g_pending, g_pending_txn_id);
		return -EINVAL;
	}
	if ((g_chan == NULL) || !g_connected) {
		g_pending = false;
		INFO("%s: EXIT ret=-ENODEV\n", __func__);
		return -ENODEV;
	}

	rc = tmecom_probe_rx();
	if (rc == 0) {
		/* No response yet; keep the transaction outstanding. */
		VERBOSE("%s: EXIT ret=-EINPROGRESS (no response yet)\n",
			__func__);
		return -EINPROGRESS;
	}
	if (rc < 0) {
		g_pending = false;
		INFO("%s: EXIT ret=%d (probe_rx error)\n", __func__, rc);
		return rc;
	}

	/* From here on the transaction is consumed regardless of outcome. */
	g_pending = false;

	recv_size = sizeof(g_recv_buf);
	rc = qcom_mbox_recv(g_chan, g_recv_buf, &recv_size);
	if (rc == -ENOSPC) {
		ERROR("qcom_mbox_recv ENOSPC need:%zu\n", recv_size);
		INFO("%s: EXIT ret=-EIO (framed response exceeds buffer)\n",
		     __func__);
		return -EIO;
	}
	if (rc != 0) {
		ERROR("qcom_mbox_recv err:%d\n", rc);
		INFO("%s: EXIT ret=%d (qcom_mbox_recv failed)\n", __func__,
		     rc);
		return rc;
	}

	rc = tmecom_validate_response(recv_size, txn_id);
	if (rc != 0) {
		INFO("%s: EXIT ret=%d (malformed response, framed_size:%zu)\n",
		     __func__, rc, recv_size);
		return rc;
	}

	payload_size = recv_size - TMECOM_MSG_HDR_SIZE;
	if (payload_size > *rsp_size) {
		/* Report the required capacity so the caller can retry. */
		INFO("%s: EXIT ret=-ENOSPC (need:%zu have:%zu)\n", __func__,
		     payload_size, *rsp_size);
		*rsp_size = payload_size;
		return -ENOSPC;
	}

	(void)memcpy(rsp, g_recv_buf + TMECOM_MSG_HDR_SIZE, payload_size);
	*rsp_size = payload_size;

	INFO("%s: EXIT ret=0 rsp_size:%zu\n", __func__, *rsp_size);
	return 0;
}

int tmecom_send_recv(const void *req, size_t req_size,
		     void *rsp, size_t *rsp_size)
{
	uint32_t txn_id = 0U;
	uint32_t poll;
	int rc;

	INFO("%s: ENTRY req_size:%zu rsp_capacity:%zu\n", __func__, req_size,
	     (rsp_size != NULL) ? *rsp_size : 0U);

	if ((rsp == NULL) || (rsp_size == NULL)) {
		INFO("%s: EXIT ret=-EINVAL\n", __func__);
		return -EINVAL;
	}

	rc = tmecom_send(req, req_size, &txn_id);
	if (rc != 0) {
		INFO("%s: EXIT ret=%d (tmecom_send failed)\n", __func__, rc);
		return rc;
	}

	for (poll = 0U; poll < TMECOM_POLL_MAX; poll++) {
		size_t rsp_capacity = *rsp_size;

		rc = tmecom_recv(txn_id, rsp, &rsp_capacity);
		if (rc != -EINPROGRESS) {
			*rsp_size = rsp_capacity;
			INFO("%s: EXIT ret=%d rsp_size:%zu polls:%u\n",
			     __func__, rc, *rsp_size, poll + 1U);
			return rc;
		}
	}

	/* Timed out waiting for the remote; drop the stale transaction. */
	g_pending = false;
	INFO("%s: EXIT ret=-ETIMEDOUT polls:%u\n", __func__, TMECOM_POLL_MAX);
	return -ETIMEDOUT;
}

bool tmecom_is_connected(void)
{
	uint32_t events;

	INFO("%s: ENTRY\n", __func__);

	if (g_chan == NULL) {
		INFO("%s: EXIT ret=false (no channel)\n", __func__);
		return false;
	}

	events = qcom_mbox_process(g_chan);
	if ((events & (QCOM_MBOX_EVT_DISCONNECTED |
		       QCOM_MBOX_EVT_REMOTE_RESET |
		       QCOM_MBOX_EVT_ERROR)) != 0U) {
		g_connected = false;
	}
	if ((events & QCOM_MBOX_EVT_CONNECTED) != 0U) {
		g_connected = true;
	}
	INFO("%s: EXIT ret=%u events=0x%08X\n", __func__,
	     (unsigned int)g_connected, events);
	return g_connected;
}
