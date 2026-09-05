/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef QTI_MBOX_PRIVATE_H
#define QTI_MBOX_PRIVATE_H

/*
 * Driver-internal header for the Qualcomm mailbox framework.
 *
 * This header is included only by the framework core (qti_mbox.c) and
 * transport backends (e.g. qti_mbox_qmp.c).  Platform code must not
 * include this header directly.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/mbox/qti_mbox_types.h>

/*
 * Transport operations table.
 *
 * Every transport backend must populate all function pointers; NULL entries
 * are rejected by config_validate() in qti_mbox.c.
 *
 * init:       one-time channel setup; called by qti_mbox_request().
 *             May return non-zero on failure; deinit() will be called to
 *             clean up any partial initialisation.
 * deinit:     tear down the channel; called by qti_mbox_release() and on
 *             init() failure.  Must not fail.
 * process:    advance the state machine; set event bits in *events.
 *             Called by qti_mbox_process().
 * send:       transmit a message; len <= chan->mtu.
 *             Called by qti_mbox_send().
 * recv:       receive a message; updates *len on success.
 *             Called by qti_mbox_recv().
 * rx_pending: return true if at least one message is waiting.
 *             Called by qti_mbox_process() to refresh the RX_READY level
 *             event.
 */
struct qti_mbox_ops {
	int (*init)(struct qti_mbox_chan *chan);
	void (*deinit)(struct qti_mbox_chan *chan);
	int (*process)(struct qti_mbox_chan *chan, uint32_t *events);
	int (*send)(struct qti_mbox_chan *chan, const void *buf, size_t len);
	int (*recv)(struct qti_mbox_chan *chan, void *buf, size_t *len);
	bool (*rx_pending)(struct qti_mbox_chan *chan);
};

/*
 * qti_mbox_chan_is_ready() - test whether a channel handle is usable.
 *
 * Returns true only when chan is non-NULL and has been successfully
 * initialised via qti_mbox_request().  Used internally by the framework
 * core to guard all channel operations.
 */
static inline bool qti_mbox_chan_is_ready(const struct qti_mbox_chan *chan)
{
	return chan && chan->ready;
}

#endif /* QTI_MBOX_PRIVATE_H */
