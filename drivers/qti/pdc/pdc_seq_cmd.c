/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/qti/pdc/pdc_seq.h>

#include "pdc_regs.h"

static uint16_t pdc_seq_copy_cmds(uint8_t *pdc_base, uint16_t start_addr,
				  uint8_t *cmds, uint16_t length)
{
	const uint16_t cmds_per_reg = 4U;
	uint32_t max_cmds;
	uint32_t mask = 0U;
	uint32_t value = 0U;
	uint32_t cmd_shift = 0U;
	uint16_t cmds_copied = 0U;
	uint16_t i;
	uint16_t addr = start_addr;

	max_cmds = PDC_PARAM_SEQ_CMD_WORDS(pdc_base) * cmds_per_reg;

	if ((start_addr >= max_cmds) ||
	    ((uint32_t)(start_addr + length) > max_cmds)) {
		return 0U;
	}

	for (i = 0U; i < length; i++, addr++) {
		switch (addr % cmds_per_reg) {
		case 0:
			if (cmds_copied != 0U) {
				PDC_SEQ_MEM_RMW((addr - 1U) / cmds_per_reg,
						mask, value);
				value = 0U;
			}
			cmd_shift = 0U;
			break;
		case 1:
			cmd_shift = 8U;
			break;
		case 2:
			cmd_shift = 16U;
			break;
		case 3:
			cmd_shift = 24U;
			break;
		default:
			break;
		}

		mask  |= (0xFFU << cmd_shift);
		value |= ((uint32_t)cmds[i] << cmd_shift);
		cmds_copied++;
	}

	PDC_SEQ_MEM_RMW((addr - 1U) / cmds_per_reg, mask, value);

	return cmds_copied;
}

enum pdc_seq_result pdc_seq_target_init(struct pdc_seq *seq)
{
	uint32_t i;
	uint32_t j;
	uint32_t copied;
	struct pdc_seq_mode *curr;
	const struct pdc_seq_mode *ref = NULL;

	for (i = 0U; i < seq->mode_count; i++) {
		curr = &seq->modes[i];

		if (curr->length == 0U) {
			return PDC_SEQ_INVALID_PARAM;
		}

		for (j = 0U; j < i; j++) {
			ref = &seq->modes[j];
			if (ref->cmds == curr->cmds) {
				break;
			}
		}

		if (j == i) {
			copied = pdc_seq_copy_cmds(seq->addr,
						   seq->copied_cmds,
						   curr->cmds,
						   curr->length);
			if (copied == 0U) {
				return PDC_SEQ_NO_MEM;
			}

			curr->start_addr = (int16_t)seq->copied_cmds;
			seq->copied_cmds += (uint16_t)copied;
		} else {
			curr->start_addr = ref->start_addr;
		}
	}

	return PDC_SEQ_SUCCESS;
}
