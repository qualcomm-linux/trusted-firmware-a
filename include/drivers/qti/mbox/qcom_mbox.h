/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCOM_MBOX_H
#define QCOM_MBOX_H

#include <stddef.h>
#include <stdint.h>

/*
 * Event bitmap returned by qcom_mbox_process().
 *
 * Edge events (latched, returned once per transition):
 *   CONNECTED, DISCONNECTED, TX_DONE, REMOTE_RESET, REMOTE_CRASH
 *
 * Level event (active while condition holds):
 *   RX_READY - at least one received message is queued
 *
 * Sticky event (active until qcom_mbox_release()):
 *   ERROR - unrecoverable transport failure
 */
#define QCOM_MBOX_EVT_CONNECTED		BIT_32(0)
#define QCOM_MBOX_EVT_DISCONNECTED	BIT_32(1)
#define QCOM_MBOX_EVT_RX_READY		BIT_32(2)
#define QCOM_MBOX_EVT_TX_DONE		BIT_32(3)
#define QCOM_MBOX_EVT_REMOTE_RESET	BIT_32(4)
#define QCOM_MBOX_EVT_REMOTE_CRASH	BIT_32(5)
#define QCOM_MBOX_EVT_ERROR		BIT_32(6)

/* Opaque channel handle returned by qcom_mbox_request(). */
struct qcom_mbox_chan;

/*
 * qcom_mbox_request() - request a mailbox channel by name.
 * @name: channel name registered by the platform; must be non-NULL.
 * @chan: output handle; set to NULL on failure.
 *
 * Locates the platform channel configuration, claims a runtime slot,
 * and establishes the transport connection.
 *
 * Return: 0 on success, negative errno on failure.
 *   -EINVAL  invalid argument
 *   -ENOENT  unknown channel name
 *   -EBUSY   channel already claimed
 *   -ENOSPC  no free runtime slots
 *   -ENODEV  transport unavailable
 *   -ETIMEDOUT transport connection timed out
 *   -EIO     transport error
 */
int qcom_mbox_request(const char *name, struct qcom_mbox_chan **chan);

/*
 * qcom_mbox_release() - release a mailbox channel.
 * @chan: channel handle; NULL is safe and has no effect.
 *
 * Tears down the transport connection and returns the runtime slot.
 * After this call the handle must not be used.
 */
void qcom_mbox_release(struct qcom_mbox_chan *chan);

/*
 * qcom_mbox_process() - advance the transport state machine.
 * @chan: channel handle.
 *
 * Polls the transport for new events.  Must be called periodically
 * by the client to collect connection, disconnect, and RX events.
 *
 * Return: bitmap of QCOM_MBOX_EVT_* bits; 0 if chan is NULL or not READY.
 */
uint32_t qcom_mbox_process(struct qcom_mbox_chan *chan);

/*
 * qcom_mbox_send() - transmit a message.
 * @chan: channel handle; must be in READY state.
 * @buf:  message buffer; must be non-NULL.
 * @len:  message length in bytes; must be > 0 and <= effective MTU.
 *
 * Return: 0 on success, negative errno on failure.
 *   -EINVAL  invalid argument
 *   -EMSGSIZE len exceeds effective MTU
 *   -ENODEV  channel not connected
 *   -ETIMEDOUT TX timed out
 *   -EIO     transport error
 */
int qcom_mbox_send(struct qcom_mbox_chan *chan,
		   const void *buf, size_t len);

/*
 * qcom_mbox_recv() - receive a message.
 * @chan: channel handle; must be in READY state.
 * @buf:  receive buffer; must be non-NULL.
 * @len:  in/out: buffer capacity on entry, received size on success.
 *
 * On -ENOSPC, *len is set to the required size and the message is
 * preserved so the caller can retry with a larger buffer.
 *
 * Return: 0 on success, negative errno on failure.
 *   -EINVAL  invalid argument
 *   -EAGAIN  no message queued
 *   -ENOSPC  buffer too small; *len set to required size
 *   -EIO     malformed remote data or transport error
 */
int qcom_mbox_recv(struct qcom_mbox_chan *chan,
		   void *buf, size_t *len);

/*
 * qcom_mbox_get_mtu() - return the effective MTU of the channel.
 * @chan: channel handle; must be in READY state.
 * @mtu:  output; set to the effective MTU in bytes.
 *
 * Return: 0 on success, -EINVAL if chan or mtu is NULL or chan not READY.
 */
int qcom_mbox_get_mtu(struct qcom_mbox_chan *chan, size_t *mtu);

#endif /* QCOM_MBOX_H */