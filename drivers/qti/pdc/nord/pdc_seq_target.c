/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <drivers/qti/pdc/pdc_seq.h>

#include "pdc_regs.h"

enum pdc_seq_result pdc_seq_target_init(struct pdc_seq *seq)
{
	size_t i;

	for (i = 0U; i < seq->mode_count; i++) {
		if (seq->modes[i].length != 0U) {
			return PDC_SEQ_INVALID_PARAM;
		}
	}

	if (seq->mode_count > 0U) {
		PDC_SEQ_BRANCH_MASK_WRITE(seq->rsc_addr,
					  seq->modes[0].branch_mask);
	}

	return PDC_SEQ_SUCCESS;
}
