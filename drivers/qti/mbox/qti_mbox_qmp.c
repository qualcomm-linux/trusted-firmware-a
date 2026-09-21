// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * QMP (Qualcomm Message Protocol) transport backend.
 *
 * Local endpoint:  SCORE (macro/slave)  — this firmware (EL3)
 * Remote endpoint: MCORE (micro/master) — the remote processor (e.g. TME)
 *
 * The remote master initialises the shared descriptor and mailbox layout
 * asynchronously.  init() always succeeds; process() discovers the layout
 * when the initialisation signature (QMP_MAGIC) is first observed.
 *
 * Shared memory layout (offsets from desc_base)
 * ----------------------------------------------
 *   [0]         magic         (uint32_t) — QMP_MAGIC ("MAIL")
 *   [4]         core_version  (uint32_t) — master protocol version
 *   [8]         core_features (uint32_t) — master feature flags
 *   [12,  36)   MCORE endpoint descriptor (6 x uint32_t)
 *   [36,  60)   SCORE endpoint descriptor (6 x uint32_t)
 *   [60, 188)   reserved / padding
 *   [188, ...)  mailbox regions at master-written offsets
 *
 * Each endpoint descriptor contains (relative offsets):
 *   [+0]  LINK_STATE     — current link state (DOWN or UP)
 *   [+4]  LINK_STATE_ACK — acknowledgment of the remote's link state
 *   [+8]  CH_STATE       — current channel state (DISCONNECTED or CONNECTED)
 *   [+12] CH_STATE_ACK   — acknowledgment of the remote's channel state
 *   [+16] MBOX_SIZE      — size of this endpoint's mailbox region in bytes
 *   [+20] MBOX_OFFSET    — offset of this endpoint's mailbox from desc_base
 *
 * Acknowledgment field ownership
 * --------------------------------
 *   MCORE descriptor: LINK_STATE_ACK and CH_STATE_ACK are written by SCORE.
 *   SCORE descriptor: LINK_STATE_ACK and CH_STATE_ACK are written by MCORE.
 *
 * Mailbox region layout
 * ---------------------
 *   [+0] msg_len  (uint32_t) — 0 = empty; >0 = message present
 *   [+4] payload  (msg_len bytes)
 *
 * Send protocol: write payload → dmbst() → write msg_len → signal remote.
 * Recv protocol: read msg_len → dmbld() → read payload → dmbld() →
 * clear msg_len.
 *
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/qti/mbox/qti_mbox.h>
#include <drivers/qti/mbox/qti_mbox_qmp.h>
#include <lib/libc/errno.h>
#include <lib/mmio.h>

#include "qti_mbox_private.h"

/*
 * QMP protocol constants — used only within this translation unit.
 *
 * QMP_MAGIC: initialisation signature written by the remote master (MCORE)
 *            at offset 0 of the shared descriptor region.
 *            Value: ASCII "MAIL" in little-endian order.
 *
 * QMP_VERSION:  protocol version this driver implements.  The remote master
 *               writes its version at QMP_OFF_CORE_VERSION; the local
 *               endpoint rejects any version other than QMP_VERSION.
 *
 * QMP_FEATURES: bitmask of protocol features this driver supports.  The
 *               remote master writes its feature flags at
 *               QMP_OFF_CORE_FEATURES; the local endpoint rejects any
 *               non-zero feature flags (no optional features are supported).
 *
 * Link-state and channel-state field encodings.  Only these two values are
 * valid for each field; any other value is a protocol error.
 */
#define QMP_MAGIC 0x4D41494CU
#define QMP_VERSION 1U
#define QMP_FEATURES 0x0U

#define QMP_LINK_DOWN 0xFFFF0000U
#define QMP_LINK_UP 0x0000FFFFU

#define QMP_CH_DISCONNECTED 0xFFFF0000U
#define QMP_CH_CONNECTED 0x0000FFFFU

