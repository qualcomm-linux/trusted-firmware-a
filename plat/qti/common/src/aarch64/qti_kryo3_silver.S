/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
 * Changes from Qualcomm Technologies, Inc. are provided under the following license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cpu_macros.S>
#include <plat_macros.S>
#include <qti_cpu.h>

/* -------------------------------------------------------------
 * AArch64 EL3/EL1 system register initialization performed on
 * every Kryo-3 Silver (Cortex-A53) CPU reset (cold & warm boot).
 *
 * Clobbers: x0 - x1
 * -------------------------------------------------------------
 */
func a53_aarch64_sysini
	/* Set the SMP bit in the EL1 CPU Extended Control Register */
	mrs	x0, S3_1_C15_C2_1
	orr	x0, x0, #(1 << 6)
	msr	S3_1_C15_C2_1, x0
	isb	sy
	dsb	sy

	/* Clear bit [10] of the EL3 Architectural Feature Trap Register */
	mrs	x0, cptr_el3
	mov	x1, #(1 << 10)
	bic	x0, x0, x1
	msr	cptr_el3, x0
	isb	sy

	/*
	 * Enable VFP / NEON, giving full access to the CP10/CP11 space
	 * through CPACR_EL1. Ideally this would be done by EL1 SW.
	 */
	mrs	x0, cpacr_el1
	orr	x0, x0, #(3 << 20)
	msr	cpacr_el1, x0
	isb	sy

	/*
	 * Enable Adv. SIMD & FP through FPEXC32_EL2. This register is
	 * specific to AArch32 EL2 and lower.
	 */
	mov	x0, #(1 << 30)
	msr	fpexc32_el2, x0
	isb	sy

	/*
	 * Disable Secure Invasive and Non-Invasive Debug through
	 * SDER32_EL3. This should ideally be set by secure AArch32
	 * monitor code.
	 */
	mrs	x0, sder32_el3
	mov	x1, #3
	bic	x0, x0, x1
	msr	sder32_el3, x0
	isb	sy

	/*
	 * Configure debug feature trapping in MDCR_EL3: some debug
	 * features are left permitted, others are set to trap to EL3.
	 */
	mrs	x0, mdcr_el3
	mov	x1, #(1 << 6)
	bic	x0, x0, x1
	mov	x1, #(3 << 9)
	bic	x0, x0, x1
	mov	x1, #(3 << 14)
	bic	x0, x0, x1
	orr	x0, x0, #(1 << 16)
	mov	x1, #(1 << 17)
	bic	x0, x0, x1
	orr	x0, x0, #(3 << 20)
	msr	mdcr_el3, x0
	isb	sy

	/* Enable writes to CPUECTLR_EL1 from EL2 by setting ACTLR_EL3[1] */
	mrs	x0, actlr_el3
	orr	x0, x0, #(1 << 1)
	msr	actlr_el3, x0
	isb	sy

	ret
endfunc a53_aarch64_sysini

/* -------------------------------------------------
 * The CPU Ops reset function for Kryo-3 Silver
 * -------------------------------------------------
 */
func qti_a53_reset_func
	mov	x19, x30
	bl	a53_aarch64_sysini
	mov	x30, x19
	/* Apply the Cortex-A53 reset-time errata and set the SMP bit */
	b	cortex_a53_reset_func
endfunc qti_a53_reset_func

/* ---------------------------------------------
 * This function provides kryo3_silver specific
 * register information for crash reporting.
 * It needs to return with x6 pointing to
 * a list of register names in ascii and
 * x8 - x15 having values of registers to be
 * reported.
 * ---------------------------------------------
 */
.section .rodata.qti_a53_regs, "aS"
qti_a53_regs:  /* The ascii list of register names to be reported */
	.asciz	""

func qti_a53_cpu_reg_dump
	adr	x6, qti_a53_regs
	ret
endfunc qti_a53_cpu_reg_dump

declare_cpu_ops	qti_a53, QTI_A53_MIDR,	\
		qti_a53_reset_func,		\
		cortex_a53_core_pwr_dwn,	\
		cortex_a53_cluster_pwr_dwn
