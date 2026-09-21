/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef QTI_MBOX_H
#define QTI_MBOX_H

/*
 * Qualcomm mailbox framework - public API.
 *
 * This header is the only interface that consumers of the mailbox framework
 * should include.  All other headers in this directory are either
 * platform-integration headers (qti_mbox_plat.h, qti_mbox_qmp.h) or
 * driver-internal headers (qti_mbox_private.h).
 *
 * Concurrency model
 * -----------------
 * Concurrent access to the same channel handle from multiple CPUs is NOT
 * supported.  The caller is responsible for serializing all operations on a
 * given channel handle.  qti_mbox_request() and qti_mbox_release() are
 * protected by an internal spinlock for slot allocation only; channel
 * operations (process, send, recv) are not protected.
 *
 * EL3 usage
 * ---------
 * All framework functions are non-blocking and return immediately.  No
 * polling loops, no waits, and no dynamic memory allocation occur inside
 * the framework.  The caller is responsible for implementing any required
 * polling or timeout logic.
 */

#include <stddef.h>
#include <stdint.h>

#include <lib/utils_def.h>

/*
 * Event bitmap returned by qti_mbox_process().
 *
 * Edge events (latched, returned once per transition):
 *   CONNECTED    - end-to-end connection established; MTU is now valid.
 *   DISCONNECTED - remote endpoint disconnected or channel closed.
 *   TX_DONE      - remote endpoint consumed the last sent message; the
 *                  local mailbox is now free for the next send.
 *   REMOTE_RESET - remote endpoint reset detected (link went down).
 *
 * Level event (reflects current state on every call):
 *   RX_READY     - at least one received message is waiting in the mailbox.
 *
 * Sticky event (set on first error; remains set until qti_mbox_release()):
 *   ERROR        - unrecoverable transport failure; channel must be released.
 *
 * Reconnection model
 * ------------------
 * DISCONNECTED and REMOTE_RESET do NOT require the client to call
 * qti_mbox_release().  The transport automatically re-negotiates:
 *
 *   REMOTE_RESET: transport resets to LINK_DOWN and waits for the remote
 *     to restart.  Continue polling with qti_mbox_process() and wait for
 *     the next CONNECTED event.
 *
 *   DISCONNECTED: transport re-enters link negotiation.  Continue polling
 *     with qti_mbox_process() and wait for the next CONNECTED event.
 *
 * ERROR is the only event that requires qti_mbox_release().  A channel
 * with ERROR set will continue to report ERROR on every subsequent
 * qti_mbox_process() call until qti_mbox_release() is called.
 *
 * TX_DONE is set exactly once per qti_mbox_send() call, when the remote
 * endpoint clears the local msg_len field to signal that it has read the
 * message.  It is not set if the channel disconnects before the remote
 * consumes the message.
 */
#define QTI_MBOX_EVT_CONNECTED BIT_32(0)
#define QTI_MBOX_EVT_DISCONNECTED BIT_32(1)
#define QTI_MBOX_EVT_RX_READY BIT_32(2)
#define QTI_MBOX_EVT_TX_DONE BIT_32(3)
#define QTI_MBOX_EVT_REMOTE_RESET BIT_32(4)
#define QTI_MBOX_EVT_ERROR BIT_32(5)

/* Opaque channel handle returned by qti_mbox_request(). */
struct qti_mbox_chan;

/*
 * qti_mbox_init() - initialize the mailbox framework.
 *
 * Calls the platform-specific initialization function (qti_mbox_plat_init())
 * to perform any required platform setup before the mailbox framework is used.
 * Must be called once during system initialization, before any call to
 * qti_mbox_request().
 *
 * Return: 0 on success, negative errno on failure.
 */
int qti_mbox_init(void);

/*
 * qti_mbox_request() - request a mailbox channel by name.
 * @name: channel name registered by the platform; must be non-NULL and
 *        non-empty.
 * @chan: output handle; set to NULL on failure.
 *
 * Locates the platform channel configuration, claims a runtime slot, and
 * initialises the transport.  The channel is ready for use when this
 * function returns 0.
 *
 * Return: 0 on success, negative errno on failure.
 *   -EINVAL  invalid argument (name is NULL/empty, or chan is NULL)
 *   -ENODEV  platform data unavailable or transport init failed
 *   -ENOENT  unknown channel name
 *   -EINVAL  channel configuration is invalid (missing ops)
 *   -EBUSY   channel already claimed by another caller
 */