/*
 * Descriptor header layout (offsets from desc_base):
 *
 *   [0]   magic         (uint32_t) — QMP_MAGIC ("MAIL"); written by MCORE.
 *   [4]   core_version  (uint32_t) — master protocol version; must equal
 *                                    QMP_VERSION.
 *   [8]   core_features (uint32_t) — master feature flags; must equal
 *                                    QMP_FEATURES (0 = no optional features).
 *   [12]  MCORE endpoint descriptor (6 x uint32_t = 24 bytes).
 *   [36]  SCORE endpoint descriptor (6 x uint32_t = 24 bytes).
 *   [60]  reserved / padding.
 *   [188] end of descriptor header; mailbox regions start at or after here.
 *
 * QMP_OFF_MAGIC:            magic word offset.
 * QMP_OFF_CORE_VERSION:     master protocol version offset.
 * QMP_OFF_CORE_FEATURES:    master feature flags offset.
 * QMP_OFF_MCORE_LINK_STATE: start of the MCORE (remote) endpoint descriptor.
 * QMP_OFF_SCORE_LINK_STATE: start of the SCORE (local) endpoint descriptor.
 * QMP_DESC_HEADER_SIZE:     total size of the descriptor header; mailbox
 *                           regions must start at or after this offset.
 */
#define QMP_OFF_MAGIC 0U
#define QMP_OFF_CORE_VERSION 4U
#define QMP_OFF_CORE_FEATURES 8U
#define QMP_OFF_MCORE_LINK_STATE 12U
#define QMP_OFF_SCORE_LINK_STATE 36U
#define QMP_DESC_HEADER_SIZE 188U

/*
 * Per-endpoint field offsets relative to the endpoint descriptor base.
 *
 *   [+0]  LINK_STATE     — current link state (DOWN or UP)
 *   [+4]  LINK_STATE_ACK — acknowledgment of the remote's link state
 *   [+8]  CH_STATE       — current channel state (DISCONNECTED or CONNECTED)
 *   [+12] CH_STATE_ACK   — acknowledgment of the remote's channel state
 *   [+16] MBOX_SIZE      — size of this endpoint's mailbox region in bytes
 *   [+20] MBOX_OFFSET    — offset of this endpoint's mailbox from desc_base
 *
 * Acknowledgment field ownership:
 *   MCORE descriptor: LINK_STATE_ACK and CH_STATE_ACK are written by SCORE.
 *   SCORE descriptor: LINK_STATE_ACK and CH_STATE_ACK are written by MCORE.
 */
#define QMP_EP_LINK_STATE_OFF 0U
#define QMP_EP_LINK_STATE_ACK_OFF 4U
#define QMP_EP_CH_STATE_OFF 8U
#define QMP_EP_CH_STATE_ACK_OFF 12U
#define QMP_EP_MBOX_SIZE_OFF 16U
#define QMP_EP_MBOX_OFFSET_OFF 20U

/*
 * Mailbox region layout: msg_len at offset 0, payload at offset 4.
 *
 * Send: write payload → dmbst() → write msg_len → signal remote.
 * Recv: read msg_len → dmbld() → read payload → dmbld() →
 * clear msg_len.
 */
#define QMP_MBOX_MSG_LEN_OFF 0U
#define QMP_MBOX_MSG_DATA_OFF 4U

/* Convenience aliases mapping local/remote roles to descriptor offsets. */
#define QMP_LOCAL_EP_OFF QMP_OFF_SCORE_LINK_STATE
#define QMP_REMOTE_EP_OFF QMP_OFF_MCORE_LINK_STATE

static void qmp_mbox_write(uintptr_t dst, const void *src, size_t len)
{
	const uint8_t *buf = (const uint8_t *)src;
	uint32_t full = (uint32_t)(len >> 2U);
	uint32_t rem = (uint32_t)(len & 3U);
	uint32_t word;
	uint32_t i;

	for (i = 0U; i < full; i++) {
		(void)memcpy(&word, buf, sizeof(word));
		mmio_write_32(dst, word);
		buf += sizeof(uint32_t);
		dst += sizeof(uint32_t);
	}
	if (rem != 0U) {
		/* Zero-pad the partial word to prevent information leakage. */
		word = 0U;
		(void)memcpy(&word, buf, rem);
		mmio_write_32(dst, word);
	}
}

