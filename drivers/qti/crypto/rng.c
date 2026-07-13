/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2026, Qualcomm Technologies, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>

#include <arch_helpers.h>
#include <drivers/qti/crypto/rng.h>
#include <lib/mmio.h>

#if !ENABLE_FEAT_RNG   /* SOC PRNG path */
/*
 * qti_rng_read_64()
 *
 * Reads 64 bits of entropy from the SoC PRNG by fetching
 * two consecutive 32‑bit words from the DATA_OUT register.
 */
static bool qti_rng_read_64(uint64_t *out)
{
	uint32_t lo = 0;
	uint32_t hi = 0;

	if (out == NULL) {
		return false;
	}

	do {
		if ((mmio_read_32(SEC_PRNG_STATUS) &
		     SEC_PRNG_STATUS_DATA_AVAIL_BMSK) == 0) {
			continue;
		}

		lo = mmio_read_32(SEC_PRNG_DATA_OUT);
		break;
	} while (1);

	do {
		if ((mmio_read_32(SEC_PRNG_STATUS) &
		     SEC_PRNG_STATUS_DATA_AVAIL_BMSK) == 0) {
			continue;
		}

		hi = mmio_read_32(SEC_PRNG_DATA_OUT);
		break;

	} while (1);

	*out = ((uint64_t)hi << 32) | (uint64_t)lo;

	return true;
}

#else  /* ENABLE_FEAT_RNG - ARM RNDR path */
/*
 * qti_rng_read_64()
 *
 * Reads a 64-bit random number using TF-A's architectural RNDR helper.
 */
static bool qti_rng_read_64(uint64_t *out)
{
	*out = read_rndr();

	return true;
}

#endif /* ENABLE_FEAT_RNG */

int qti_rng_get_data(uint8_t *out, uint32_t out_len)
{
	uint64_t tmp_rndm = 0;
	uint32_t bytes_left = out_len;
	int ret = -1;

	if (out == NULL || out_len == 0) {
		goto bail;
	}

	do {
		if (qti_rng_read_64(&tmp_rndm) == false) {
			goto bail;
		}

		for (int i = 0; i < sizeof(tmp_rndm); i++) {
			*out = (uint8_t)(tmp_rndm & 0xFFU);
			tmp_rndm >>= 8;

			out++;
			bytes_left--;

			if (bytes_left == 0) {
				break;
			}
		}

	} while (bytes_left != 0);

	ret = 0;

bail:
	return ret;
}
