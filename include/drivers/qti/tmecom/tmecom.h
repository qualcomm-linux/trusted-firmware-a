/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Public interface for the upstream TMECOM driver (drivers/qti/tme/tmecom.c).
 *
 * Uses the qcom_mbox mailbox API for transport; does not depend on GLink.
 */

#ifndef QTI_TMECOM_H
#define QTI_TMECOM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Wire header prepended to every request and response. */
struct tmecom_msg_hdr {
	uint16_t version;
	uint16_t crc;
	uint32_t txn_id;
} __packed;

#define TMECOM_MSG_HDR_SIZE	sizeof(struct tmecom_msg_hdr)

/* Version field value placed in every outbound header (major=1, minor=0). */
#define TMECOM_WIRE_VERSION	0x0100U

/* Maximum on-wire message size (header + payload). */
#define TMECOM_MAX_MSG_SIZE	2048U

/* Maximum payload the caller may pass to tmecom_send_recv(). */
#define TMECOM_MAX_PAYLOAD_SIZE	(TMECOM_MAX_MSG_SIZE - TMECOM_MSG_HDR_SIZE)

/*
 * tmecom_init() - open a named channel and wait for the remote to connect.
 * @channel_name: mailbox channel name registered by the platform.
 *
 * Return: 0 on success, negative errno on failure.
 */
int tmecom_init(const char *channel_name);

/*
 * tmecom_deinit() - close the channel and release all resources.
 */
void tmecom_deinit(void);

/*
 * tmecom_send_recv() - synchronous request/response exchange.
 * @req:      request payload (caller-owned).
 * @req_size: size of @req in bytes; must be <= TMECOM_MAX_PAYLOAD_SIZE.
 * @rsp:      response buffer (caller-owned).
 * @rsp_size: in/out — capacity on entry, actual response size on success.
 *
 * Convenience wrapper: tmecom_send() followed by a bounded poll loop over
 * tmecom_recv().
 *
 * Return: 0 on success, negative errno on failure.
 */
int tmecom_send_recv(const void *req, size_t req_size,
		     void *rsp, size_t *rsp_size);

/*
 * tmecom_send() - submit a request without waiting for the response.
 * @req:      request payload (caller-owned).
 * @req_size: size of @req in bytes; must be <= TMECOM_MAX_PAYLOAD_SIZE.
 * @txn_id:   out — transaction id identifying this request, to be passed to
 *            tmecom_recv().
 *
 * Frames the request and hands it to the transport.  Returns once the local
 * transport has accepted the message (a bounded tx wait); does NOT wait for
 * the remote to respond.  Only one transaction may be outstanding at a time.
 *
 * Return: 0 on success, -EAGAIN if a transaction is already outstanding,
 *         negative errno on other failure.
 */
int tmecom_send(const void *req, size_t req_size, uint32_t *txn_id);

/*
 * tmecom_recv() - poll, without blocking, for the response to a request
 *                 previously submitted via tmecom_send().
 * @txn_id:   transaction id returned by tmecom_send().
 * @rsp:      response buffer (caller-owned).
 * @rsp_size: in/out — capacity on entry, actual response size on success.
 *
 * Advances the transport once and checks for a ready response.  Clears the
 * outstanding-transaction state on any terminal outcome (success or error);
 * leaves it set on -EINPROGRESS so the caller can poll again.
 *
 * Return: 0 if the response is ready and copied out, -EINPROGRESS if the
 *         remote has not responded yet, -EINVAL if @txn_id does not match the
 *         outstanding transaction, negative errno on other failure.
 */
int tmecom_recv(uint32_t txn_id, void *rsp, size_t *rsp_size);

/*
 * tmecom_is_connected() - poll channel state.
 *
 * Return: true if the channel is currently connected.
 */
bool tmecom_is_connected(void);

#endif /* QTI_TMECOM_H */
