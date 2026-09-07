/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCOM_MBOX_PRIVATE_H
#define QCOM_MBOX_PRIVATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/spinlock.h>

#include <drivers/qti/mbox/qcom_mbox.h>

struct qcom_mbox_ops {
	int  (*init)(struct qcom_mbox_chan *chan);
	void (*deinit)(struct qcom_mbox_chan *chan);
	int  (*process)(struct qcom_mbox_chan *chan, uint32_t *events);
	int  (*send)(struct qcom_mbox_chan *chan, const void *buf, size_t len);
	int  (*recv)(struct qcom_mbox_chan *chan, void *buf, size_t *len);
	bool (*rx_pending)(struct qcom_mbox_chan *chan);
};

/*
 * Immutable channel configuration supplied by platform code.
 * transport_priv points to platform-allocated mutable transport state.
 */
struct qcom_mbox_chan_config {
	const char			*name;
	const struct qcom_mbox_ops	*ops;
	const void			*transport_cfg;
	void				*transport_priv;
};

struct qcom_mbox_chan {
	const struct qcom_mbox_chan_config	*cfg;
	bool					 ready;
	size_t					 mtu;
	uint32_t				 pending_events;
	uint32_t				 sticky_events;
};

/* Configuration entry N maps permanently to slot N. */
struct qcom_mbox_chan_slot {
	struct qcom_mbox_chan	chan;
	bool			in_use;
};

extern spinlock_t g_slot_lock;

static inline bool qcom_mbox_chan_is_ready(const struct qcom_mbox_chan *chan)
{
	return (chan != NULL) && chan->ready;
}

#endif /* QCOM_MBOX_PRIVATE_H */