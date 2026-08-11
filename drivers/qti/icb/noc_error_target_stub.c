/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Default no-op back-end. Platforms with no target-specific NoC error
 * handling (no SKU fusing, no FFI safety mapping) link against this stub;
 * platforms that need either provide their own implementation.
 */

#include <stddef.h>

#include "noc_error.h"
#include "noc_error_target.h"

/* No platform NoC error configuration data in the absence of a back-end. */
struct nocerr_config_info nocerr_config_info = {
	.num_configs = 0U,
	.configs = NULL,
};

struct nocerr_config_info_oem nocerr_config_info_oem = {
	.num_configs = 0U,
	.configs = NULL,
};

void qti_noc_error_init_target(struct nocerr_info_type *noc_info_list,
				uint32_t len,
				struct nocerr_info_type_oem *noc_info_oem_list)
{
	(void)noc_info_list;
	(void)len;
	(void)noc_info_oem_list;
}

bool qti_noc_error_handle_target(struct nocerr_info_type *noc_info,
				  struct nocerr_info_type_oem *noc_info_oem,
				  bool *delay_fatal)
{
	(void)noc_info;
	(void)noc_info_oem;

	if (delay_fatal != NULL) {
		*delay_fatal = false;
	}

	/* Treat every fault as fatal in the absence of a back-end. */
	return true;
}

void qti_noc_error_ffi_mem_map_wrap(bool is_map)
{
	(void)is_map;
}
