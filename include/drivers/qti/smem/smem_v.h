/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMEM_V_H
#define SMEM_V_H

#include <stdint.h>

#include "smem_type.h"

#define SMEM_ALLOC_FLAG_CACHED	0x80000000 /* Allocate from cached heap */
#define SMEM_ALLOC_FLAG_PARTITION_ONLY \
	0x40000000 /* Allocate from edge partition only */

#endif /* SMEM_V_H */
