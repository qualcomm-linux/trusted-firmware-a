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
 * Each message is prefixed with an 8-byte header (version, CRC, txn_id).
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

/* Cap on how many received bytes get dumped for diagnostics. */
#define TMECOM_DUMP_MAX_BYTES		32U

static struct qcom_mbox_chan	*g_chan;
static bool			 g_connected;
static size_t			 g_mtu;

/*
 * At most one transaction may be outstanding at a time (the remote does not
 * queue requests).  g_pending is set by tmecom_send() and cleared by
 * tmecom_recv() on any terminal outcome; g_pending_txn_id identifies it.
 */
static bool			 g_pending;
static uint32_t		 g_pending_txn_id;
static uint32_t		 g_next_txn_id = TMECOM_INITIAL_TXN_ID;

/*
 * Scratch buffer used to assemble [tmecom_msg_hdr][payload] before handing
 * it to qcom_mbox_send() - static, not on the stack, per the same rationale
 * as the tmecomMsgReq_t/tmecomMsgRsp_t buffers in TmeMessage.c (BL31's
 * per-CPU stack is too small for a ~2KB frame).
 */
static uint8_t			 g_tx_frame[TMECOM_MAX_MSG_SIZE];

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
	 * the connected path above did not run.
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
	g_pending = false;
}

int tmecom_send(const void *req, size_t req_size)
{
	struct tmecom_msg_hdr hdr;
	int rc;

	INFO("%s: ENTRY req_size:%zu\n", __func__, req_size);

	if ((req == NULL) || (req_size == 0U)) {
		INFO("%s: EXIT ret=-EINVAL\n", __func__);
		return -EINVAL;
	}
	if (req_size > TMECOM_MAX_PAYLOAD_SIZE) {
		INFO("%s: EXIT ret=-EMSGSIZE (req_size:%zu > max:%zu)\n",
		     __func__, req_size, (size_t)TMECOM_MAX_PAYLOAD_SIZE);
		return -EMSGSIZE;
	}
	if ((g_chan == NULL) || !g_connected) {
		INFO("%s: EXIT ret=-ENODEV\n", __func__);
		return -ENODEV;
	}
	if (g_pending) {
		INFO("%s: EXIT ret=-EAGAIN (txn already pending)\n", __func__);
		return -EAGAIN;
	}

	/*
	 * Frame the request: [tmecom_msg_hdr][payload].  The wire protocol
	 * requires this header on every message (see the file header comment
	 * above) - the remote parses it, validates the CRC (over the payload
	 * only) and echoes txn_id back in its response header.
	 */
	hdr.version = TMECOM_WIRE_VERSION;
	hdr.crc     = tmeCalculateCRC16(req, req_size);
	hdr.txn_id  = g_next_txn_id;

	memcpy(g_tx_frame, &hdr, TMECOM_MSG_HDR_SIZE);
	memcpy(g_tx_frame + TMECOM_MSG_HDR_SIZE, req, req_size);

	rc = qcom_mbox_send(g_chan, g_tx_frame, TMECOM_MSG_HDR_SIZE + req_size);
	if (rc != 0) {
		INFO("%s: EXIT ret=%d (qcom_mbox_send failed)\n", __func__,
		     rc);
		return rc;
	}

	g_pending_txn_id = g_next_txn_id;
	g_next_txn_id++;
	g_pending = true;
	INFO("%s: EXIT ret=0 txn_id:0x%08X\n", __func__, g_pending_txn_id);
	return 0;
}

int tmecom_recv(void *rsp, size_t *rsp_size)
{
	struct tmecom_msg_hdr hdr;
	size_t                payload_size;
	int                   rc;

	/* Called on every poll iteration (up to TMECOM_POLL_MAX times per
	 * transaction) - keep entry/in-progress tracing at VERBOSE so a
	 * default build does not flood the log. */
	VERBOSE("%s: ENTRY\n", __func__);

	if ((rsp == NULL) || (rsp_size == NULL)) {
		INFO("%s: EXIT ret=-EINVAL\n", __func__);
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

	rc = qcom_mbox_recv(g_chan, rsp, rsp_size);
	if (rc == -ENOSPC) {
		ERROR("qcom_mbox_recv ENOSPC need:%zu\n", *rsp_size);
		INFO("%s: EXIT ret=-EIO (response too large)\n", __func__);
		return -EIO;
	}
	if (rc != 0) {
		ERROR("qcom_mbox_recv err:%d\n", rc);
		INFO("%s: EXIT ret=%d (qcom_mbox_recv failed)\n", __func__,
		     rc);
		return rc;
	}

	/*
	 * Strip and validate the [tmecom_msg_hdr][payload] framing the
	 * remote prepends to every response (see the file header comment).
	 */
	if (*rsp_size < TMECOM_MSG_HDR_SIZE) {
		ERROR("tmecom_recv short frame:%zu\n", *rsp_size);
		INFO("%s: EXIT ret=-EIO (frame shorter than header)\n",
		     __func__);
		return -EIO;
	}

	memcpy(&hdr, rsp, TMECOM_MSG_HDR_SIZE);
	payload_size = *rsp_size - TMECOM_MSG_HDR_SIZE;

	if (hdr.version != TMECOM_WIRE_VERSION) {
		ERROR("tmecom_recv bad version:0x%04X\n",
		      hdr.version);
		INFO("%s: EXIT ret=-EIO (version mismatch)\n", __func__);
		return -EIO;
	}
	if (hdr.txn_id != g_pending_txn_id) {
		ERROR("tmecom_recv txn_id got:0x%08X want:0x%08X\n",
		      hdr.txn_id, g_pending_txn_id);
		INFO("%s: EXIT ret=-EIO (txn_id mismatch)\n", __func__);
		return -EIO;
	}
	if (!tmeDoesCRC16Match(hdr.crc, (const uint8_t *)rsp + TMECOM_MSG_HDR_SIZE,
			       payload_size)) {
		ERROR("tmecom_recv CRC mismatch\n");
		INFO("%s: EXIT ret=-EIO (CRC mismatch)\n", __func__);
		return -EIO;
	}

	memmove(rsp, (const uint8_t *)rsp + TMECOM_MSG_HDR_SIZE, payload_size);
	*rsp_size = payload_size;

	INFO("%s: EXIT ret=0 rsp_size:%zu\n", __func__, *rsp_size);
	return 0;
}

int tmecom_send_recv(const void *req, size_t req_size,
		     void *rsp, size_t *rsp_size)
{
	uint32_t poll;
	int rc;

	INFO("%s: ENTRY req_size:%zu rsp_capacity:%zu\n", __func__, req_size,
	     (rsp_size != NULL) ? *rsp_size : 0U);

	if ((rsp == NULL) || (rsp_size == NULL)) {
		INFO("%s: EXIT ret=-EINVAL\n", __func__);
		return -EINVAL;
	}

	rc = tmecom_send(req, req_size);
	if (rc != 0) {
		INFO("%s: EXIT ret=%d (tmecom_send failed)\n", __func__, rc);
		return rc;
	}

	for (poll = 0U; poll < TMECOM_POLL_MAX; poll++) {
		size_t rsp_capacity = *rsp_size;

		rc = tmecom_recv(rsp, &rsp_capacity);
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