static void qmp_mbox_read(void *dst, uintptr_t src, size_t len)
{
	uint8_t *buf = (uint8_t *)dst;
	uint32_t full = (uint32_t)(len >> 2U);
	uint32_t rem = (uint32_t)(len & 3U);
	uint32_t word;
	uint32_t i;

	for (i = 0U; i < full; i++) {
		word = mmio_read_32(src);
		(void)memcpy(buf, &word, sizeof(word));
		buf += sizeof(uint32_t);
		src += sizeof(uint32_t);
	}
	if (rem != 0U) {
		word = mmio_read_32(src);
		(void)memcpy(buf, &word, rem);
	}
}

static struct qti_mbox_qmp_priv *qmp_priv(struct qti_mbox_chan *chan)
{
	return (struct qti_mbox_qmp_priv *)chan->cfg->transport_priv;
}

/*
 * qmp_signal_remote() - write the remote signal register to notify the remote.
 *
 * Issues a store barrier before the signal write to ensure all preceding
 * shared-memory writes are visible to the remote before it is interrupted.
 * If remote_signal.reg is 0, signaling is disabled (polling-only mode).
 */
static void qmp_signal_remote(const struct qti_mbox_qmp_config *cfg)
{
	/* Publish local state before signaling the remote endpoint. */
	dmbst();

	if (cfg->remote_signal.reg != 0U)
		mmio_write_32(cfg->remote_signal.reg, cfg->remote_signal.value);
}

/*
 * qmp_validate_layout() - validate the master-initialised shared layout.
 *
 * Reads mailbox parameters from the descriptor and validates that all
 * extents are contained within the configured shared area.  Caches
 * addresses and sizes on success.
 *
 * Returns true on success, false if the layout is invalid.
 */
static bool qmp_validate_layout(struct qti_mbox_chan *chan,
				struct qti_mbox_qmp_priv *priv)
{
	const struct qti_mbox_qmp_config *cfg = priv->cfg;
	uintptr_t base = cfg->desc_base;
	uintptr_t local_desc;
	uintptr_t remote_desc;
	uint32_t lsz;
	uint32_t loff;
	uint32_t rsz;
	uint32_t roff;
	uint32_t lpay;
	uint32_t rpay;

	if (cfg->shared_size < QMP_DESC_HEADER_SIZE)
		return false;

	local_desc = base + QMP_LOCAL_EP_OFF;
	remote_desc = base + QMP_REMOTE_EP_OFF;

	lsz = mmio_read_32(local_desc + QMP_EP_MBOX_SIZE_OFF);
	loff = mmio_read_32(local_desc + QMP_EP_MBOX_OFFSET_OFF);
	rsz = mmio_read_32(remote_desc + QMP_EP_MBOX_SIZE_OFF);
	roff = mmio_read_32(remote_desc + QMP_EP_MBOX_OFFSET_OFF);

	/* Validate local mailbox extent. */
	if (lsz < (uint32_t)sizeof(uint32_t) ||
	    loff < QMP_DESC_HEADER_SIZE ||
	    (loff % (uint32_t)sizeof(uint32_t)) != 0U ||
	    loff > cfg->shared_size || lsz > (cfg->shared_size - loff))
		return false;

	/* Validate remote mailbox extent. */
	if (rsz < (uint32_t)sizeof(uint32_t) ||
	    roff < QMP_DESC_HEADER_SIZE ||
	    (roff % (uint32_t)sizeof(uint32_t)) != 0U ||
	    roff > cfg->shared_size || rsz > (cfg->shared_size - roff))
		return false;

