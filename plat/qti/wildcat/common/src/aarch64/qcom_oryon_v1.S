/*
* Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cpu_macros.S>

#include <plat_cpu_helpers.S>
#include <plat_macros.S>
#include <qcom_oryon_v1.h>


/*
 * Hardware handled coherency
 * #if HW_ASSISTED_COHERENCY == 0
 * #error "oryon_v1 must be compiled with HW_ASSISTED_COHERENCY enabled"
 * #endif
 */

cpu_reset_prologue oryon_v1

cpu_reset_func_start oryon_v1
/* Enable Exclusive load/store accesses to Non Cacheable Memory
 * This is required to prevent a Data Abort and allow NC memory
 * read/write access
 * Set Bit[1] in ACTLR_EL3
 */
en_ld_str_nc_mem:
	ENABLE_EX_LD_STR_TO_NC_MEM
	isb

#if ENABLE_FEAT_AMU
amu_config_start:
	//Halt AMU counters when PE is halted in Debug state.
	SPR_OP AMCR_EL0, #1, AMCR_HDBG_SHFT, #1

	//Configure auxiliary AMU counters with event ids
	CONFIG_AUX_AMU_CNTR AMEVTYPER1n_EL0(14, 0), L1_D_CACHE_REFILL_EVT
	CONFIG_AUX_AMU_CNTR AMEVTYPER1n_EL0(14, 1), L2_D_CACHE_REFILL_EVT
	CONFIG_AUX_AMU_CNTR AMEVTYPER1n_EL0(14, 2), BUS_ACCESS_EVT
	CONFIG_AUX_AMU_CNTR AMEVTYPER1n_EL0(14, 3), INST_SPEC_EVT
	CONFIG_AUX_AMU_CNTR AMEVTYPER1n_EL0(14, 4), NCC_ENERGY_CPU_EVT
	CONFIG_AUX_AMU_CNTR AMEVTYPER1n_EL0(14, 5), NCC_ENERGY_CPU_DYN_EVT
	CONFIG_AUX_AMU_CNTR AMEVTYPER1n_EL0(14, 6), L1_I_CACHE_REFILL
	CONFIG_AUX_AMU_CNTR AMEVTYPER1n_EL0(14, 7), BR_MIS_PRED
	isb

en_amu:
	// Allow access to Activity Monitor Registers
	ENABLE_CPTR_AMU_REG
	ENABLE_HAFGRTR_AMU_REG
	DISABLE_AMCR_AUX_AMU_REG
	DISABLE_AMUSERENR_AMU_REG

armv9_amu_enable:
	MOV X0, 0xFFFF
	MSR AMCNTENSET0_EL0, X0
	MSR AMCNTENSET1_EL0, X0
	ISB
#endif

/*==============Power/Perf specific settings start===========*/

partnum_specific_settings:
	CHECK_MIDR_PARTNUM_AND_JUMP MIDR_PARTNUM_PERF, perf_settings_start
	CHECK_MIDR_PARTNUM_AND_JUMP MIDR_PARTNUM_POWER, power_settings_start

/*=============Power/Perf specific settings end================*/
	dsb	sy
	isb
	ret
cpu_reset_func_end oryon_v1

perf_settings_start:
	b end_of_function

power_settings_start:
	b end_of_function

end_of_function:
	dsb sy
	isb
	ret

func oryon_v1_core_pwr_dwn
	dsb	sy
	isb
	ret
endfunc oryon_v1_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides oryon_v1 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.oryon_v1_regs, "aS"
oryon_v1_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func oryon_v1_cpu_reg_dump
	adr	x6, oryon_v1_regs
	mrs	x8, ORYON_V1_CPUECTLR_EL1
	ret
endfunc oryon_v1_cpu_reg_dump

declare_cpu_ops oryon_v1, ORYON_V1_MIDR, \
	oryon_v1_reset_func, \
	oryon_v1_core_pwr_dwn

declare_cpu_ops oryon_v1, ORYON_V1_POWER_MIDR, \
	oryon_v1_reset_func, \
	oryon_v1_core_pwr_dwn
declare_cpu_ops oryon_v1, ORYON_V1_PERF_MIDR, \
	oryon_v1_reset_func, \
	oryon_v1_core_pwr_dwn
