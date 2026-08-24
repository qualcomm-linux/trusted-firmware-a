/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>

#include <drivers/qti/rpm/rpm_transport.h>

__attribute__((weak))
int qti_rpm_transport_init(const struct qti_rpm_transport_config *cfg)
{
	(void)cfg;
	return -ENODEV;
}

__attribute__((weak))
int qti_rpm_transport_tx(const void *buf, size_t len)
{
	(void)buf;
	(void)len;
	return -ENODEV;
}

__attribute__((weak))
int qti_rpm_transport_poll(void)
{
	return -ENODEV;
}

__attribute__((weak))
int qti_rpm_transport_mask_rx_interrupt(bool mask)
{
	(void)mask;
	return -ENODEV;
}
