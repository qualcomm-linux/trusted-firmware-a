
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef STRINGL_H
#define STRINGL_H

#include <stdint.h>
#include <string.h>

/**
  memscpy - Size bounded memory copy.

  Copies bytes from the source buffer to the destination buffer.

  This function ensures that there will not be a copy beyond
  the size of the destination buffer.

  The result of calling this on overlapping source and destination
  buffers is undefined.

  @param[out] dst       Destination buffer.
  @param[in]  dst_size  Size of the destination buffer in bytes.
  @param[in]  src       Source buffer.
  @param[in]  src_size  Number of bytes to copy from source buffer.

  @return
  The number of bytes copied to the destination buffer.  It is the
  caller's responsibility to check for trunction if it cares about it -
  truncation has occurred if the return value is less than src_size.

  @dependencies
  None.
*/
size_t memscpy(void *dst, size_t dst_size, const void *src, size_t src_size);

/**
  memsmove - Size bounded memory move.

  Moves bytes from the source buffer to the destination buffer.

  This function ensures that there will not be a copy beyond
  the size of the destination buffer.

  This function should be used in preference to memscpy() if there
  is the possiblity of source and destination buffers overlapping.
  The result of the operation is defined to be as if the copy were from
  the source to a temporary buffer that overlaps neither source nor
  destination, followed by a copy from that temporary buffer to the
  destination.

  @param[out] dst       Destination buffer.
  @param[in]  dst_size  Size of the destination buffer in bytes.
  @param[in]  src       Source buffer.
  @param[in]  src_size  Number of bytes to copy from source buffer.

  @return
  The number of bytes copied to the destination buffer.  It is the
  caller's responsibility to check for trunction if it cares about it -
  truncation has occurred if the return value is less than src_size.

  @dependencies
  None.
*/

size_t memsmove(void *dst, size_t dst_size, const void *src, size_t src_size);

#endif // STRINGL_H
