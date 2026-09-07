/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * QMP (Qualcomm Message Protocol) transport.
 *
 * Local endpoint: SCORE (macro/slave).
 * Remote endpoint: MCORE (micro/master).
 *
 * The remote master initializes the shared descriptor and mailbox layout
 * asynchronously.  init() always succeeds; process() discovers the layout
 * when the initialization signature is first observed.
 *
 * Descriptor layout (offsets from desc_base):
 *   [0, QMP_DESC_HEADER_SIZE)  descriptor header
 *   MCORE descriptor at QMP_OFF_MCORE_LINK_STATE
 *   SCORE descriptor at QMP_OFF_SCORE_LINK_STATE
 *   Mailbox regions at master-written offsets within [0, shared_size)
 *
 * Acknowledgment field ownership:
 *   MCORE_LINK_STATE_ACK and MCORE_CH_STATE_ACK are written by the local
 *   endpoint (SCORE).  SCORE_LINK_STATE_ACK and SCORE_CH_STATE_ACK are
 *   written by the remote endpoint (MCORE).
 *
 * MTU equals local_payload_size and becomes valid after layout discovery.
 * qcom_mbox_get_mtu() returns -EAGAIN before MTU is valid.
 *
 * Remote signaling occurs only when a shared-state update was published.
 * Absence of remote initialization is not an error.
 *
 * Mailbox access width: the mailbox RAM is ECC-protected, so it must only ever
 * be WRITTEN in whole 32-bit words - a partially written word is left with an
 * ECC syndrome that disagrees with its data, and the remote takes an
 * uncorrectable fatal when it reads it.  See qcom_mbox_qmp_send().
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/libc/errno.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <drivers/qti/mbox/qcom_mbox.h>
#include "qcom_mbox_private.h"
#include "qcom_mbox_qmp.h"

/* Descriptor field offsets from desc_base. */
#define QMP_OFF_MAGIC			0U
#define QMP_OFF_MCORE_LINK_STATE	12U
#define QMP_OFF_SCORE_LINK_STATE	36U
#define QMP_DESC_HEADER_SIZE		188U

/* Per-endpoint field offsets relative to the endpoint descriptor base. */
#define QMP_EP_LINK_STATE_OFF		0U
#define QMP_EP_LINK_STATE_ACK_OFF	4U
#define QMP_EP_CH_STATE_OFF		8U
#define QMP_EP_CH_STATE_ACK_OFF		12U
#define QMP_EP_MBOX_SIZE_OFF		16U
#define QMP_EP_MBOX_OFFSET_OFF		20U

/* Mailbox layout: msg_len at offset 0, payload at offset 4. */
#define QMP_MBOX_MSG_LEN_OFF		0U
#define QMP_MBOX_MSG_DATA_OFF		4U

#define QMP_LOCAL_EP_OFF	QMP_OFF_SCORE_LINK_STATE
#define QMP_REMOTE_EP_OFF	QMP_OFF_MCORE_LINK_STATE

/* Non-cacheable shared-memory access. */
static inline uint32_t qmp_read32(uintptr_t addr)
{
	return *(volatile uint32_t *)addr;
}

static inline void qmp_write32(uintptr_t addr, uint32_t val)
{
	*(volatile uint32_t *)addr = val;
}

static inline uint8_t qmp_read8(uintptr_t addr)
{
	return *(volatile uint8_t *)addr;
}

/*
 * There is deliberately no qmp_write8().  The mailbox RAM is ECC-protected and
 * a sub-word store leaves the containing word's ECC syndrome inconsistent with
 * its data, which makes the remote take an uncorrectable MBOX_ECC_DED_ERR fatal
 * when it reads that word.  All mailbox writes must go through qmp_write32() -
 * see the comment in qcom_mbox_qmp_send().
 *
 * Byte READS are fine: reads do not recompute a syndrome.
 */

static struct qcom_mbox_qmp_priv *qmp_priv(struct qcom_mbox_chan *chan)
{
	return (struct qcom_mbox_qmp_priv *)chan->cfg->transport_priv;
}