	/*
	 * Verify no overlap; both extents are validated above so the
	 * end-address arithmetic cannot overflow.
	 */
	if (loff + lsz > roff && roff + rsz > loff)
		return false;

	lpay = lsz - (uint32_t)sizeof(uint32_t);
	rpay = rsz - (uint32_t)sizeof(uint32_t);

	if (lpay == 0U || rpay == 0U)
		return false;

	priv->local_desc_base = local_desc;
	priv->remote_desc_base = remote_desc;
	priv->local_mbox_base = base + loff;
	priv->remote_mbox_base = base + roff;
	priv->local_payload_size = lpay;
	priv->remote_payload_size = rpay;

	chan->mtu = (size_t)lpay;

	return true;
}

/*
 * qmp_ack_remote_state() - acknowledge remote link and channel states.
 *
 * Writes acknowledgment only when it differs from the observed state and
 * the observed state is a permitted protocol encoding. Invalid state
 * values are silently ignored (not acknowledged) to prevent the local
 * endpoint from acknowledging garbage data.
 *
 * Returns true if any acknowledgment was written.
 */
static bool qmp_ack_remote_state(const struct qti_mbox_qmp_priv *priv)
{
	uint32_t rlink;
	uint32_t rch;
	uint32_t ack;
	bool updated = false;

	rlink = mmio_read_32(priv->remote_desc_base + QMP_EP_LINK_STATE_OFF);
	ack = mmio_read_32(priv->remote_desc_base + QMP_EP_LINK_STATE_ACK_OFF);
	if ((rlink == QMP_LINK_DOWN || rlink == QMP_LINK_UP) &&
	    ack != rlink) {
		mmio_write_32(priv->remote_desc_base +
				      QMP_EP_LINK_STATE_ACK_OFF,
			      rlink);
		updated = true;
	}

	rch = mmio_read_32(priv->remote_desc_base + QMP_EP_CH_STATE_OFF);
	ack = mmio_read_32(priv->remote_desc_base + QMP_EP_CH_STATE_ACK_OFF);
	if ((rch == QMP_CH_DISCONNECTED || rch == QMP_CH_CONNECTED) &&
	    ack != rch) {
		mmio_write_32(priv->remote_desc_base + QMP_EP_CH_STATE_ACK_OFF,
			      rch);
		updated = true;
	}

	return updated;
}

static void qmp_reset_to_link_down(struct qti_mbox_chan *chan,
				   struct qti_mbox_qmp_priv *priv)
{
	priv->state = QMP_STATE_LINK_DOWN;
	priv->layout_valid = false;
	/*
	 * Any pending TX is lost when the link goes down.  TX_DONE will not
	 * be generated for it; the caller must handle the disconnect event.
	 */
	priv->tx_pending = false;
	priv->local_desc_base = 0U;
	priv->remote_desc_base = 0U;
	priv->local_mbox_base = 0U;
	priv->remote_mbox_base = 0U;
	priv->local_payload_size = 0U;
	priv->remote_payload_size = 0U;
	chan->mtu = 0U;
}

/*
 * qmp_link_teardown() - publish disconnect states and reset to LINK_DOWN.
 *
 * Writes CH_STATE=DISCONNECTED (when ch_was_connected), then LINK_STATE=DOWN,
 * acks the remote's last-known states, signals the remote, and resets all
 * priv fields via qmp_reset_to_link_down().
 *
 * Must be called while priv->local_desc_base and priv->remote_desc_base
 * are still valid (i.e., before qmp_reset_to_link_down() zeroes them).
 */
