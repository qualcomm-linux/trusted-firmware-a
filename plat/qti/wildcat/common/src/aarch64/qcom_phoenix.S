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
#include <qcom_phoenix.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include <plat_cpu_helpers.S>

/*
 * Hardware handled coherency
 * #if HW_ASSISTED_COHERENCY == 0
 * #error "phoenix must be compiled with HW_ASSISTED_COHERENCY enabled"
 * #endif
 */


func phoenix_reset
/* Enable Exclusive load/store accesses to Non Cacheable Memory
 * This is required to prevent a Data Abort and allow NC memory
 * read/write access
 * Set Bit[1] in ACTLR_EL3
 */
en_ld_str_nc_mem:
	ENABLE_EX_LD_STR_TO_NC_MEM
	isb

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

/* =============== Workaround Start ==================== */
wa_start:
	CHECK_MIDR_AND_JUMP MIDR_VERSION_R3P0, wa_r3p0_start // PAKALA_LARGE_V1
	CHECK_MIDR_AND_JUMP MIDR_VERSION_R4P0, wa_r4p0_start // PAKALA_MED_V1
	CHECK_MIDR_AND_JUMP MIDR_VERSION_R3P4, wa_r3p4_start // PAKALA_LARGE_R2
	CHECK_MIDR_AND_JUMP MIDR_VERSION_R4P4, wa_r4p4_start // PAKALA_MED_R2
	b partnum_specific_settings

wa_r3p0_start:
wa_r4p0_start:
wa_r3p4_start:
wa_r4p4_start:
	bl qcce_326
	b partnum_specific_settings

/* ============= Workaround End ======================== */

/*==============Power/Perf specific settings start===========*/

partnum_specific_settings:
	CHECK_MIDR_PARTNUM_AND_JUMP MIDR_PARTNUM_PERF, perf_settings_start
	CHECK_MIDR_PARTNUM_AND_JUMP MIDR_PARTNUM_POWER, power_settings_start
	b end_of_function

perf_settings_start:
	b end_of_function

power_settings_start:
	b end_of_function

/*=============Power/Perf specific settings end================*/
	dsb	sy
	isb
	ret
endfunc phoenix_reset

end_of_function:
	dsb sy
	isb
	ret

/*
 * =====================================================
 * Errata	: QCCE-326
 * Ticket No	: --
 * Applicable to: Pakala V1, R2 - Medium and Large cores
 * =====================================================
 */
qcce_326:
	ENABLE_PIPED_DEV
	isb
	ret

func phoenix_core_pwr_dwn
	dsb	sy
	isb
	ret
endfunc phoenix_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Phoenix. Must follow AAPCS.
 */
func phoenix_errata_report
	ret
endfunc phoenix_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides phoenix specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.phoenix_regs, "aS"
phoenix_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func phoenix_cpu_reg_dump
	adr	x6, phoenix_regs
	mrs	x8, PHOENIX_CPUECTLR_EL1
	ret
endfunc phoenix_cpu_reg_dump

declare_cpu_ops phoenix, PHOENIX_MIDR, \
	phoenix_reset, \
	phoenix_core_pwr_dwn

declare_cpu_ops phoenix, PHOENIX_POWER_MIDR, \
	phoenix_reset, \
	phoenix_core_pwr_dwn
declare_cpu_ops phoenix, PHOENIX_PERF_MIDR, \
	phoenix_reset, \
	phoenix_core_pwr_dwn