static void qmp_signal_remote(const struct qcom_mbox_qmp_config *cfg)
{
	if (cfg->remote_signal.reg != 0U) {
		mmio_write_32(cfg->remote_signal.reg, cfg->remote_signal.value);
	}
}

/*
 * qmp_validate_layout - validate the master-initialized shared layout.
 *
 * Reads mailbox parameters from the descriptor and validates that all
 * extents are contained within the configured shared area.  Caches
 * addresses and sizes on success.  Returns true on success.
 */
static bool qmp_validate_layout(struct qcom_mbox_chan *chan,
				struct qcom_mbox_qmp_priv *priv)
{
	const struct qcom_mbox_qmp_config *cfg = priv->cfg;
	uintptr_t base = cfg->desc_base;
	uintptr_t local_desc;
	uintptr_t remote_desc;
	uint32_t lsz;
	uint32_t loff;
	uint32_t rsz;
	uint32_t roff;
	uint32_t lpay;
	uint32_t rpay;

	if (cfg->shared_size < QMP_DESC_HEADER_SIZE) {
		return false;
	}

	local_desc  = base + QMP_LOCAL_EP_OFF;
	remote_desc = base + QMP_REMOTE_EP_OFF;

	lsz  = qmp_read32(local_desc  + QMP_EP_MBOX_SIZE_OFF);
	loff = qmp_read32(local_desc  + QMP_EP_MBOX_OFFSET_OFF);
	rsz  = qmp_read32(remote_desc + QMP_EP_MBOX_SIZE_OFF);
	roff = qmp_read32(remote_desc + QMP_EP_MBOX_OFFSET_OFF);

	/* Validate local mailbox extent. */
	if ((lsz < (uint32_t)sizeof(uint32_t)) ||
	    (loff < QMP_DESC_HEADER_SIZE) ||
	    ((loff % (uint32_t)sizeof(uint32_t)) != 0U) ||
	    (loff > cfg->shared_size) ||
	    (lsz > cfg->shared_size - loff)) {
		return false;
	}

	/* Validate remote mailbox extent. */
	if ((rsz < (uint32_t)sizeof(uint32_t)) ||
	    (roff < QMP_DESC_HEADER_SIZE) ||
	    ((roff % (uint32_t)sizeof(uint32_t)) != 0U) ||
	    (roff > cfg->shared_size) ||
	    (rsz > cfg->shared_size - roff)) {
		return false;
	}

	/*
	 * Check no overlap between local and remote mailboxes.
	 * End-address arithmetic is safe: both extents are validated above.
	 */
	if (!((loff + lsz <= roff) || (roff + rsz <= loff))) {
		return false;
	}

	lpay = lsz - (uint32_t)sizeof(uint32_t);
	rpay = rsz - (uint32_t)sizeof(uint32_t);

	if ((lpay == 0U) || (rpay == 0U)) {
		return false;
	}

	priv->local_desc_base     = local_desc;
	priv->remote_desc_base    = remote_desc;
	priv->local_mbox_base     = base + loff;
	priv->remote_mbox_base    = base + roff;
	priv->local_payload_size  = lpay;
	priv->remote_payload_size = rpay;

	chan->mtu = (size_t)lpay;

	return true;
}

/*
 * qmp_ack_remote_state - acknowledge remote link and channel states.
 *
 * Writes acknowledgment only when it differs from the observed state and
 * the observed state is a permitted protocol encoding.
 * Returns true if any acknowledgment was written.
 */