static void qmp_link_teardown(struct qti_mbox_chan *chan,
			      struct qti_mbox_qmp_priv *priv,
			      bool ch_was_connected)
{
	if (ch_was_connected) {
		mmio_write_32(priv->local_desc_base + QMP_EP_CH_STATE_OFF,
			      QMP_CH_DISCONNECTED);
		/* ch_state must be visible before link_state = DOWN. */
		dmbst();
	}
	mmio_write_32(priv->local_desc_base + QMP_EP_LINK_STATE_OFF,
		      QMP_LINK_DOWN);
	/* link_state must be visible before reading remote state for acks. */
	dmbsy();
	mmio_write_32(priv->remote_desc_base + QMP_EP_LINK_STATE_ACK_OFF,
		      mmio_read_32(priv->remote_desc_base +
				   QMP_EP_LINK_STATE_OFF));
	mmio_write_32(priv->remote_desc_base + QMP_EP_CH_STATE_ACK_OFF,
		      mmio_read_32(priv->remote_desc_base +
				   QMP_EP_CH_STATE_OFF));
	qmp_signal_remote(priv->cfg);
	qmp_reset_to_link_down(chan, priv);
}

/*
 * qmp_handle_link_down() - wait for remote layout initialisation.
 *
 * Absence of the initialisation signature is not an error; the remote may
 * not have started yet.  A present but invalid version, unsupported features,
 * or malformed layout is a transport error (-EIO).
 */
static int qmp_handle_link_down(struct qti_mbox_chan *chan,
				struct qti_mbox_qmp_priv *priv)
{
	const struct qti_mbox_qmp_config *cfg = priv->cfg;
	uint32_t version;
	uint32_t features;

	if (mmio_read_32(cfg->desc_base + QMP_OFF_MAGIC) != QMP_MAGIC) {
		/* Remote not yet initialised — not an error. */
		return 0;
	}

	/*
	 * Validate protocol version and feature flags.  These fields are
	 * written by the remote master alongside the magic word.
	 */
	version = mmio_read_32(cfg->desc_base + QMP_OFF_CORE_VERSION);
	features = mmio_read_32(cfg->desc_base + QMP_OFF_CORE_FEATURES);

	if (version != QMP_VERSION) {
		ERROR("qti_mbox_qmp: unsupported version %u (expected %u)\n",
		      version, QMP_VERSION);
		return -EIO;
	}
	if (features != QMP_FEATURES) {
		ERROR("qti_mbox_qmp: unsupported features 0x%08X (exp 0x%08X)\n",
		      features, QMP_FEATURES);
		return -EIO;
	}

	if (!qmp_validate_layout(chan, priv))
		return -EIO;

	priv->layout_valid = true;

	/* Initialise local mailbox and publish LINK_STATE=UP. */
	mmio_write_32(priv->local_mbox_base + QMP_MBOX_MSG_LEN_OFF, 0U);
	mmio_write_32(priv->local_desc_base + QMP_EP_CH_STATE_OFF,
		      QMP_CH_DISCONNECTED);
	/* ch_state must be visible before link_state = UP. */
	dmbst();
	mmio_write_32(priv->local_desc_base + QMP_EP_LINK_STATE_OFF,
		      QMP_LINK_UP);

	(void)qmp_ack_remote_state(priv);

	priv->state = QMP_STATE_LINK_NEGOTIATION;

	qmp_signal_remote(cfg);

	return 0;
}

static int qmp_handle_link_negotiation(struct qti_mbox_chan *chan,
				       struct qti_mbox_qmp_priv *priv)
{
	const struct qti_mbox_qmp_config *cfg = priv->cfg;
	uint32_t rlink;
	uint32_t rlink_ack;
	uint32_t llink_ack;
	bool state_updated = false;

	rlink = mmio_read_32(priv->remote_desc_base + QMP_EP_LINK_STATE_OFF);
	rlink_ack = mmio_read_32(priv->remote_desc_base +
				 QMP_EP_LINK_STATE_ACK_OFF);
	llink_ack =
		mmio_read_32(priv->local_desc_base + QMP_EP_LINK_STATE_ACK_OFF);

	if (rlink != QMP_LINK_UP) {
		qmp_link_teardown(chan, priv, false);
		if (rlink != QMP_LINK_DOWN)
			return -EIO;
		return 0;
	}

