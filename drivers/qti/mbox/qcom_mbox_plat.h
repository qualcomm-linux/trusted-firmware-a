/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCOM_MBOX_PLAT_H
#define QCOM_MBOX_PLAT_H

/*
 * Platform-integration header for qcom_mbox.
 *
 * Platform code includes this header to define the static channel
 * configuration array, the matching runtime slot array, and the
 * platform mailbox-data descriptor.
 *
 * The platform derives the channel count from the configuration array
 * using ARRAY_SIZE().  Configuration entry N permanently maps to
 * runtime slot N.  The common driver never defines a channel or slot
 * array and never uses a common maximum channel count.
 *
 * Typical platform usage:
 *
 *   static const struct qcom_mbox_chan_config plat_channels[] = {
 *       { .name = "tme-qmp", .ops = &qcom_mbox_qmp_ops, ... },
 *       { .name = "tme-qmp-lite", .ops = &qcom_mbox_qmp_lite_ops, ... },
 *   };
 *
 *   static struct qcom_mbox_chan_slot
 *       plat_slots[ARRAY_SIZE(plat_channels)];
 *
 *   static const struct qcom_mbox_plat_data plat_mbox_data = {
 *       .configs      = plat_channels,
 *       .slots        = plat_slots,
 *       .num_channels = ARRAY_SIZE(plat_channels),
 *   };
 *
 *   const struct qcom_mbox_plat_data *plat_qcom_mbox_get_data(void)
 *   {
 *       return &plat_mbox_data;
 *   }
 */

#include <stddef.h>

#include "qcom_mbox_private.h"

/*
 * Platform mailbox-data descriptor.
 *
 * The platform provides one immutable instance of this structure.
 * configs[N] permanently maps to slots[N]; the common driver selects
 * slot N when a request matches configuration N.
 *
 * configs:      pointer to the immutable channel configuration array.
 * slots:        pointer to the mutable runtime slot array.
 * num_channels: number of entries in both arrays; must equal
 *               ARRAY_SIZE(configs) == ARRAY_SIZE(slots).
 */
struct qcom_mbox_plat_data {
	const struct qcom_mbox_chan_config	*configs;
	struct qcom_mbox_chan_slot		*slots;
	size_t					 num_channels;
};

/*
 * plat_qcom_mbox_get_data - return the platform mailbox-data descriptor.
 *
 * The platform must implement this function.  It is called by the
 * common driver on every qcom_mbox_request() to obtain the channel
 * configuration and runtime slot arrays.
 *
 * The returned pointer must be non-NULL and must remain valid for the
 * lifetime of the driver.  The backing data must be statically allocated.
 *
 * Return: pointer to the platform mailbox-data descriptor.
 */
const struct qcom_mbox_plat_data *plat_qcom_mbox_get_data(void);

#endif /* QCOM_MBOX_PLAT_H */