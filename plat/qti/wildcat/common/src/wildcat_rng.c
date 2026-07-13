/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <arch_features.h>

#include <drivers/qti/crypto/rng.h>

#if TRNG_SUPPORT

#include <lib/smccc.h>
#include <tools_share/uuid.h>

DEFINE_SVC_UUID2(_plat_trng_uuid, 0xeb1445fd, 0xac25, 0x4561, 0xa4, 0x88, 0x05,
		 0xf1, 0x0c, 0x69, 0x76, 0x4c);

uuid_t plat_trng_uuid;

void plat_entropy_setup(void)
{
	plat_trng_uuid = _plat_trng_uuid;
}
#endif /* TRNG_SUPPORT */

/**
 *  bool plat_get_entropy(uint64_t *out) [mandatory]
 *
 * This function writes entropy into storage provided by the caller. If no
 * entropy is available, it must return false and the storage must not be
 * written.
 */
bool plat_get_entropy(uint64_t *out)
{
	if (out == NULL) {
		return false;
	}

	if (qti_rng_get_data((uint8_t *)out, sizeof(uint64_t)) == 0) {
		return true;
	}

	return false;
}
