/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include <drivers/qti/pdc/pdc_seq.h>
#include "pdc_regs.h"

#include <platform_def.h>

extern struct pdc_seq *g_pdc_seqs;
extern uint32_t g_pdc_seq_count;

static enum pdc_seq_result pdc_seq_internal_cfg(struct pdc_seq *seq)
{
	uint32_t i;
	const struct pdc_seq_cfg *cfg = seq->cfg;

	for (i = 0U; i < cfg->br_count; i++) {
		PDC_SEQ_BR_ADDR_WRITE(seq->addr, i, cfg->br_addr[i]);
	}

	for (i = 0U; i < cfg->delay_count; i++) {
		PDC_SEQ_DELAY_WRITE(seq->addr, i, cfg->delay[i]);
	}

	if (PDC_PARAM_PROFILING_UNIT(seq->addr) != PDC_TS_COUNT) {
		return PDC_SEQ_INVALID_PARAM;
	}

	return PDC_SEQ_SUCCESS;
}

static enum pdc_seq_result pdc_seq_init(struct pdc_seq *seq)
{
	enum pdc_seq_result result;

	if ((seq == NULL) || (seq->cfg == NULL) ||
	    (seq->modes == NULL) || (seq->offset == 0U)) {
		return PDC_SEQ_INVALID_HANDLE;
	}

	seq->addr = (uint8_t *)(QTI_AOSS_BASE + seq->offset);
	seq->rsc_addr = (uint8_t *)(QTI_APSS_HM_BASE + seq->rsc_offset);

	result = pdc_seq_internal_cfg(seq);
	if (result != PDC_SEQ_SUCCESS) {
		return result;
	}

	return pdc_seq_target_init(seq);
}

void pdc_seq_sys_init(void)
{
	uint32_t i;
	struct pdc_seq *seq;

	for (i = 0U; i < g_pdc_seq_count; i++) {
		seq = &g_pdc_seqs[i];

		if (pdc_seq_init(seq) != PDC_SEQ_SUCCESS) {
			ERROR("PDC: sequence %u init failed\n", i);
			return;
		}

		PDC_ENABLE_PDC_RMW(seq->addr, 1U);
	}
}