	if (llink_ack == QMP_LINK_UP) {
		mmio_write_32(priv->local_desc_base + QMP_EP_CH_STATE_OFF,
			      QMP_CH_CONNECTED);
		if (rlink_ack != QMP_LINK_UP) {
			/* ch_state must be visible before the ack. */
			dmbst();
			mmio_write_32(priv->remote_desc_base +
					      QMP_EP_LINK_STATE_ACK_OFF,
				      QMP_LINK_UP);
		}
		priv->state = QMP_STATE_LOCAL_CONNECTING;
		state_updated = true;
	} else if (rlink_ack != QMP_LINK_UP) {
		mmio_write_32(priv->remote_desc_base +
				      QMP_EP_LINK_STATE_ACK_OFF,
			      QMP_LINK_UP);
		state_updated = true;
	}

	if (state_updated)
		qmp_signal_remote(cfg);

	return 0;
}

static int qmp_handle_local_connecting(struct qti_mbox_chan *chan,
				       struct qti_mbox_qmp_priv *priv,
				       uint32_t *events)
{
	const struct qti_mbox_qmp_config *cfg = priv->cfg;
	uint32_t rlink;
	uint32_t rch;
	uint32_t rch_ack;
	uint32_t lch_ack;
	bool state_updated = false;

	rlink = mmio_read_32(priv->remote_desc_base + QMP_EP_LINK_STATE_OFF);
	rch = mmio_read_32(priv->remote_desc_base + QMP_EP_CH_STATE_OFF);
	rch_ack =
		mmio_read_32(priv->remote_desc_base + QMP_EP_CH_STATE_ACK_OFF);
	lch_ack = mmio_read_32(priv->local_desc_base + QMP_EP_CH_STATE_ACK_OFF);

	if (rlink != QMP_LINK_UP) {
		*events |= QTI_MBOX_EVT_REMOTE_RESET |
			   QTI_MBOX_EVT_DISCONNECTED;
		qmp_link_teardown(chan, priv, true);
		if (rlink != QMP_LINK_DOWN)
			return -EIO;
		return 0;
	}

	if (rch == QMP_CH_CONNECTED && rch_ack != QMP_CH_CONNECTED) {
		/* Observe remote channel state before acknowledging. */
		dmbld();
		mmio_write_32(priv->remote_desc_base + QMP_EP_CH_STATE_ACK_OFF,
			      QMP_CH_CONNECTED);
		state_updated = true;
	}

	if (lch_ack == QMP_CH_CONNECTED && rch == QMP_CH_CONNECTED) {
		priv->state = QMP_STATE_E2E_CONNECTED;
		*events |= QTI_MBOX_EVT_CONNECTED;
	}

	if (state_updated)
		qmp_signal_remote(cfg);

	return 0;
}

