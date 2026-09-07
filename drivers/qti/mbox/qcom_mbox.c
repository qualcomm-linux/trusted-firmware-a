/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <lib/libc/errno.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>

#include <drivers/qti/mbox/qcom_mbox.h>
#include "qcom_mbox_private.h"
#include "qcom_mbox_plat.h"

spinlock_t g_slot_lock;

static int plat_data_validate(const struct qcom_mbox_plat_data *pd)
{
	if ((pd == NULL) || (pd->num_channels == 0U) ||
	    (pd->configs == NULL) || (pd->slots == NULL)) {
		return -EINVAL;
	}
	return 0;
}

static int config_validate(const struct qcom_mbox_chan_config *cfg)
{
	if ((cfg->name == NULL) || (cfg->name[0] == '\0') ||
	    (cfg->ops == NULL)) {
		return -EINVAL;
	}
	if ((cfg->ops->init == NULL) || (cfg->ops->deinit == NULL) ||
	    (cfg->ops->process == NULL) || (cfg->ops->send == NULL) ||
	    (cfg->ops->recv == NULL) || (cfg->ops->rx_pending == NULL)) {
		return -EINVAL;
	}
	return 0;
}

static int config_find(const struct qcom_mbox_plat_data *pd,
		       const char *name, size_t *idx)
{
	size_t i;

	for (i = 0U; i < pd->num_channels; i++) {
		if (strcmp(pd->configs[i].name, name) == 0) {
			*idx = i;
			return 0;
		}
	}
	return -ENOENT;
}

int qcom_mbox_request(const char *name, struct qcom_mbox_chan **chan)
{
	const struct qcom_mbox_plat_data *pd;
	const struct qcom_mbox_chan_config *cfg;
	struct qcom_mbox_chan_slot *slot;
	struct qcom_mbox_chan *ch;
	size_t idx;
	int rc;

	if (chan == NULL) {
		return -EINVAL;
	}
	*chan = NULL;

	if ((name == NULL) || (name[0] == '\0')) {
		return -EINVAL;
	}

	pd = plat_qcom_mbox_get_data();
	if (plat_data_validate(pd) != 0) {
		return -ENODEV;
	}

	if (config_find(pd, name, &idx) != 0) {
		return -ENOENT;
	}

	cfg = &pd->configs[idx];
	if (config_validate(cfg) != 0) {
		return -EINVAL;
	}

	slot = &pd->slots[idx];

	spin_lock(&g_slot_lock);
	if (slot->in_use) {
		spin_unlock(&g_slot_lock);
		return -EBUSY;
	}
	slot->in_use = true;
	spin_unlock(&g_slot_lock);

	ch = &slot->chan;
	(void)memset(ch, 0, sizeof(*ch));
	ch->cfg = cfg;

	rc = cfg->ops->init(ch);
	if (rc != 0) {
		cfg->ops->deinit(ch);
		(void)memset(ch, 0, sizeof(*ch));
		spin_lock(&g_slot_lock);
		slot->in_use = false;
		spin_unlock(&g_slot_lock);
		return rc;
	}

	ch->ready = true;
	*chan = ch;
	return 0;
}

void qcom_mbox_release(struct qcom_mbox_chan *chan)
{
	const struct qcom_mbox_plat_data *pd;
	struct qcom_mbox_chan_slot *slot;
	size_t i;

	if (chan == NULL) {
		return;
	}

	pd = plat_qcom_mbox_get_data();
	if ((pd == NULL) || (pd->slots == NULL)) {
		return;
	}

	slot = NULL;
	for (i = 0U; i < pd->num_channels; i++) {
		if (&pd->slots[i].chan == chan) {
			slot = &pd->slots[i];
			break;
		}
	}
	if ((slot == NULL) || !slot->in_use) {
		return;
	}

	chan->ready = false;
	chan->cfg->ops->deinit(chan);

	spin_lock(&g_slot_lock);
	(void)memset(&slot->chan, 0, sizeof(slot->chan));
	slot->in_use = false;
	spin_unlock(&g_slot_lock);
}

uint32_t qcom_mbox_process(struct qcom_mbox_chan *chan)
{
	uint32_t events = 0U;
	uint32_t result;
	int rc;

	if (!qcom_mbox_chan_is_ready(chan)) {
		return 0U;
	}

	rc = chan->cfg->ops->process(chan, &events);
	if ((rc != 0) || ((events & QCOM_MBOX_EVT_ERROR) != 0U)) {
		chan->sticky_events |= QCOM_MBOX_EVT_ERROR;
	}

	chan->pending_events |= events & ~QCOM_MBOX_EVT_ERROR;

	if (chan->cfg->ops->rx_pending(chan)) {
		chan->pending_events |= QCOM_MBOX_EVT_RX_READY;
	} else {
		chan->pending_events &= ~QCOM_MBOX_EVT_RX_READY;
	}

	result = chan->pending_events | chan->sticky_events;
	chan->pending_events &= QCOM_MBOX_EVT_RX_READY;

	return result;
}

int qcom_mbox_send(struct qcom_mbox_chan *chan, const void *buf, size_t len)
{
	if ((chan == NULL) || (buf == NULL) || (len == 0U)) {
		return -EINVAL;
	}
	if (!qcom_mbox_chan_is_ready(chan)) {
		return -EINVAL;
	}
	if ((chan->mtu > 0U) && (len > chan->mtu)) {
		return -EMSGSIZE;
	}
	return chan->cfg->ops->send(chan, buf, len);
}

int qcom_mbox_recv(struct qcom_mbox_chan *chan, void *buf, size_t *len)
{
	if ((chan == NULL) || (buf == NULL) || (len == NULL)) {
		return -EINVAL;
	}
	if (!qcom_mbox_chan_is_ready(chan)) {
		return -EINVAL;
	}
	return chan->cfg->ops->recv(chan, buf, len);
}

int qcom_mbox_get_mtu(struct qcom_mbox_chan *chan, size_t *mtu)
{
	if ((chan == NULL) || (mtu == NULL)) {
		return -EINVAL;
	}
	if (!qcom_mbox_chan_is_ready(chan)) {
		return -EINVAL;
	}
	if (chan->mtu == 0U) {
		return -EAGAIN;
	}
	*mtu = chan->mtu;
	return 0;
}