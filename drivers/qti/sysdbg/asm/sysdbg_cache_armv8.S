/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.globl	icache_enable
	.globl	dcache_enable

	/* SCTLR_EL3 bits. */
	.set SCTLR_EL3_I_BIT_MASK, (1 << 0x0C)
	.set SCTLR_EL3_C_BIT_MASK, (1 << 0x02)

	/* ---------------------------------------------------------------
	 * void icache_enable(uint64_t sctlr)
	 * ---------------------------------------------------------------
	 */
func icache_enable
	mov	x1, #(SCTLR_EL3_I_BIT_MASK)
	orr	x0, x0, x1
	msr	sctlr_el3, x0
	ret
endfunc icache_enable

	/* ---------------------------------------------------------------
	 * void dcache_enable(uint64_t sctlr)
	 * ---------------------------------------------------------------
	 */
func dcache_enable
	mov	x1, #(SCTLR_EL3_C_BIT_MASK)
	orr	x0, x0, x1
	msr	sctlr_el3, x0
	ret
endfunc dcache_enable
