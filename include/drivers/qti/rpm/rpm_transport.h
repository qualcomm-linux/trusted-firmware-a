/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_RPM_TRANSPORT_H
#define QTI_RPM_TRANSPORT_H

#include <stdbool.h>
#include <stddef.h>

struct qti_rpm_transport_config {
	const char	*channel_name;
	const char	*remote_ss;
};

int qti_rpm_transport_init(const struct qti_rpm_transport_config *cfg);
int qti_rpm_transport_tx(const void *buf, size_t len);
int qti_rpm_transport_poll(void);
int qti_rpm_transport_mask_rx_interrupt(bool mask);

/*
 * Transport backends call this from their RX path with one complete RPM packet.
 */
void qti_rpm_rx(const void *buf, size_t len);

#endif /* QTI_RPM_TRANSPORT_H */
