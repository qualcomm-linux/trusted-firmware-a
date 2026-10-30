/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <drivers/qti/vpp/vpp.h>

extern vpp_t qti_vpp_table[];
extern const uint32_t qti_vpp_table_size;

const vpp_t *qti_vpp_get_plan(const char *rail, const char *clk)
{
	for (uint32_t i = 0; i < qti_vpp_table_size; i++) {
		if ((strcmp(qti_vpp_table[i].rail_name, rail) == 0) &&
		    (strcmp(qti_vpp_table[i].clk_name, clk) == 0)) {
			return &qti_vpp_table[i];
		}
	}

	return NULL;
}
