/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Qualcomm Mailbox channel configuration.
 */

#include <stddef.h>
#include <lib/utils_def.h>

#include <qcom_mbox_plat.h>
#include <qcom_mbox_qmp.h>

/* TME_MBOX_MBOX_RAM */
#define TME_MBOX_MBOX_RAM_HIGH			0x3623e800
#define TME_MBOX_MBOX_RAM_HIGH_SIZE		0x00001000

/* APSS_INTU_TZ_TME_MBOX_INTERRUPT */
#define APSS_INTU_TZ_TME_MBOX_INTERRUPT		0x17824010
#define APSS_INTU_TZ_TME_MBOX_INTERRUPT_MSK	0x1

static const struct qcom_mbox_qmp_config qcom_mbox_qmp_tme_cfg = {
	.desc_base    = TME_MBOX_MBOX_RAM_HIGH,
	.shared_size  = TME_MBOX_MBOX_RAM_HIGH_SIZE,
	.remote_signal = {
		.reg   = APSS_INTU_TZ_TME_MBOX_INTERRUPT,
		.value = APSS_INTU_TZ_TME_MBOX_INTERRUPT_MSK,
	},
};

static struct qcom_mbox_qmp_priv qcom_mbox_qmp_tme_priv;

static const struct qcom_mbox_chan_config qcom_mbox_channels[] = {
	{
		.name           = "tme-qmp",
		.ops            = &qcom_mbox_qmp_ops,
		.transport_cfg  = &qcom_mbox_qmp_tme_cfg,
		.transport_priv = &qcom_mbox_qmp_tme_priv,
	}
};

static struct qcom_mbox_chan_slot qcom_mbox_chan_slots[ARRAY_SIZE(qcom_mbox_channels)];

static const struct qcom_mbox_plat_data qcom_mbox_plat_data = {
	.configs      = qcom_mbox_channels,
	.slots        = qcom_mbox_chan_slots,
	.num_channels = ARRAY_SIZE(qcom_mbox_chan_slots),
};

const struct qcom_mbox_plat_data *plat_qcom_mbox_get_data(void)
{
	return &qcom_mbox_plat_data;
}