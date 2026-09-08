/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CPU library for the Qualcomm Oryon-1 core, as used on the Nord SA8797P /
 * "Wildcat" platform.
 *
 * Oryon-1 MIDR: implementer 0x51 (Qualcomm), part 0x001, variant 0x5,
 * revision 0x4 -> 0x515F0014. get_cpu_ops_ptr() matches on the masked
 * implementer+partnum (CPU_IMPL_PN_MASK), so this resolves for any Oryon-1
 * variant/revision.
 *
 * Why this file exists
 * --------------------
 * Oryon-1 needs its own reset/power-down paths: it does not implement the
 * IMP-DEF registers (CPUECTLR_EL1, L2MERRSR_EL1, ...) that generic CPU
 * libraries poke, and accessing them traps at EL3, killing BL31 in the
 * reset-function call before it reaches el3_arch_init_common.
 *
 * Reset-time content:
 *    - ACTLR_EL3[1:0] = 0b11: enable exclusive load/store accesses to
 *      Non-Cacheable memory, required to avoid a Data Abort on NC memory
 *      exclusive access.
 *    - CPTR_EL3.TAM (bit 30) = 0: un-trap AMU register access to EL3, plus
 *      AMCR_EL0.HDBG (bit 10) = 1 (halt AMU counters when the PE is halted
 *      in Debug state) and AMCR_EL0.CG1RZ (bit 17) = 1, plus the 8 auxiliary
 *      AMU counter event-ID assignments (AMEVTYPER1n_EL0, n=0..7) for
 *      cache-refill, bus-access, branch-misprediction and energy events.
 *    HAFGRTR_EL2 un-trapping is not done here: it is gated on SCR_EL3.FGTEN,
 *    which this port only sets later in context_mgmt.c - at reset time
 *    SCR_EL3.FGTEN always reads 0, so there is nothing meaningful to do yet.
 *
 * The publicly documented Oryon-1 erratum, "broken CNTVOFF_EL2" (Linux
 * ARM64_WORKAROUND_QCOM_ORYON_CNTVOFF), is a virtual-counter offset issue
 * handled at EL2 by the hypervisor/KVM, not an EL3 reset concern.
 *
 * Coherency
 * ---------
 * The platform builds with HW_ASSISTED_COHERENCY=1 / USE_COHERENT_MEM=0 and XBL
 * releases the cores already powered and coherent, so the power-down hooks do
 * NO software cache maintenance and do NOT toggle SMP/coherency in software.
 */

#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>
#include <lib/cpus/errata.h>

#if !HW_ASSISTED_COHERENCY
#error "Oryon must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

#if CTX_INCLUDE_AARCH32_REGS
#error "Oryon supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

/* Oryon-1 MIDR (implementer 0x51, part 0x001, variant 0x5, revision 0x4). */
#define ORYON_MIDR	U(0x515F0014)

/* ACTLR_EL3 bitfields used by the reset-time setup below. */
#define ORYON_ACTLR_EL3_EXCL_NC_MEM_SHIFT	U(0)
#define ORYON_ACTLR_EL3_EXCL_NC_MEM_MASK	U(0x3)

/* CPTR_EL3.TAM (Trap AMU) - clearing it allows EL3 access to AMU regs. */
#define ORYON_CPTR_EL3_TAM_BIT			(U(1) << 30)

/* AMCR_EL0 bitfields. */
#define ORYON_AMCR_EL0_HDBG_BIT			(U(1) << 10)
#define ORYON_AMCR_EL0_CG1RZ_BIT			(U(1) << 17)

/* Auxiliary AMU counter event IDs. */
#define ORYON_AMU_EVT_L1_D_CACHE_REFILL		U(0x3)
#define ORYON_AMU_EVT_L2_D_CACHE_REFILL		U(0x17)
#define ORYON_AMU_EVT_BUS_ACCESS			U(0x19)
#define ORYON_AMU_EVT_INST_SPEC			U(0x1B)
#define ORYON_AMU_EVT_NCC_ENERGY_CPU		U(0x2C00)
#define ORYON_AMU_EVT_NCC_ENERGY_CPU_DYN		U(0x2C01)
#define ORYON_AMU_EVT_L1_I_CACHE_REFILL		U(0x1)
#define ORYON_AMU_EVT_BR_MIS_PRED			U(0x10)

/* S3_3_C13_C14_n : AMEVTYPER1n_EL0, the auxiliary AMU event-type registers. */
#define AMEVTYPER10_EL0		S3_3_C13_C14_0
#define AMEVTYPER11_EL0		S3_3_C13_C14_1
#define AMEVTYPER12_EL0		S3_3_C13_C14_2
#define AMEVTYPER13_EL0		S3_3_C13_C14_3
#define AMEVTYPER14_EL0		S3_3_C13_C14_4
#define AMEVTYPER15_EL0		S3_3_C13_C14_5
#define AMEVTYPER16_EL0		S3_3_C13_C14_6
#define AMEVTYPER17_EL0		S3_3_C13_C14_7

	/* ---------------------------------------------------------------
	 * Reset function.
	 * ---------------------------------------------------------------
	 */
