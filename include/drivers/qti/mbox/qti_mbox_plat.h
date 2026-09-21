/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef QTI_MBOX_PLAT_H
#define QTI_MBOX_PLAT_H

/*
 * Platform-integration header for the Qualcomm mailbox framework.
 *
 * Platform code includes this header to define the static channel
 * configuration array, the matching runtime slot array, and the platform
 * mailbox-data descriptor.  The framework core obtains these arrays via
 * qti_mbox_plat_init(), which the platform must implement.
 *
 * Typical platform usage
 * ----------------------
 *
 *   #include <drivers/qti/mbox/qti_mbox_plat.h>
 *   #include <drivers/qti/mbox/qti_mbox_qmp.h>   // for QMP transport
 *
 *   static const struct qti_mbox_qmp_config my_qmp_cfg = { ... };
 *   static struct qti_mbox_qmp_priv         my_qmp_priv;
 *
 *   static const struct qti_mbox_chan_config plat_channels[] = {
 *       {
 *           .name           = "my-channel",
 *           .ops            = &qti_mbox_qmp_ops,
 *           .transport_cfg  = &my_qmp_cfg,
 *           .transport_priv = &my_qmp_priv,
 *       },
 *   };
 *
 *   static struct qti_mbox_chan_slot
 *       plat_slots[ARRAY_SIZE(plat_channels)];
 *
 *   static const struct qti_mbox_plat_data plat_mbox_data = {
 *       .configs      = plat_channels,
 *       .slots        = plat_slots,
 *       .num_channels = ARRAY_SIZE(plat_channels),
 *   };
 *
 *   int qti_mbox_plat_init(const struct qti_mbox_plat_data **plat_data)
 *   {
 *       // perform platform-specific setup (e.g. SMEM lookup)
 *       *plat_data = &plat_mbox_data;
 *       return 0;
 *   }
 *
 * Constraints
 * -----------
 * - configs[N] permanently maps to slots[N]; the framework selects slot N
 *   when a request matches configuration N.
 * - num_channels must equal ARRAY_SIZE(configs) == ARRAY_SIZE(slots).
 * - All arrays must be statically allocated and remain valid for the
 *   lifetime of the driver.
 * - qti_mbox_plat_init() must write a non-NULL pointer into *plat_data
 *   on success.
 */

#include <stddef.h>

#include <drivers/qti/mbox/qti_mbox_types.h>

/*
 * Platform mailbox-data descriptor.
 *
 * The platform provides one immutable instance of this structure.
 *
 * configs:      pointer to the immutable channel configuration array.
 * slots:        pointer to the mutable runtime slot array.
 * num_channels: number of entries in both arrays.
 */
struct qti_mbox_plat_data {
	const struct qti_mbox_chan_config *configs;
	struct qti_mbox_chan_slot *slots;
	size_t num_channels;
};

/*
 * qti_mbox_plat_init() - platform-specific mailbox initialization.
 *
 * Performs any required platform setup (e.g. SMEM address discovery) and
 * writes a pointer to the static platform mailbox-data descriptor into
 * @plat_data.
 *
 * Called once by qti_mbox_init() before any qti_mbox_request() call.
 * The pointer written to @plat_data must remain valid for the lifetime
 * of the driver.
 *
 * @plat_data: [out] set to the platform mailbox-data descriptor on success.
 *
 * Return: 0 on success, negative errno on failure.
 */
int qti_mbox_plat_init(const struct qti_mbox_plat_data **plat_data);

#endif /* QTI_MBOX_PLAT_H */