int qti_mbox_request(const char *name, struct qti_mbox_chan **chan);

/*
 * qti_mbox_release() - release a mailbox channel.
 * @chan: channel handle returned by qti_mbox_request(); NULL is safe.
 *
 * Tears down the transport connection and returns the runtime slot to the
 * free pool.  After this call the handle must not be used.  Clears any
 * sticky ERROR event.
 *
 * This function is idempotent: calling it with NULL or an already-released
 * handle has no effect.
 *
 * Note: qti_mbox_release() is NOT required after DISCONNECTED or
 * REMOTE_RESET events.  The transport reconnects automatically.  Call
 * qti_mbox_release() only when the channel is no longer needed or when
 * QTI_MBOX_EVT_ERROR is set.
 */
void qti_mbox_release(struct qti_mbox_chan *chan);

/*
 * qti_mbox_process() - advance the transport state machine.
 * @chan:   channel handle returned by qti_mbox_request().
 * @events: output bitmap of QTI_MBOX_EVT_* bits; set to 0 on failure or
 *          when no events are pending.
 *
 * Polls the transport for new events and advances the connection state
 * machine.  Must be called periodically by the client to collect
 * connection, disconnect, and RX events.
 *
 * If @chan is NULL or @events is NULL, *events is not written and
 * -EINVAL is returned.  If the channel is not yet ready (not returned
 * by a successful qti_mbox_request()), *events is set to 0 and 0 is
 * returned.
 *
 * Return: 0 on success, -EINVAL if chan or events is NULL.
 */
int qti_mbox_process(struct qti_mbox_chan *chan, uint32_t *events);

/*
 * qti_mbox_send() - transmit a message.
 * @chan: channel handle; must be non-NULL and in READY state.
 * @buf:  message buffer; must be non-NULL.
 * @len:  message length in bytes; must be > 0 and <= effective MTU.
 *
 * Return: 0 on success, negative errno on failure.
 *   -EINVAL   invalid argument (NULL chan/buf, or zero len)
 *   -ENODEV   channel not ready or MTU not yet negotiated (MTU == 0)
 *   -EMSGSIZE len exceeds effective MTU
 *   -ENOTCONN transport not yet connected (QMP: not in E2E_CONNECTED state)
 *   -EBUSY    mailbox occupied by a previous unacknowledged message
 *   Additional transport-specific codes may be returned by the underlying
 *   transport ops (e.g. -EIO for a transport-level error).
 */
int qti_mbox_send(struct qti_mbox_chan *chan, const void *buf, size_t len);

/*
 * qti_mbox_recv() - receive a message.
 * @chan: channel handle; must be non-NULL and in READY state.
 * @buf:  receive buffer; must be non-NULL.
 * @len:  in/out: buffer capacity on entry; received message size on success.
 *
 * On -ENOSPC, *len is updated to the required buffer size and the message
 * is preserved in the mailbox so the caller can retry with a larger buffer.
 *
 * Return: 0 on success, negative errno on failure.
 *   -EINVAL  invalid argument (NULL chan/buf/len)
 *   -ENODEV  channel not ready
 *   -EAGAIN  no message queued
 *   -ENOSPC  buffer too small; *len updated to required size; message preserved
 *   -EIO     malformed remote data or transport error
 */
int qti_mbox_recv(struct qti_mbox_chan *chan, void *buf, size_t *len);

/*
 * qti_mbox_get_mtu() - return the effective MTU of the channel.
 * @chan: channel handle; must be non-NULL and in READY state.
 * @mtu:  output; set to the effective MTU in bytes on success.
 *
 * The MTU is not valid until the transport reports a CONNECTED event.
 * Poll with qti_mbox_process() and wait for QTI_MBOX_EVT_CONNECTED
 * before calling this function.
 *
 * Return: 0 on success, negative errno on failure.
 *   -EINVAL  chan or mtu is NULL, or chan is not in READY state
 *   -EAGAIN  MTU not yet negotiated; retry after QTI_MBOX_EVT_CONNECTED
 */
int qti_mbox_get_mtu(struct qti_mbox_chan *chan, size_t *mtu);

#endif /* QTI_MBOX_H */