static bool qmp_ack_remote_state(const struct qcom_mbox_qmp_priv *priv)
{
	uint32_t rlink;
	uint32_t rch;
	uint32_t ack;
	bool updated = false;

	rlink = qmp_read32(priv->remote_desc_base + QMP_EP_LINK_STATE_OFF);
	ack   = qmp_read32(priv->remote_desc_base + QMP_EP_LINK_STATE_ACK_OFF);
	if (((rlink == QMP_LINK_DOWN) || (rlink == QMP_LINK_UP)) &&
	    (ack != rlink)) {
		qmp_write32(priv->remote_desc_base + QMP_EP_LINK_STATE_ACK_OFF,
			    rlink);
		updated = true;
	}

	rch = qmp_read32(priv->remote_desc_base + QMP_EP_CH_STATE_OFF);
	ack = qmp_read32(priv->remote_desc_base + QMP_EP_CH_STATE_ACK_OFF);
	if (((rch == QMP_CH_DISCONNECTED) || (rch == QMP_CH_CONNECTED)) &&
	    (ack != rch)) {
		qmp_write32(priv->remote_desc_base + QMP_EP_CH_STATE_ACK_OFF,
			    rch);
		updated = true;
	}

	return updated;
}

/*
 * qmp_handle_link_down - wait for remote layout initialization.
 *
 * Absence of the initialization signature is not an error.
 * A present but malformed layout is a transport error (-EIO).
 */
static int qmp_handle_link_down(struct qcom_mbox_chan *chan,
				struct qcom_mbox_qmp_priv *priv)
{
	const struct qcom_mbox_qmp_config *cfg = priv->cfg;
	bool ack_updated;

	if (qmp_read32(cfg->desc_base + QMP_OFF_MAGIC) != QMP_MAGIC) {
		return 0;
	}

	if (!qmp_validate_layout(chan, priv)) {
		return -EIO;
	}

	priv->layout_valid = true;

	qmp_write32(priv->local_mbox_base + QMP_MBOX_MSG_LEN_OFF, 0U);
	qmp_write32(priv->local_desc_base + QMP_EP_CH_STATE_OFF,
		    QMP_CH_DISCONNECTED);
	/* ch_state must be visible before link_state = UP. */
	dmbst();
	qmp_write32(priv->local_desc_base + QMP_EP_LINK_STATE_OFF, QMP_LINK_UP);

	ack_updated = qmp_ack_remote_state(priv);

	priv->state = QMP_STATE_LINK_NEGOTIATION;

	/* Local state publication always requires notification. */
	dmbst();
	qmp_signal_remote(cfg);
	(void)ack_updated;

	return 0;
}

static int qmp_handle_link_negotiation(struct qcom_mbox_qmp_priv *priv)
{
	const struct qcom_mbox_qmp_config *cfg = priv->cfg;
	uint32_t rlink;
	uint32_t rlink_ack;
	uint32_t llink_ack;
	bool state_updated = false;

	dmbsy();

	rlink     = qmp_read32(priv->remote_desc_base + QMP_EP_LINK_STATE_OFF);
	rlink_ack = qmp_read32(priv->remote_desc_base + QMP_EP_LINK_STATE_ACK_OFF);
	llink_ack = qmp_read32(priv->local_desc_base  + QMP_EP_LINK_STATE_ACK_OFF);

	if ((llink_ack == QMP_LINK_UP) && (rlink == QMP_LINK_UP)) {
		qmp_write32(priv->local_desc_base + QMP_EP_CH_STATE_OFF,
			    QMP_CH_CONNECTED);
		if (rlink_ack != QMP_LINK_UP) {
			/* ch_state must be visible before the ack. */
			dmbsy();
			qmp_write32(priv->remote_desc_base +
				    QMP_EP_LINK_STATE_ACK_OFF, QMP_LINK_UP);
		}
		priv->state = QMP_STATE_LOCAL_CONNECTING;
		state_updated = true;
	} else if ((rlink == QMP_LINK_UP) && (rlink_ack != QMP_LINK_UP)) {
		dmbsy();
		qmp_write32(priv->remote_desc_base +
			    QMP_EP_LINK_STATE_ACK_OFF, QMP_LINK_UP);
		state_updated = true;
	}

	if (state_updated) {
		dmbst();
		qmp_signal_remote(cfg);
	}

	return 0;
}