cpu_reset_prologue oryon

cpu_reset_func_start oryon
	/*
	 * Enable exclusive load/store accesses to Non-Cacheable memory.
	 * Without this, an exclusive access to NC memory takes a Data Abort.
	 */
	mrs	x0, actlr_el3
	orr	x0, x0, #ORYON_ACTLR_EL3_EXCL_NC_MEM_MASK
	msr	actlr_el3, x0
	isb

#if ENABLE_FEAT_AMU
	/*
	 * Un-trap AMU register access to EL3, and configure the shared AMU
	 * counter-halt-on-debug and CG1RZ behaviour. Only the CPTR_EL3 half is
	 * done here; the CPTR_EL2 half is a lower-EL concern, not a CPU-reset one.
	 */
	mrs	x0, cptr_el3
	bic	x0, x0, #ORYON_CPTR_EL3_TAM_BIT
	msr	cptr_el3, x0

	mrs	x0, AMCR_EL0
	orr	x0, x0, #ORYON_AMCR_EL0_HDBG_BIT
	orr	x0, x0, #ORYON_AMCR_EL0_CG1RZ_BIT
	msr	AMCR_EL0, x0
	isb

	/*
	 * Configure the 8 auxiliary AMU counters (AMEVTYPER10-17_EL0) with
	 * their event IDs.
	 */
	mov	x0, #ORYON_AMU_EVT_L1_D_CACHE_REFILL
	msr	AMEVTYPER10_EL0, x0
	mov	x0, #ORYON_AMU_EVT_L2_D_CACHE_REFILL
	msr	AMEVTYPER11_EL0, x0
	mov	x0, #ORYON_AMU_EVT_BUS_ACCESS
	msr	AMEVTYPER12_EL0, x0
	mov	x0, #ORYON_AMU_EVT_INST_SPEC
	msr	AMEVTYPER13_EL0, x0
	mov	x0, #ORYON_AMU_EVT_NCC_ENERGY_CPU
	msr	AMEVTYPER14_EL0, x0
	mov	x0, #ORYON_AMU_EVT_NCC_ENERGY_CPU_DYN
	msr	AMEVTYPER15_EL0, x0
	mov	x0, #ORYON_AMU_EVT_L1_I_CACHE_REFILL
	msr	AMEVTYPER16_EL0, x0
	mov	x0, #ORYON_AMU_EVT_BR_MIS_PRED
	msr	AMEVTYPER17_EL0, x0
	/* isb included in cpu_reset_func_end below */
#endif /* ENABLE_FEAT_AMU */
cpu_reset_func_end oryon

	/* ---------------------------------------------------------------
	 * void oryon_core_pwr_dwn(void)
	 * Prepare the core for power down. With HW-assisted coherency the
	 * hardware manages cache/coherency, so no software cache maintenance
	 * or SMP-disable is required here. Left as a no-op that returns; PSCI
	 * on this platform uses the native wildcat_pm path for the actual
	 * power-collapse sequencing.
	 * ---------------------------------------------------------------
	 */
func oryon_core_pwr_dwn
	ret
endfunc oryon_core_pwr_dwn

	/* ---------------------------------------------------------------
	 * void oryon_cluster_pwr_dwn(void)
	 * As above: no software cache maintenance under HW-assisted
	 * coherency.
	 * ---------------------------------------------------------------
	 */
func oryon_cluster_pwr_dwn
	ret
endfunc oryon_cluster_pwr_dwn

#if defined(IMAGE_BL31) && CRASH_REPORTING
	/* ---------------------------------------------------------------
	 * Crash-reporting register dump. Oryon exposes no extra IMP-DEF
	 * state we can safely read here (its IMP-DEF CPUECTLR/L2MERRSR
	 * registers trap), so report an empty register list.
	 * ---------------------------------------------------------------
	 */
.section .rodata.oryon_regs, "aS"
oryon_regs:
	.asciz	""

func oryon_cpu_reg_dump
	adr	x6, oryon_regs
	ret
endfunc oryon_cpu_reg_dump
#endif

	/* ---------------------------------------------------------------
	 * Register the Oryon-1 CPU ops. reset_func is the symbol created by
	 * cpu_reset_prologue (oryon_reset_func); two power-down levels
	 * (core, cluster) fill CPU_MAX_PWR_DWN_OPS.
	 * ---------------------------------------------------------------
	 */
declare_cpu_ops oryon, ORYON_MIDR, \
	oryon_reset_func, \
	oryon_core_pwr_dwn, \
	oryon_cluster_pwr_dwn