static int qmp_handle_connected(struct qti_mbox_chan *chan,
				struct qti_mbox_qmp_priv *priv,
				uint32_t *events)
{
	const struct qti_mbox_qmp_config *cfg = priv->cfg;
	uint32_t rlink;
	uint32_t rch;
	uint32_t msg_len;

	rlink = mmio_read_32(priv->remote_desc_base + QMP_EP_LINK_STATE_OFF);
	rch = mmio_read_32(priv->remote_desc_base + QMP_EP_CH_STATE_OFF);

	if (rlink != QMP_LINK_UP) {
		*events |= QTI_MBOX_EVT_REMOTE_RESET |
			   QTI_MBOX_EVT_DISCONNECTED;
		qmp_link_teardown(chan, priv, true);
		if (rlink != QMP_LINK_DOWN)
			return -EIO;
		return 0;
	}

	if (rch != QMP_CH_CONNECTED) {
		*events |= QTI_MBOX_EVT_DISCONNECTED;
		mmio_write_32(priv->remote_desc_base + QMP_EP_CH_STATE_ACK_OFF,
			      rch);
		mmio_write_32(priv->local_desc_base + QMP_EP_CH_STATE_OFF,
			      QMP_CH_DISCONNECTED);
		/*
		 * Clear any in-flight or stale TX so the remote does not
		 * re-consume it when the channel is reopened.  Any pending TX
		 * is lost; TX_DONE will not be generated for it.
		 */
		mmio_write_32(priv->local_mbox_base + QMP_MBOX_MSG_LEN_OFF,
			      0U);
		priv->tx_pending = false;
		chan->mtu = 0U;
		priv->state = QMP_STATE_LINK_NEGOTIATION;
		qmp_signal_remote(cfg);
		if (rch != QMP_CH_DISCONNECTED)
			return -EIO;
		return 0;
	}

	/*
	 * TX_DONE: detect when the remote has consumed the last sent message
	 * by observing the local msg_len field transition from non-zero to
	 * zero.  tx_pending is set by qti_mbox_qmp_send() and cleared here.
	 */
	if (priv->tx_pending) {
		if (mmio_read_32(priv->local_mbox_base +
				 QMP_MBOX_MSG_LEN_OFF) == 0U) {
			*events |= QTI_MBOX_EVT_TX_DONE;
			priv->tx_pending = false;
		}
	}

	msg_len = mmio_read_32(priv->remote_mbox_base + QMP_MBOX_MSG_LEN_OFF);
	if (msg_len > 0U && msg_len <= priv->remote_payload_size)
		*events |= QTI_MBOX_EVT_RX_READY;

	return 0;
}

/* --------------------------------------------------------------------------
 * Transport operation callbacks
 * --------------------------------------------------------------------------
 */

static int qti_mbox_qmp_init(struct qti_mbox_chan *chan)
{
	struct qti_mbox_qmp_priv *priv = qmp_priv(chan);
	const struct qti_mbox_qmp_config *cfg;

	cfg = (const struct qti_mbox_qmp_config *)chan->cfg->transport_cfg;
	if (!cfg || cfg->desc_base == 0U ||
	    cfg->shared_size == 0U || !priv)
		return -EINVAL;

	priv->cfg = cfg;
	priv->state = QMP_STATE_LINK_DOWN;
	priv->layout_valid = false;
	priv->tx_pending = false;
	priv->local_desc_base = 0U;
	priv->remote_desc_base = 0U;
	priv->local_mbox_base = 0U;
	priv->remote_mbox_base = 0U;
	priv->local_payload_size = 0U;
	priv->remote_payload_size = 0U;
	return 0;
}

static void qti_mbox_qmp_deinit(struct qti_mbox_chan *chan)
{
	struct qti_mbox_qmp_priv *priv = qmp_priv(chan);
	bool ch_connected;

	if (!priv)
		return;

	if (!priv->cfg)
		return;

	if (priv->layout_valid) {
		ch_connected = (priv->state == QMP_STATE_E2E_CONNECTED) ||
			       (priv->state == QMP_STATE_LOCAL_CONNECTING);
		qmp_link_teardown(chan, priv, ch_connected);
	} else {
		qmp_reset_to_link_down(chan, priv);
	}
	priv->cfg = NULL;
}

static int qti_mbox_qmp_process(struct qti_mbox_chan *chan, uint32_t *events)
{
	struct qti_mbox_qmp_priv *priv = qmp_priv(chan);

	if (!priv->cfg)
		return -ENODEV;

	/*
	 * Full system barrier before observing remote state.  This ensures
	 * all previous local writes are visible to the remote and that any
	 * pending load results from previous iterations are committed before
	 * the new observation window begins.  See file header for rationale.
	 */
	dmbsy();

	switch (priv->state) {
	case QMP_STATE_LINK_DOWN:
		return qmp_handle_link_down(chan, priv);
	case QMP_STATE_LINK_NEGOTIATION:
		return qmp_handle_link_negotiation(chan, priv);
	case QMP_STATE_LOCAL_CONNECTING:
		return qmp_handle_local_connecting(chan, priv, events);
	case QMP_STATE_E2E_CONNECTED:
		return qmp_handle_connected(chan, priv, events);
	default:
		return -EIO;
	}
}

