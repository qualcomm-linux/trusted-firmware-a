// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * Qualcomm mailbox framework core.
 *
 * Provides transport-agnostic channel lifecycle management (request/release),
 * state machine advancement (process), and message transfer (send/recv).
 *
 * All functions are non-blocking and EL3-safe.  No dynamic memory allocation
 * is performed; all channel state is embedded in statically-allocated slot
 * arrays supplied by the platform.
 *
 * Concurrency
 * -----------
 * mbox_slot_lock protects slot->in_use transitions only.  Channel operations
 * (process, send, recv) are not protected by any lock.  Concurrent access to
 * the same channel handle from multiple CPUs is not supported; the caller is
 * responsible for serialisation.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <arch_helpers.h>
#include <drivers/qti/mbox/qti_mbox.h>
#include <drivers/qti/mbox/qti_mbox_plat.h>
#include <lib/libc/errno.h>
#include <lib/spinlock.h>

#include "qti_mbox_private.h"

/* Protects in_use transitions for all channel slots. */
static spinlock_t mbox_slot_lock;

/* Platform data cached by qti_mbox_init(); NULL until initialised. */
static const struct qti_mbox_plat_data *g_plat_data;

static int plat_data_validate(const struct qti_mbox_plat_data *pd)
{
	if (!pd || pd->num_channels == 0U || !pd->configs || !pd->slots)
		return -EINVAL;

	return 0;
}

static int config_validate(const struct qti_mbox_chan_config *cfg)
{
	if (!cfg->name || cfg->name[0] == '\0' || !cfg->ops)
		return -EINVAL;

	if (!cfg->ops->init || !cfg->ops->deinit || !cfg->ops->process ||
	    !cfg->ops->send || !cfg->ops->recv || !cfg->ops->rx_pending)
		return -EINVAL;

	return 0;
}

static int config_find(const struct qti_mbox_plat_data *pd, const char *name,
		       size_t *idx)
{
	size_t i;

	for (i = 0U; i < pd->num_channels; i++) {
		if (pd->configs[i].name &&
		    strcmp(pd->configs[i].name, name) == 0) {
			*idx = i;
			return 0;
		}
	}
	return -ENOENT;
}

int qti_mbox_init(void)
{
	return qti_mbox_plat_init(&g_plat_data);
}

int qti_mbox_request(const char *name, struct qti_mbox_chan **chan)
{
	const struct qti_mbox_plat_data *pd;
	const struct qti_mbox_chan_config *cfg;
	struct qti_mbox_chan_slot *slot;
	struct qti_mbox_chan *ch;
	size_t idx;
	int rc;

	if (!chan)
		return -EINVAL;

	*chan = NULL;

	if (!name || name[0] == '\0')
		return -EINVAL;

	pd = g_plat_data;
	if (plat_data_validate(pd) != 0)
		return -ENODEV;

	if (config_find(pd, name, &idx) != 0)
		return -ENOENT;

	cfg = &pd->configs[idx];
	if (config_validate(cfg) != 0)
		return -EINVAL;

	slot = &pd->slots[idx];

	spin_lock(&mbox_slot_lock);
	if (slot->in_use) {
		spin_unlock(&mbox_slot_lock);
		return -EBUSY;
	}
	slot->in_use = true;
	spin_unlock(&mbox_slot_lock);

	ch = &slot->chan;
	(void)memset(ch, 0, sizeof(*ch));
	ch->cfg = cfg;

	rc = cfg->ops->init(ch);
	if (rc != 0) {
		cfg->ops->deinit(ch);
		(void)memset(ch, 0, sizeof(*ch));
		spin_lock(&mbox_slot_lock);
		slot->in_use = false;
		spin_unlock(&mbox_slot_lock);
		return rc;
	}

	ch->ready = true;
	*chan = ch;
	return 0;
}

void qti_mbox_release(struct qti_mbox_chan *chan)
{
	const struct qti_mbox_plat_data *pd;
	struct qti_mbox_chan_slot *slot;
	size_t i;

	if (!chan)
		return;

	pd = g_plat_data;
	if (!pd || !pd->slots)
		return;

	slot = NULL;
	for (i = 0U; i < pd->num_channels; i++) {
		if (&pd->slots[i].chan == chan) {
			slot = &pd->slots[i];
			break;
		}
	}

	spin_lock(&mbox_slot_lock);
	if (!slot || !slot->in_use || !chan->ready) {
		spin_unlock(&mbox_slot_lock);
		return;
	}
	chan->ready = false;
	spin_unlock(&mbox_slot_lock);

	chan->cfg->ops->deinit(chan);
	(void)memset(&slot->chan, 0, sizeof(slot->chan));

	spin_lock(&mbox_slot_lock);
	slot->in_use = false;
	spin_unlock(&mbox_slot_lock);
}

int qti_mbox_process(struct qti_mbox_chan *chan, uint32_t *events)
{
	uint32_t ev = 0U;
	int rc;

	if (!chan || !events)
		return -EINVAL;

	*events = 0U;

	if (!qti_mbox_chan_is_ready(chan))
		/* Channel not yet ready — not an error, just no events. */
		return 0;

	rc = chan->cfg->ops->process(chan, &ev);
	if (rc != 0 || (ev & QTI_MBOX_EVT_ERROR) != 0U)
		chan->sticky_events |= QTI_MBOX_EVT_ERROR;

	/* Accumulate non-error edge events; ERROR is handled as sticky. */
	chan->pending_events |= ev & ~QTI_MBOX_EVT_ERROR;

	/* Refresh the RX_READY level event from the transport. */
	if (chan->cfg->ops->rx_pending(chan))
		chan->pending_events |= QTI_MBOX_EVT_RX_READY;
	else
		chan->pending_events &= ~QTI_MBOX_EVT_RX_READY;

	*events = chan->pending_events | chan->sticky_events;

	/* Clear edge events; retain RX_READY (level) for the next call. */
	chan->pending_events &= QTI_MBOX_EVT_RX_READY;

	return 0;
}

int qti_mbox_send(struct qti_mbox_chan *chan, const void *buf, size_t len)
{
	if (!chan || !buf || len == 0U)
		return -EINVAL;

	if (!qti_mbox_chan_is_ready(chan) || chan->mtu == 0U)
		return -ENODEV;

	if (len > chan->mtu)
		return -EMSGSIZE;

	return chan->cfg->ops->send(chan, buf, len);
}

int qti_mbox_recv(struct qti_mbox_chan *chan, void *buf, size_t *len)
{
	if (!chan || !buf || !len)
		return -EINVAL;

	if (!qti_mbox_chan_is_ready(chan))
		return -ENODEV;

	return chan->cfg->ops->recv(chan, buf, len);
}

int qti_mbox_get_mtu(struct qti_mbox_chan *chan, size_t *mtu)
{
	if (!chan || !mtu)
		return -EINVAL;

	if (!qti_mbox_chan_is_ready(chan))
		return -EINVAL;

	if (chan->mtu == 0U) {
		/*
		 * MTU is not yet valid.  The transport sets chan->mtu when
		 * the shared layout is validated. The caller should poll
		 * with qti_mbox_process() and retry after observing
		 * QTI_MBOX_EVT_CONNECTED.
		 */
		return -EAGAIN;
	}
	*mtu = chan->mtu;
	return 0;
}
