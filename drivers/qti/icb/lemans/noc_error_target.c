/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Lemans (QCS9075) target back-end for the QTI NoC error handler.
 */

#include <stddef.h>

#include "noc_error.h"
#include "noc_error_target.h"

void qti_noc_error_init_target(struct nocerr_info *noc_info_list,
			       uint32_t len,
			       struct nocerr_info_oem *noc_info_oem_list)
{
	(void)noc_info_list;
	(void)len;
	(void)noc_info_oem_list;
}

bool qti_noc_error_handle_target(struct nocerr_info *noc_info,
				 struct nocerr_info_oem *noc_info_oem,
				 bool *delay_fatal)
{
	if (delay_fatal != NULL)
		*delay_fatal = false;

	if (noc_info == NULL || noc_info_oem == NULL)
		return true;

	return noc_info_oem->error_fatal;
}

void qti_noc_error_ffi_mem_map_wrap(bool is_map)
{
	(void)is_map;
}

bool qti_noc_error_is_part_disabled(uint32_t qultivate_part_type, uint32_t idx)
{
	(void)qultivate_part_type;
	(void)idx;

	/* Lemans does not gate any NoC instance via SKU fuses. */
	return false;
}