static int qti_mbox_qmp_send(struct qti_mbox_chan *chan, const void *buf,
			     size_t len)
{
	struct qti_mbox_qmp_priv *priv = qmp_priv(chan);

	if (!priv->cfg)
		return -ENODEV;

	if (priv->state != QMP_STATE_E2E_CONNECTED)
		return -ENOTCONN;

	if (len > (size_t)priv->local_payload_size)
		return -EMSGSIZE;

	/*
	 * The remote clears msg_len after consuming the previous message.
	 * Reject the send if the mailbox is still occupied.
	 */
	if (mmio_read_32(priv->local_mbox_base + QMP_MBOX_MSG_LEN_OFF) != 0U)
		return -EBUSY;

	qmp_mbox_write(priv->local_mbox_base + QMP_MBOX_MSG_DATA_OFF, buf, len);

	/* Payload must be visible before msg_len is published. */
	dmbst();
	mmio_write_32(priv->local_mbox_base + QMP_MBOX_MSG_LEN_OFF,
		      (uint32_t)len);

	/*
	 * Mark TX as pending so qmp_handle_connected() can detect when the
	 * remote clears msg_len and generate QTI_MBOX_EVT_TX_DONE.
	 */
	priv->tx_pending = true;

	qmp_signal_remote(priv->cfg);

	return 0;
}

static int qti_mbox_qmp_recv(struct qti_mbox_chan *chan, void *buf, size_t *len)
{
	struct qti_mbox_qmp_priv *priv = qmp_priv(chan);
	uint32_t msg_len;

	if (!priv->cfg)
		return -ENODEV;

	if (priv->state != QMP_STATE_E2E_CONNECTED)
		return -EAGAIN;

	msg_len = mmio_read_32(priv->remote_mbox_base + QMP_MBOX_MSG_LEN_OFF);
	if (msg_len == 0U)
		return -EAGAIN;

	/* Remote wrote payload before msg_len; observe msg_len before
	 * payload.
	 */
	dmbld();

	if (msg_len > priv->remote_payload_size) {
		ERROR("qti_mbox_qmp: msg_len %u exceeds payload_size %u\n",
		      msg_len, priv->remote_payload_size);
		return -EIO;
	}
	if ((size_t)msg_len > *len) {
		/*
		 * Buffer too small.  Preserve the message (do not clear
		 * msg_len) so the caller can retry with a larger buffer.
		 */
		*len = (size_t)msg_len;
		return -ENOSPC;
	}

	qmp_mbox_read(buf, priv->remote_mbox_base + QMP_MBOX_MSG_DATA_OFF,
		      (size_t)msg_len);

	/* Complete all payload reads before returning ownership to remote. */
	dmbld();
	mmio_write_32(priv->remote_mbox_base + QMP_MBOX_MSG_LEN_OFF, 0U);

	*len = (size_t)msg_len;
	return 0;
}

static bool qti_mbox_qmp_rx_pending(struct qti_mbox_chan *chan)
{
	struct qti_mbox_qmp_priv *priv = qmp_priv(chan);
	uint32_t msg_len;

	if (!priv->cfg || priv->state != QMP_STATE_E2E_CONNECTED)
		return false;

	msg_len = mmio_read_32(priv->remote_mbox_base + QMP_MBOX_MSG_LEN_OFF);
	return msg_len > 0U && msg_len <= priv->remote_payload_size;
}

const struct qti_mbox_ops qti_mbox_qmp_ops = {
	.init = qti_mbox_qmp_init,
	.deinit = qti_mbox_qmp_deinit,
	.process = qti_mbox_qmp_process,
	.send = qti_mbox_qmp_send,
	.recv = qti_mbox_qmp_recv,
	.rx_pending = qti_mbox_qmp_rx_pending,
};
