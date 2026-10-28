/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Bruin 1.1 Cluster/DSU Hardware Initialization.
 * Constraint:  Must be called BEFORE MMU is enabled.
 *
 * This function performs the raw DSU register writes only. The once-per-
 * cluster guard and spinlock are handled by the C wrapper
 * plat_cluster_init_wrapper() in qti_cpuss_wrapper.c, which calls this function.
 *
 * System registers (implementation-defined, Qualcomm/ARM DSU extensions):
 *   ERRSELR_EL1      S3_0_C5_C3_1   Error Record Selector
 *   ERRxCTLR_EL1     S3_0_C5_C4_1   Error Record Control
 *   CLUSTERECTLR_EL1 S3_0_C15_C3_4  Cluster Extended Control (DSU)
 */

#include <arch.h>
#include <asm_macros.S>
#include <lib/cpus/aarch64/cpu_macros.S>

	/* System register encodings — implementation-defined DSU registers */
#define ERRSELR_EL1		S3_0_C5_C3_1
#define ERRxCTLR_EL1		S3_0_C5_C4_1
#define CLUSTERECTLR_EL1	S3_0_C15_C3_4

	/* ERRxCTLR_EL1 bit fields */
#define ERRxCTLR_ED		(U(1) << 0)	/* Error Detection enable */
#define ERRxCTLR_CFI		(U(1) << 2)	/* Correctable Fault Interrupt */
#define ERRxCTLR_UE		(U(1) << 3)	/* Uncorrectable Error enable */
#define ERRxCTLR_FI		(U(1) << 8)	/* Fault Interrupt enable */
#define ERRxCTLR_ALL		(ERRxCTLR_ED | ERRxCTLR_CFI | \
				 ERRxCTLR_UE | ERRxCTLR_FI)

	/* CLUSTERECTLR_EL1 bit fields */
#define CLUSTERECTLR_NC_WRITE_EN	(U(1) << 0)	/* NC write interleaving */
#define CLUSTERECTLR_DIS_CLEANEVICT	(U(1) << 3)	/* Disable clean evict to LLCC */
#define CLUSTERECTLR_UNIQUECLEAN_EN	(U(1) << 14)	/* UniqueClean eviction to LLCC */

	.globl	plat_cluster_init

/* ---------------------------------------------------------------------------
 * void plat_cluster_init(void)
 * Raw DSU register writes for Bruin 1.1 cluster initialization.
 * Called by plat_cluster_init_wrapper() in qti_cpuss_wrapper.c under a spinlock.
 * Clobbers: x0
 * ---------------------------------------------------------------------------
 */
func plat_cluster_init

	/*
	 * Enable L3 ECC Error Record Interrupts
	 *
	 * Select L3 error record (record 1) via ERRSELR_EL1, then configure
	 * ERRxCTLR_EL1 to enable ECC error detection and interrupt generation.
	 */
	mov	x0, #1
	msr	ERRSELR_EL1, x0
	isb
	mov	x0, #ERRxCTLR_ALL
	msr	ERRxCTLR_EL1, x0
	isb

	/*
	 * Disable UniqueClean Evictions to LLCC
	 *
	 * CLUSTERECTLR_EL1[14] = 0
	 *
	 * Bruin FCM has no LLCC (Last Level Cache Controller), so UniqueClean
	 * evictions with data must be disabled.
	 */
	sysreg_bit_clear CLUSTERECTLR_EL1, CLUSTERECTLR_UNIQUECLEAN_EN
	isb

	/*
	 * Enable NC Write Interleaving
	 *
	 * CLUSTERECTLR_EL1[0] = 1
	 *
	 * Enables interleaving of Non-Cacheable (NC) write transactions across
	 * both ACE ports. Improves NC write throughput on Bruin 1.1.
	 */
	sysreg_bit_set CLUSTERECTLR_EL1, CLUSTERECTLR_NC_WRITE_EN
	isb

	/*
	 * Disable Clean Evict to External (LLCC)
	 *
	 * CLUSTERECTLR_EL1[3] = 1
	 *
	 * Disables sending evict transactions for clean cache lines from L3
	 * to LLCC. Required because Bruin FCM has no LLCC.
	 */
	sysreg_bit_set CLUSTERECTLR_EL1, CLUSTERECTLR_DIS_CLEANEVICT
	isb

	dsb	sy
	ret

endfunc plat_cluster_init