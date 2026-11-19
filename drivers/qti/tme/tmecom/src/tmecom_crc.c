/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <tmecom_crc.h>

bool tme_does_crc16_match(uint16_t crc16, const void *p_buffer, size_t size)
{
	return crc16 == tme_calculate_crc16(p_buffer, size);
}

uint16_t tme_calculate_crc16(const void *p_buffer, size_t size)
{
	uint16_t crc16 = 0;

	if ((p_buffer != NULL) && (size != 0U)) {
		static const uint16_t crc16_polynomial = 0x8408;
		static const uint16_t crc16_preset_value = 0xFFFF;
		const uint8_t *p_byte = p_buffer;

		for (crc16 = crc16_preset_value; size > 0; --size, ++p_byte) {
			crc16 ^= *p_byte & 0xFF;

			for (size_t j = 0; j < 8; ++j) {
				if ((crc16 & 1U) != 0U) {
					crc16 = (crc16 >> 1) ^ crc16_polynomial;
				} else {
					crc16 >>= 1;
				}
			}
		}

		/* Return the 1's complement of the CRC */
		crc16 ^= crc16_preset_value;
	}

	return crc16;
}
