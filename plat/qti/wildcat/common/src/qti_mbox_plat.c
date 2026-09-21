// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

/*
 * QTI mailbox channel configuration.
 */

#include <stddef.h>

#include <drivers/qti/mbox/qti_mbox_plat.h>
#include <drivers/qti/mbox/qti_mbox_qmp.h>
#include <lib/utils_def.h>
#include <platform_def.h>

/*
 * QMP transport configuration for the TME channel.
 */
static struct qti_mbox_qmp_config qti_mbox_qmp_tme_cfg = {
	.desc_base   = QTI_TME_MBOX_RAM_HIGH,
	.shared_size = QTI_TME_MBOX_RAM_HIGH_SIZE,
	.remote_signal = {
		.reg   = QTI_TFA_TME_MBOX_INTERRUPT,
		.value = QTI_TFA_TME_MBOX_INTERRUPT_MSK,
	},
};

static struct qti_mbox_qmp_priv qti_mbox_qmp_tme_priv;

static const struct qti_mbox_chan_config qti_mbox_channels[] = {
	{
		.name = "tme-qmp",
		.ops = &qti_mbox_qmp_ops,
		.transport_cfg = &qti_mbox_qmp_tme_cfg,
		.transport_priv = &qti_mbox_qmp_tme_priv,
	},
};

static struct qti_mbox_chan_slot
	qti_mbox_chan_slots[ARRAY_SIZE(qti_mbox_channels)];

static const struct qti_mbox_plat_data qti_mbox_plat_data = {
	.configs = qti_mbox_channels,
	.slots = qti_mbox_chan_slots,
	.num_channels = ARRAY_SIZE(qti_mbox_channels),
};

int qti_mbox_plat_init(const struct qti_mbox_plat_data **plat_data)
{
	*plat_data = &qti_mbox_plat_data;
	return 0;
}
