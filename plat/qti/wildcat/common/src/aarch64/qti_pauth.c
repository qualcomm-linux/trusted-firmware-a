/**********************************************************************
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *********************************************************************/

#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <plat_trng.h>

/*
 * Generate a random 128bit key for PAUTH
 *
 * This function will panic if a random number could not be obtained
 */
uint128_t plat_init_apkey(void)
{
	uint64_t hi, lo;

	if (!(plat_get_entropy(&hi) && plat_get_entropy(&lo))) {
		ERROR("Unable to get entropy in %s", __func__);
		panic();
	}

	return ((uint128_t)(hi) << 64) | lo;
}
