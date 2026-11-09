/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <lib/mmio.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>
#include <platform_def.h>


int plat_cpu_scmi_protocol_init(scmi_channel_t *ch)
{
	int ret;

	ret = scmi_reset_domain_init(ch);
	if (ret != 0) {
		ERROR("SCMI: Reset domain init failed: %d\n", ret);
		return ret;
	}

	ret = scmi_pwr_init(ch);
	if (ret != 0) {
		ERROR("SCMI: PWR domain init failed: %d\n", ret);
		return ret;
	}

	return ret;
}
