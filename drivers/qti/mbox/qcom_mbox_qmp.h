/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCOM_MBOX_QMP_H
#define QCOM_MBOX_QMP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define QMP_MAGIC		0x4D41494CU
#define QMP_VERSION		1U
#define QMP_FEATURES		0U
#define QMP_LINK_DOWN		0xFFFF0000U
#define QMP_LINK_UP		0x0000FFFFU
#define QMP_CH_DISCONNECTED	0xFFFF0000U
#define QMP_CH_CONNECTED	0x0000FFFFU

struct qcom_mbox_signal_config {
	uintptr_t	reg;
	uint32_t	value;
};

/*
 * desc_base:   physical base of the shared QMP descriptor and mailbox region.
 * shared_size: total size of the shared region; used for layout validation.
 */
struct qcom_mbox_qmp_config {
	uintptr_t			desc_base;
	uint32_t			shared_size;
	struct qcom_mbox_signal_config	remote_signal;
};

enum qcom_mbox_qmp_state {
	QMP_STATE_LINK_DOWN = 0,
	QMP_STATE_LINK_NEGOTIATION,
	QMP_STATE_LOCAL_CONNECTING,
	QMP_STATE_E2E_CONNECTED,
};

/*
 * layout_valid:        set after the shared descriptor layout is validated.
 * local_desc_base:     base of the local (SCORE) endpoint descriptor.
 * remote_desc_base:    base of the remote (MCORE) endpoint descriptor.
 * local_mbox_base:     base of the local transmit mailbox.
 * remote_mbox_base:    base of the remote receive mailbox.
 * local_payload_size:  usable TX capacity (local_mbox_size - sizeof(msg_len)).
 * remote_payload_size: usable RX capacity (remote_mbox_size - sizeof(msg_len)).
 */
struct qcom_mbox_qmp_priv {
	const struct qcom_mbox_qmp_config	*cfg;
	enum qcom_mbox_qmp_state		 state;
	bool					 layout_valid;
	uintptr_t				 local_desc_base;
	uintptr_t				 remote_desc_base;
	uintptr_t				 local_mbox_base;
	uintptr_t				 remote_mbox_base;
	uint32_t				 local_payload_size;
	uint32_t				 remote_payload_size;
};

struct qcom_mbox_ops;
extern const struct qcom_mbox_ops qcom_mbox_qmp_ops;

#endif /* QCOM_MBOX_QMP_H */