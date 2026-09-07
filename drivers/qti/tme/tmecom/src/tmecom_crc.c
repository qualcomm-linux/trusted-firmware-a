/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "tmecom_crc.h"

bool tmeDoesCRC16Match(uint16_t crc16, const void *pbuffer, size_t size)
{
  return crc16 == tmeCalculateCRC16(pbuffer, size);
}

uint16_t tmeCalculateCRC16(const void *pbuffer, size_t size)
{
  uint16_t crc16 = 0;

  if (pbuffer && size)
  {
    static const uint16_t TME_CCITT_CRC16_POLYNOMIAL   = 0x8408;
    static const uint16_t TME_CCITT_CRC16_PRESET_VALUE = 0xFFFF;
    const uint8_t        *pByte                        = pbuffer;

    for (crc16 = TME_CCITT_CRC16_PRESET_VALUE; size > 0; --size, ++pByte)
    {
      crc16 ^= *pByte & 0xFF;

      for (size_t j = 0; j < 8; ++j)
      {
        if (crc16 & 1)
        {
          crc16 = (crc16 >> 1) ^ TME_CCITT_CRC16_POLYNOMIAL;
        }
        else
        {
          crc16 >>= 1;
        }
      }
    }

    /* Return the 1's complement of the CRC */
    crc16 ^= TME_CCITT_CRC16_PRESET_VALUE;
  }

  return crc16;
}