static int qmp_handle_local_connecting(struct qcom_mbox_qmp_priv *priv,
				       uint32_t *events)
{
	const struct qcom_mbox_qmp_config *cfg = priv->cfg;
	uint32_t rch;
	uint32_t rch_ack;
	uint32_t lch_ack;
	bool state_updated = false;

	dmbsy();

	rch     = qmp_read32(priv->remote_desc_base + QMP_EP_CH_STATE_OFF);
	rch_ack = qmp_read32(priv->remote_desc_base + QMP_EP_CH_STATE_ACK_OFF);
	lch_ack = qmp_read32(priv->local_desc_base  + QMP_EP_CH_STATE_ACK_OFF);

	if ((rch == QMP_CH_CONNECTED) && (rch_ack != QMP_CH_CONNECTED)) {
		dmbsy();
		qmp_write32(priv->remote_desc_base +
			    QMP_EP_CH_STATE_ACK_OFF, QMP_CH_CONNECTED);
		state_updated = true;
	}

	if ((lch_ack == QMP_CH_CONNECTED) && (rch == QMP_CH_CONNECTED)) {
		priv->state = QMP_STATE_E2E_CONNECTED;
		*events |= QCOM_MBOX_EVT_CONNECTED;
	}

	if (state_updated) {
		dmbst();
		qmp_signal_remote(cfg);
	}

	return 0;
}

static int qmp_handle_connected(struct qcom_mbox_chan *chan,
				struct qcom_mbox_qmp_priv *priv,
				uint32_t *events)
{
	uint32_t rlink;
	uint32_t rch;
	uint32_t msg_len;

	dmbsy();

	rlink = qmp_read32(priv->remote_desc_base + QMP_EP_LINK_STATE_OFF);
	rch   = qmp_read32(priv->remote_desc_base + QMP_EP_CH_STATE_OFF);

	if (rlink != QMP_LINK_UP) {
		*events |= QCOM_MBOX_EVT_REMOTE_RESET | QCOM_MBOX_EVT_DISCONNECTED;
		priv->state            = QMP_STATE_LINK_DOWN;
		priv->layout_valid     = false;
		priv->local_desc_base  = 0U;
		priv->remote_desc_base = 0U;
		priv->local_mbox_base  = 0U;
		priv->remote_mbox_base = 0U;
		priv->local_payload_size  = 0U;
		priv->remote_payload_size = 0U;
		chan->mtu = 0U;
		return 0;
	}

	if (rch != QMP_CH_CONNECTED) {
		*events |= QCOM_MBOX_EVT_DISCONNECTED;
		priv->state = QMP_STATE_LINK_NEGOTIATION;
		return 0;
	}

	msg_len = qmp_read32(priv->remote_mbox_base + QMP_MBOX_MSG_LEN_OFF);
	if ((msg_len > 0U) && (msg_len <= priv->remote_payload_size)) {
		*events |= QCOM_MBOX_EVT_RX_READY;
	}

	return 0;
}

static int qcom_mbox_qmp_init(struct qcom_mbox_chan *chan)
{
	struct qcom_mbox_qmp_priv *priv = qmp_priv(chan);
	const struct qcom_mbox_qmp_config *cfg;

	cfg = (const struct qcom_mbox_qmp_config *)chan->cfg->transport_cfg;
	if ((cfg == NULL) || (cfg->shared_size == 0U)) {
		return -EINVAL;
	}

	priv->cfg                 = cfg;
	priv->state               = QMP_STATE_LINK_DOWN;
	priv->layout_valid        = false;
	priv->local_desc_base     = 0U;
	priv->remote_desc_base    = 0U;
	priv->local_mbox_base     = 0U;
	priv->remote_mbox_base    = 0U;
	priv->local_payload_size  = 0U;
	priv->remote_payload_size = 0U;

	return 0;
}

