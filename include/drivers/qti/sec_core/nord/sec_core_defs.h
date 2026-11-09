/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NORD_SEC_CORE_DEFS_H
#define NORD_SEC_CORE_DEFS_H

/*
 * NCC RVBAR address registers for Nord.
 * These are the 32-bit register addresses used to program the reset vector
 * base address for the NCC (Next-generation CPU Cluster) on Nord.
 *
 * The LO register receives bits [31:0] of the physical entrypoint address.
 * The HI register receives bits [63:32] of the physical entrypoint address.
 */
#define NCC0_NCC_NCC_ARCH_0_NCC_RVBAR_ADDR_LO		(0x19880328UL)
#define NCC0_NCC_NCC_ARCH_0_NCC_RVBAR_ADDR_HI		(0x1988032CUL)

#endif /* NORD_SEC_CORE_DEFS_H */