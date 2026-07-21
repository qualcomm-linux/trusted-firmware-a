/**********************************************************************
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *********************************************************************/
#include <arch_helpers.h>

#include "qti_errno.h"
#include "qti_tlb.h"

int qti_tlb_ctrl(tlb_op_t op, uintptr_t arg)
{
	unsigned int el = get_current_el();
	int ret = 0;

	if (op == TLB_OP_INVAL_ALL_EL1) {
		if (el == MODE_EL3) {
			tlbialle1();
		} else {
			ret = ARMLIB_E_TLB_CTRL_ALLEL1_NOT_ALLOWED;
		}
	} else if (op == TLB_OP_INVAL_ALL_EL1_IS) {
		if (el == MODE_EL3) {
			tlbialle1is();
		} else {
			ret = ARMLIB_E_TLB_CTRL_ALLEL1IS_NOT_ALLOWED;
		}
	} else if (op == TLB_OP_INVAL_BY_ASID_EL1_IS) {
		/* Only 16-bit ASID is supported */
		arg = (arg & 0xFFFFULL) << 48;
		tlbiaside1is(arg);

	} else if (op == TLB_OP_INVAL_BY_MVA_EL1_IS) {
		/* TODO: Will it be "tlbi VAE1IS"?, arm v7 execute TLBIMVAIS here. */
		/* 64-bit MVA address is - RSVD, Bits [63:44] & VA[55:12], Bits [43:0] */
		arg = (arg & 0xFFFFFFFFFFFULL);
		tlbivaae1(arg);
	} else {
		ret = ARMLIB_E_TLB_CTRL_OP_NOT_SUPPORTED;
	}

	dsbsy();
	isb();

	return ret;
}
