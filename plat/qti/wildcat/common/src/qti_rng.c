/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Portions copyright (c) 2026, Qualcomm Technologies, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <arch_features.h>

#ifdef QTI_USE_SEC_PRNG
#include <drivers/qti/crypto/rng.h>
#endif

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
#ifdef QTI_USE_SEC_PRNG
	if (out == NULL) {
		return false;
	}

	return qti_rng_get_data((uint8_t *)out, sizeof(*out)) == 0;
#else
	bool ok = false;
	uint64_t value = 0;

	if (out == NULL) {
		return false;
	}

    if (!is_feat_rng_supported()) {
        return false;
    }

	__asm volatile(
		".arch_extension rng\n\t"
		"mrs	%[value], RNDR\n\t"
		"cset	%w[ok], ne\n\t"
		".arch_extension norng\n\t"
		: [value] "=r" (value), [ok] "=r" (ok)
		:
		: "cc"
	);

	if (!ok) {
		return false;
	}

	*out = value;
	return true;
#endif
}
