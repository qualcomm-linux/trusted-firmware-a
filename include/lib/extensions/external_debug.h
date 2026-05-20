/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EXTERNAL_DEBUG_H
#define EXTERNAL_DEBUG_H

#include <stdbool.h>

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct cpu_context;

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/

/*
 * These functions manage MDCR_EL3.EDAD (bit 20), External Debug Access Disable
 */

void disable_external_debug_access(cpu_context_t *ctx);
void enable_external_debug_access(cpu_context_t *ctx);





#endif /* EXTERNAL_DEBUG_H */
