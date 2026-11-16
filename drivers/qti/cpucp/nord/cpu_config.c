/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <drivers/arm/css/scmi.h>
#include <platform_def.h>


int plat_cpu_scmi_protocol_init(scmi_channel_t *ch)
{
	int ret;

	ret = scmi_reset_domain_init(ch);
	if (ret != 0) {
		return ret;
	}

	ret = scmi_pwr_init(ch);
	if (ret != 0) {
		return ret;
	}

	return ret;
}
