/**********************************************************************
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *********************************************************************/

#ifndef QTI_TLB_H
#define QTI_TLB_H

#include <stdint.h>
#include "bl31qtilib_defs.h"

/**
 * Different tlb control/invalidate operation.
 * This is a generic API which will support multiple possible tlb operations
 * listed in tlb_op_t enum definition
 *
 * @param op     TLB operation request.
 * @param arg    Optional argument if needed by tlb_op_t operation.
 *               Invalidate by address or asid or vmid expect values
 *               for them. Default value can be 0x0.
 *
 * @returns  E_SUCCESS on success, Negative value otherwise
 */
typedef enum tlb_op_s {
	/** AARCH64 Supported TLB Operations */
	/* Invalidate all EL1 TLB. */
	TLB_OP_INVAL_ALL_EL1 = 0,
	/* Invalidate all EL1 TLB Inner Shared. */
	TLB_OP_INVAL_ALL_EL1_IS,
	/** Invalidate for the supplied ASID, Inner Shareable.
	 * Expect additional argument for ASID.
	 */
	TLB_OP_INVAL_BY_ASID_EL1_IS,
	/** Invalidate for the supplied address. Inner Shareable.
	 * Expect additional argument for Address.
	 */
	TLB_OP_INVAL_BY_MVA_EL1_IS,

	/** AARCH32 Supported TLB Operations */
	TLB_OP_INVAL_ALL = TLB_OP_INVAL_ALL_EL1, /* Invalidate unified TLB. */
	/** Invalidate entire TLB Inner Shareable. */
	TLB_OP_INVAL_ALL_IS = TLB_OP_INVAL_ALL_EL1_IS,
	/** Invalidate unified TLB by ASID match, Inner Shareable.
	 * Expect additional argument for ASID.
	 */
	TLB_OP_INVAL_BY_ASID_IS = TLB_OP_INVAL_BY_ASID_EL1_IS,
	/** Invalidate unified TLB entry by MVA, Inner Shareable.
	 * Expect additional argument for Address.
	 */
	TLB_OP_INVAL_BY_MVA_IS = TLB_OP_INVAL_BY_MVA_EL1_IS,
	/* Invalidate instruction TLB */
	TLB_OP_INVAL_ITLB_ALL,
	/* Invalidate Data TLB */
	TLB_OP_INVAL_DTLB_ALL,

	TLB_OP_ID_MAX,
	TLB_OP_INVALID = 0x7FFFFFFFU,
} tlb_op_t;

/**
 * Perform TLB Invalidate commands
 *
 * @param[in]   op		The TLB operation to perform
 * @param[in]   arg		Parameter for the TLB operation
 *
 * @return      0 for success, negative error code if fails
 */
int qti_tlb_ctrl(tlb_op_t op, uintptr_t arg);

/* TF-A does not have an implementation for tlbi aside1is */
static inline void tlbiaside1is(u_register_t arg)
{
#ifndef BUILD_OFFTARGET_UNITTEST
	__asm volatile("tlbi    aside1is, %0" : : "r"(arg));
#endif
}

/* TF-A does not have an implementation for tlbi vaae1 */
static inline void tlbivaae1(u_register_t arg)
{
#ifndef BUILD_OFFTARGET_UNITTEST
	__asm volatile("tlbi    vaae1, %0" : : "r"(arg));
#endif
}

#endif /* QTI_TLB_H */
