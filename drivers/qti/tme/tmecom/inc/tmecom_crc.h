/*===========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef TMECOM_CRC_H_
#define TMECOM_CRC_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @addtogroup Utilities
 * @{
 */

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * Compare source CRC16 against the CRC16 calculated on input data.
   *
   * @param[in]  crc16    Input CRC16 to compare against the CRC of the input data buffer.
   * @param[in]  pbuffer  Pointer to the data buffer whose CRC is to be calculated
   *                      and compared against the source CRC16.
   * @param[in]  size     Number of bytes to use when calculating the CRC.
   *
   * @return True if the input CRC16 matches the CRC of the input data buffer,
   *         false otherwise.
   */
  bool tmeDoesCRC16Match(uint16_t crc16, const void *pbuffer, size_t size);

  /**
   * Calculate CRC16 on input data.
   *
   * @param[in]  pbuffer  Pointer to the data buffer for which the CRC is to be calculated.
   * @param[in]  size     Number of bytes to use when calculating the CRC.
   *
   * @return Computed CRC16 of the given data.
   */
  uint16_t tmeCalculateCRC16(const void *pbuffer, size_t size);

#ifdef __cplusplus
}
#endif

/** @} */ /* end addtogroup Utilities */

#endif /* TMECOM_CRC_H_ */
