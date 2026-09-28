/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <string.h>

size_t memscpy(void *dst, size_t dst_size, const void *src, size_t src_size)
{
	size_t copy_size = (src_size < dst_size) ? src_size : dst_size;

	memcpy(dst, src, copy_size);
	return copy_size;
}

size_t memsmove(void *dst, size_t dst_size, const void *src, size_t src_size)
{
	size_t copy_size = (src_size < dst_size) ? src_size : dst_size;

	memmove(dst, src, copy_size);
	return copy_size;
}