static void qcom_mbox_qmp_deinit(struct qcom_mbox_chan *chan)
{
	struct qcom_mbox_qmp_priv *priv = qmp_priv(chan);
	bool state_updated = false;

	if (priv->cfg == NULL) {
		return;
	}

	if (priv->layout_valid) {
		if (priv->state == QMP_STATE_E2E_CONNECTED) {
			qmp_write32(priv->local_desc_base + QMP_EP_CH_STATE_OFF,
				    QMP_CH_DISCONNECTED);
			/* ch_state must be visible before link_state = DOWN. */
			dmbst();
		}
		qmp_write32(priv->local_desc_base + QMP_EP_LINK_STATE_OFF,
			    QMP_LINK_DOWN);
		dmbsy();
		qmp_write32(priv->remote_desc_base + QMP_EP_LINK_STATE_ACK_OFF,
			    qmp_read32(priv->remote_desc_base +
				       QMP_EP_LINK_STATE_OFF));
		qmp_write32(priv->remote_desc_base + QMP_EP_CH_STATE_ACK_OFF,
			    qmp_read32(priv->remote_desc_base +
				       QMP_EP_CH_STATE_OFF));
		state_updated = true;
	}

	if (state_updated) {
		dmbst();
		qmp_signal_remote(priv->cfg);
	}

	priv->state               = QMP_STATE_LINK_DOWN;
	priv->layout_valid        = false;
	priv->local_desc_base     = 0U;
	priv->remote_desc_base    = 0U;
	priv->local_mbox_base     = 0U;
	priv->remote_mbox_base    = 0U;
	priv->local_payload_size  = 0U;
	priv->remote_payload_size = 0U;
	priv->cfg                 = NULL;
	chan->mtu                 = 0U;
}

static int qcom_mbox_qmp_process(struct qcom_mbox_chan *chan,
				 uint32_t *events)
{
	struct qcom_mbox_qmp_priv *priv = qmp_priv(chan);

	if (priv->cfg == NULL) {
		return -ENODEV;
	}

	switch (priv->state) {
	case QMP_STATE_LINK_DOWN:
		return qmp_handle_link_down(chan, priv);
	case QMP_STATE_LINK_NEGOTIATION:
		return qmp_handle_link_negotiation(priv);
	case QMP_STATE_LOCAL_CONNECTING:
		return qmp_handle_local_connecting(priv, events);
	case QMP_STATE_E2E_CONNECTED:
		return qmp_handle_connected(chan, priv, events);
	default:
		return -EIO;
	}
}

static int qcom_mbox_qmp_send(struct qcom_mbox_chan *chan,
			      const void *buf, size_t len)
{
	struct qcom_mbox_qmp_priv *priv = qmp_priv(chan);
	const uint8_t *src;
	size_t padded_len;
	size_t i;

	if (priv->cfg == NULL) {
		return -ENODEV;
	}
	if (!priv->layout_valid || (priv->state != QMP_STATE_E2E_CONNECTED)) {
		return -ENOTCONN;
	}
	/*
	 * Bound the PADDED length: the word-at-a-time loop below writes up to
	 * three bytes past len when len is not a multiple of 4.
	 */
	padded_len = (len + 3U) & ~((size_t)3U);
	if (padded_len > (size_t)priv->local_payload_size) {
		return -EMSGSIZE;
	}

	/*
	 * The remote clears msg_len after consuming the previous message.
	 * Reject the send if the mailbox is still occupied.
	 */
	if (qmp_read32(priv->local_mbox_base + QMP_MBOX_MSG_LEN_OFF) != 0U) {
		return -EBUSY;
	}

	/*
	 * Write the payload a whole 32-bit word at a time, zero-padding the
	 * tail of the final partial word.
	 *
	 * The mailbox RAM is ECC-protected - the remote reports single- and
	 * double-bit errors on it via TME_ECC_PROT_ERR_STATUS.MBOX_ECC_SEC_ERR
	 * and .MBOX_ECC_DED_ERR.  A sub-word (byte) store cannot produce a
	 * correct ECC syndrome for the word containing it, so a word left only
	 * partially written ends up with a syndrome that disagrees with its
	 * data.  The remote then takes an uncorrectable MBOX_ECC_DED_ERR fatal
	 * the moment it reads that word - it dies in its error-fatal ISR before
	 * the message is ever dispatched, so the request neither gets a response
	 * nor appears in the remote's message log.
	 *
	 * This was originally a byte loop, which happened to work for every
	 * message whose framed length was a multiple of 4 (fuse read: 20 bytes,
	 * SHA digest: 32) and killed the remote on the ones that were not
	 * (fuse write multiple: 786).
	 *
	 * msg_len below still reports the true byte count, so the padding is
	 * never read by the remote and the payload CRC is unaffected.
	 *
	 * local_mbox_base and QMP_MBOX_MSG_DATA_OFF are both 4-byte aligned, so
	 * every store here is naturally aligned.  Bytes are packed little-endian
	 * to match the byte order the previous byte-wise loop produced.
	 */
	src = (const uint8_t *)buf;
	for (i = 0U; i < len; i += 4U) {
		size_t   remaining = len - i;
		size_t   n         = (remaining < 4U) ? remaining : 4U;
		uint32_t word      = 0U;
		size_t   b;

		for (b = 0U; b < n; b++) {
			word |= (uint32_t)src[i + b] << (8U * b);
		}
		qmp_write32(priv->local_mbox_base + QMP_MBOX_MSG_DATA_OFF + i,
			    word);
	}
	/* Payload must be visible before msg_len is published. */
	dmbst();
	qmp_write32(priv->local_mbox_base + QMP_MBOX_MSG_LEN_OFF, (uint32_t)len);
	dmbst();
	qmp_signal_remote(priv->cfg);

	return 0;
}

