/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Changes from Qualcomm Technologies, Inc. are provided under the following
 * license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_phoenix.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include <sysini.h>

/*
 * Hardware handled coherency
 * #if HW_ASSISTED_COHERENCY == 0
 * #error "cortex_phoenix must be compiled with HW_ASSISTED_COHERENCY enabled"
 * #endif
 */

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_phoenix_reset
	mov	x19, x30
	mov	x0, #SYSINI_POWER_UP
	mov	x1, #0
	bl	phoenix_gen1_aarch64_sysini
	ret	x19
endfunc cortex_phoenix_reset

func cortex_phoenix_core_pwr_dwn
	mov	x0, #SYSINI_POWER_DOWN
	mov	x1, #0
	b	phoenix_gen1_aarch64_sysini
endfunc cortex_phoenix_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex Phoenix. Must follow AAPCS.
 */
func cortex_phoenix_errata_report
	ret
endfunc cortex_phoenix_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cortex_phoenix specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_phoenix_regs, "aS"
cortex_phoenix_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_phoenix_cpu_reg_dump
	adr	x6, cortex_phoenix_regs
	mrs	x8, CORTEX_PHOENIX_CPUECTLR_EL1
	ret
endfunc cortex_phoenix_cpu_reg_dump

declare_cpu_ops cortex_phoenix, CORTEX_PHOENIX_POWER_MIDR, \
	cortex_phoenix_reset, \
	cortex_phoenix_core_pwr_dwn
declare_cpu_ops cortex_phoenix, CORTEX_PHOENIX_PERF_MIDR, \
	cortex_phoenix_reset, \
	cortex_phoenix_core_pwr_dwn