static int qcom_mbox_qmp_recv(struct qcom_mbox_chan *chan,
			      void *buf, size_t *len)
{
	struct qcom_mbox_qmp_priv *priv = qmp_priv(chan);
	uint32_t msg_len;
	uint8_t *dst;
	size_t i;

	if (priv->cfg == NULL) {
		return -ENODEV;
	}
	if (!priv->layout_valid || (priv->state != QMP_STATE_E2E_CONNECTED)) {
		return -EAGAIN;
	}

	msg_len = qmp_read32(priv->remote_mbox_base + QMP_MBOX_MSG_LEN_OFF);
	if (msg_len == 0U) {
		return -EAGAIN;
	}
	/* Remote wrote payload before msg_len; read payload after observing msg_len. */
	dmbld();

	if (msg_len > priv->remote_payload_size) {
		ERROR("qcom_mbox_qmp: msg_len %u > remote_payload_size %u\n",
		      msg_len, priv->remote_payload_size);
		return -EIO;
	}
	if (msg_len > (uint32_t)chan->mtu) {
		ERROR("qcom_mbox_qmp: msg_len %u > MTU %zu\n",
		      msg_len, chan->mtu);
		return -EIO;
	}
	if ((size_t)msg_len > *len) {
		/* Preserve message; do not return ownership to remote. */
		*len = (size_t)msg_len;
		return -ENOSPC;
	}

	dst = (uint8_t *)buf;
	for (i = 0U; i < (size_t)msg_len; i++) {
		dst[i] = qmp_read8(priv->remote_mbox_base +
				   QMP_MBOX_MSG_DATA_OFF + i);
	}
	/* Payload reads must complete before returning ownership. */
	dmbsy();
	qmp_write32(priv->remote_mbox_base + QMP_MBOX_MSG_LEN_OFF, 0U);

	*len = (size_t)msg_len;
	return 0;
}

static bool qcom_mbox_qmp_rx_pending(struct qcom_mbox_chan *chan)
{
	struct qcom_mbox_qmp_priv *priv = qmp_priv(chan);
	uint32_t msg_len;

	if ((priv->cfg == NULL) || !priv->layout_valid ||
	    (priv->state != QMP_STATE_E2E_CONNECTED)) {
		return false;
	}

	msg_len = qmp_read32(priv->remote_mbox_base + QMP_MBOX_MSG_LEN_OFF);
	return (msg_len > 0U) && (msg_len <= priv->remote_payload_size);
}

const struct qcom_mbox_ops qcom_mbox_qmp_ops = {
	.init       = qcom_mbox_qmp_init,
	.deinit     = qcom_mbox_qmp_deinit,
	.process    = qcom_mbox_qmp_process,
	.send       = qcom_mbox_qmp_send,
	.recv       = qcom_mbox_qmp_recv,
	.rx_pending = qcom_mbox_qmp_rx_pending,
};