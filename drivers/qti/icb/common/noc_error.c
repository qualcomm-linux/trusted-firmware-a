/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/spinlock.h>
#include <platform_def.h>
#include <qti_interrupt_svc.h>

#include <drivers/qti/icb/icb_error.h>

#include "noc_error.h"
#include "noc_error_platform.h"
#include "noc_error_target.h"

#define NOC_OUT8(addr, data)   (*((volatile uint8_t *)(addr)) = ((uint8_t)(data)))
#define NOC_OUT16(addr, data)  (*((volatile uint16_t *)(addr)) = ((uint16_t)(data)))
#define NOC_OUT32(addr, data)  (*((volatile uint32_t *)(addr)) = ((uint32_t)(data)))

#define NOC_OUTM32(addr, mask, data) NOC_OUT32(addr, (NOC_IN32(addr) & ~(mask)) | (data))

#define NOC_IN8(addr)   ((uint8_t)(*((volatile uint8_t *)(addr))))
#define NOC_IN16(addr)  ((uint16_t)(*((volatile uint16_t *)(addr))))
#define NOC_IN32(addr)  ((uint32_t)(*((volatile uint32_t *)(addr))))

#define NOC_REG_ADDR(base, offset)   (((uint8_t *)(base)) + (offset))

#define REGISTER_VALID(offs) ((offs) != REGISTER_NOT_APPLICABLE)

static spinlock_t isr_log_sync_lock;

extern struct nocerr_config_info nocerr_config_info;
extern struct nocerr_config_info_oem nocerr_config_info_oem;
static struct nocerr_propdata *nocerr_propdata;
static struct nocerr_propdata_oem *nocerr_propdata_oem;
static struct nocerr_info *noc_info_list;
static struct nocerr_info_oem *noc_info_oem_list;

struct nocerr_propdata *qti_noc_error_platform_get_propdata(void)
{
	struct nocerr_propdata *noc_propdata_ptr = NULL;

	if (nocerr_config_info.num_configs > 0) {
		noc_propdata_ptr = &nocerr_config_info.configs[0];
	}

	return noc_propdata_ptr;
}

struct nocerr_propdata_oem *qti_noc_error_platform_get_propdata_oem(void)
{
	struct nocerr_propdata_oem *noc_propdata_oem_ptr = NULL;

	if (nocerr_config_info_oem.num_configs > 0) {
		noc_propdata_oem_ptr = &nocerr_config_info_oem.configs[0];
	}

	return noc_propdata_oem_ptr;
}

/*
 * Has the NoC at @idx already been registered? NO_INTERRUPT entries are
 * treated as "already registered" so that callers skip them.
 */
static bool is_qti_noc_error_isr_registered(uint32_t idx)
{
	uintptr_t intr_vector = noc_info_list[idx].intr_vector;

	if (intr_vector == NO_INTERRUPT) {
		return true;
	}

	for (uint32_t noc_idx = 0;
	     noc_idx < idx && noc_idx < nocerr_propdata->len;
	     noc_idx++) {
		if (noc_info_list[noc_idx].intr_vector == intr_vector) {
			return true;
		}
	}

	return false;
}

static void qti_noc_error_log_obs(struct nocerr_info *noc_info)
{
	if (REGISTER_VALID(noc_info->hw->errlog0_low))
		noc_info->syndrome.ERRLOG0_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog0_low));

	if (REGISTER_VALID(noc_info->hw->errlog0_high))
		noc_info->syndrome.ERRLOG0_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog0_high));

	if (REGISTER_VALID(noc_info->hw->errlog1_low))
		noc_info->syndrome.ERRLOG1_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog1_low));

	if (REGISTER_VALID(noc_info->hw->errlog1_high))
		noc_info->syndrome.ERRLOG1_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog1_high));

	if (REGISTER_VALID(noc_info->hw->errlog2_low))
		noc_info->syndrome.ERRLOG2_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog2_low));

	if (REGISTER_VALID(noc_info->hw->errlog2_high))
		noc_info->syndrome.ERRLOG2_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog2_high));

	if (REGISTER_VALID(noc_info->hw->errlog3_low))
		noc_info->syndrome.ERRLOG3_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog3_low));

	if (REGISTER_VALID(noc_info->hw->errlog3_high))
		noc_info->syndrome.ERRLOG3_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog3_high));

	if (REGISTER_VALID(noc_info->hw->errlog2_1_low))
		noc_info->syndrome.ERRLOG2_1_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog2_1_low));

	if (REGISTER_VALID(noc_info->hw->errlog2_1_high))
		noc_info->syndrome.ERRLOG2_1_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog2_1_high));

	if (REGISTER_VALID(noc_info->hw->errlog4_3_low))
		noc_info->syndrome.ERRLOG4_3_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog4_3_low));

	if (REGISTER_VALID(noc_info->hw->errlog4_3_high))
		noc_info->syndrome.ERRLOG4_3_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog4_3_high));

	if (REGISTER_VALID(noc_info->hw->errlog6_5_low))
		noc_info->syndrome.ERRLOG6_5_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog6_5_low));

	if (REGISTER_VALID(noc_info->hw->errlog6_5_high))
		noc_info->syndrome.ERRLOG6_5_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog6_5_high));

	if (REGISTER_VALID(noc_info->hw->errlog8_high))
		noc_info->syndrome.ERRLOG8_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
					      noc_info->hw->errlog8_high));

	ERROR("(%x %s %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x)\n",
	      NOC_ERR_FATAL_SYNDROME_REG, noc_info->name,
	      noc_info->syndrome.ERRLOG0_LOW,
	      noc_info->syndrome.ERRLOG0_HIGH,
	      noc_info->syndrome.ERRLOG1_LOW,
	      noc_info->syndrome.ERRLOG1_HIGH,
	      noc_info->syndrome.ERRLOG2_LOW,
	      noc_info->syndrome.ERRLOG2_HIGH,
	      noc_info->syndrome.ERRLOG3_LOW,
	      noc_info->syndrome.ERRLOG3_HIGH,
	      noc_info->syndrome.ERRLOG2_1_LOW,
	      noc_info->syndrome.ERRLOG2_1_HIGH,
	      noc_info->syndrome.ERRLOG4_3_LOW,
	      noc_info->syndrome.ERRLOG4_3_HIGH,
	      noc_info->syndrome.ERRLOG6_5_LOW,
	      noc_info->syndrome.ERRLOG6_5_HIGH,
	      noc_info->syndrome.ERRLOG8_HIGH);
}

static void qti_noc_error_log_msi(struct nocerr_info *noc_info, uint32_t idx)
{
	if (REGISTER_VALID(noc_info->msi_info->msi_hw[idx]->msienc_errlog0_low))
		noc_info->syndrome.msis[idx].MSIENC_ERRLOG0_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->msi_info->msi_base_addrs[idx],
					      noc_info->msi_info->msi_hw[idx]->msienc_errlog0_low));

	if (REGISTER_VALID(noc_info->msi_info->msi_hw[idx]->msienc_errlog0_high))
		noc_info->syndrome.msis[idx].MSIENC_ERRLOG0_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->msi_info->msi_base_addrs[idx],
					     noc_info->msi_info->msi_hw[idx]->msienc_errlog0_high));

	if (REGISTER_VALID(noc_info->msi_info->msi_hw[idx]->msienc_errlog1_low))
		noc_info->syndrome.msis[idx].MSIENC_ERRLOG1_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->msi_info->msi_base_addrs[idx],
					      noc_info->msi_info->msi_hw[idx]->msienc_errlog1_low));

	if (REGISTER_VALID(noc_info->msi_info->msi_hw[idx]->msienc_errlog1_high))
		noc_info->syndrome.msis[idx].MSIENC_ERRLOG1_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->msi_info->msi_base_addrs[idx],
					     noc_info->msi_info->msi_hw[idx]->msienc_errlog1_high));

	if (REGISTER_VALID(noc_info->msi_info->msi_hw[idx]->msienc_errlog2_low))
		noc_info->syndrome.msis[idx].MSIENC_ERRLOG2_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->msi_info->msi_base_addrs[idx],
					      noc_info->msi_info->msi_hw[idx]->msienc_errlog2_low));

	if (REGISTER_VALID(noc_info->msi_info->msi_hw[idx]->msienc_errlog2_high))
		noc_info->syndrome.msis[idx].MSIENC_ERRLOG2_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->msi_info->msi_base_addrs[idx],
					     noc_info->msi_info->msi_hw[idx]->msienc_errlog2_high));

	ERROR("(%x %s %x %x %x %x %x %x %x)\n", NOC_FAULT_NAME_MSI,
	      noc_info->name, idx,
	      noc_info->syndrome.msis[idx].MSIENC_ERRLOG0_LOW,
	      noc_info->syndrome.msis[idx].MSIENC_ERRLOG0_HIGH,
	      noc_info->syndrome.msis[idx].MSIENC_ERRLOG1_LOW,
	      noc_info->syndrome.msis[idx].MSIENC_ERRLOG1_HIGH,
	      noc_info->syndrome.msis[idx].MSIENC_ERRLOG2_LOW,
	      noc_info->syndrome.msis[idx].MSIENC_ERRLOG2_HIGH);
}

static bool qti_noc_error_scan_faultin(uint32_t num_sbms,
				       struct nocerr_sbm_syndrome *sbms,
				       struct nocerr_sbm_info_oem *obs_mask)
{
	uint32_t fault_mask = 0;

	for (uint32_t idx = 0; idx < num_sbms; idx++) {
		fault_mask |= sbms[idx].FAULTINSTATUS0_LOW &
			      ~obs_mask[idx].faultin_en0_low;
		fault_mask |= sbms[idx].FAULTINSTATUS0_HIGH &
			      ~obs_mask[idx].faultin_en0_high;
		fault_mask |= sbms[idx].FAULTINSTATUS1_LOW &
			      ~obs_mask[idx].faultin_en1_low;
		fault_mask |= sbms[idx].FAULTINSTATUS1_HIGH &
			      ~obs_mask[idx].faultin_en1_high;
		fault_mask |= sbms[idx].FAULTINSTATUS2_LOW &
			      ~obs_mask[idx].faultin_en2_low;
		fault_mask |= sbms[idx].FAULTINSTATUS2_HIGH &
			      ~obs_mask[idx].faultin_en2_high;
	}

	return fault_mask != 0;
}

static bool errcode_matches_filter(struct nocerr_filter *filter, uint32_t errcode)
{
	for (uint32_t i = 0; i < filter->num_errcodes; i++) {
		if (errcode == filter->errcodes[i])
			return true;
	}
	return false;
}

/*
 * Match the captured syndrome against the OEM filter list and return whether
 * the fault should still be treated as fatal.
 */
static bool qti_noc_error_handle_filter(struct nocerr_info *noc_info,
					struct nocerr_info_oem *noc_info_oem,
					bool obs_err_valid,
					bool *delay_crash)
{
	bool is_fatal_allowed = true, is_fault = true;

	if (nocerr_propdata == NULL ||
	    nocerr_propdata_oem == NULL ||
	    noc_info == NULL ||
	    noc_info_oem == NULL ||
	    delay_crash == NULL)
		return true;

	*delay_crash = false;

	if (nocerr_propdata->filters != NULL &&
	    nocerr_propdata->num_filters != 0) {
		/* Check for non-obs faults, these are always fatal. */
		is_fault = qti_noc_error_scan_faultin(noc_info->num_sbms,
						      noc_info->syndrome.sbms,
						      noc_info_oem->obs_mask);

		/* Now process obs faults */
		if (obs_err_valid) {
			bool matched = false;
			/* ERRLOG1_HIGH masks out the MID field. */
			uint32_t extid = noc_info->syndrome.ERRLOG1_HIGH & 0xFFFFFF00;
			uint32_t errcode = (noc_info->syndrome.ERRLOG0_LOW >> 8) & 0x7;
			struct nocerr_filter *filter = NULL;
			struct nocerr_filter_oem *oem_filter = NULL;

			for (uint32_t fidx = 0;
			     fidx < nocerr_propdata->num_filters;
			     fidx++) {
				filter = &nocerr_propdata->filters[fidx];
				oem_filter = &nocerr_propdata_oem->filters[fidx];

				for (uint32_t extidx = 0;
				     extidx < filter->num_extids;
				     extidx++) {
					if (extid == filter->extids[extidx]) {
						matched = true;
						break;
					}
				}

				if (matched) {
					if (oem_filter->enable && filter->non_fatal &&
					    errcode_matches_filter(filter, errcode))
						is_fatal_allowed = false;

					if (is_fatal_allowed)
						*delay_crash = filter->delay_fatal &&
							       oem_filter->delay_fatal;

					break;
				}
			}
		}
	}

	return is_fatal_allowed || is_fault;
}

static void qti_noc_error_log_safety(struct nocerr_info *noc_info,
				     uint32_t idx)
{
	if (REGISTER_VALID(noc_info->sfty_ctl_hw[idx]->cflta_low))
		noc_info->syndrome.sfty_ctl[idx].CFLTA_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->sfty_ctl_addrs[idx],
					      noc_info->sfty_ctl_hw[idx]->cflta_low));

	if (REGISTER_VALID(noc_info->sfty_ctl_hw[idx]->cflta_high))
		noc_info->syndrome.sfty_ctl[idx].CFLTA_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->sfty_ctl_addrs[idx],
					      noc_info->sfty_ctl_hw[idx]->cflta_high));

	if (REGISTER_VALID(noc_info->sfty_ctl_hw[idx]->uflta_low))
		noc_info->syndrome.sfty_ctl[idx].UFLTA_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->sfty_ctl_addrs[idx],
					      noc_info->sfty_ctl_hw[idx]->uflta_low));

	if (REGISTER_VALID(noc_info->sfty_ctl_hw[idx]->uflta_high))
		noc_info->syndrome.sfty_ctl[idx].UFLTA_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->sfty_ctl_addrs[idx],
					      noc_info->sfty_ctl_hw[idx]->uflta_high));

	if (REGISTER_VALID(noc_info->sfty_ctl_hw[idx]->cfltb_low))
		noc_info->syndrome.sfty_ctl[idx].CFLTB_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->sfty_ctl_addrs[idx],
					      noc_info->sfty_ctl_hw[idx]->cfltb_low));

	if (REGISTER_VALID(noc_info->sfty_ctl_hw[idx]->cfltb_high))
		noc_info->syndrome.sfty_ctl[idx].CFLTB_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->sfty_ctl_addrs[idx],
					      noc_info->sfty_ctl_hw[idx]->cfltb_high));

	if (REGISTER_VALID(noc_info->sfty_ctl_hw[idx]->ufltb_low))
		noc_info->syndrome.sfty_ctl[idx].UFLTB_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->sfty_ctl_addrs[idx],
					      noc_info->sfty_ctl_hw[idx]->ufltb_low));

	if (REGISTER_VALID(noc_info->sfty_ctl_hw[idx]->ufltb_high))
		noc_info->syndrome.sfty_ctl[idx].UFLTB_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->sfty_ctl_addrs[idx],
					      noc_info->sfty_ctl_hw[idx]->ufltb_high));

	if (REGISTER_VALID(noc_info->sfty_ctl_hw[idx]->cfltc_low))
		noc_info->syndrome.sfty_ctl[idx].CFLTC_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->sfty_ctl_addrs[idx],
					      noc_info->sfty_ctl_hw[idx]->cfltc_low));

	if (REGISTER_VALID(noc_info->sfty_ctl_hw[idx]->cfltc_high))
		noc_info->syndrome.sfty_ctl[idx].CFLTC_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->sfty_ctl_addrs[idx],
					      noc_info->sfty_ctl_hw[idx]->cfltc_high));

	if (REGISTER_VALID(noc_info->sfty_ctl_hw[idx]->ufltc_low))
		noc_info->syndrome.sfty_ctl[idx].UFLTC_LOW =
			NOC_IN32(NOC_REG_ADDR(noc_info->sfty_ctl_addrs[idx],
					      noc_info->sfty_ctl_hw[idx]->ufltc_low));

	if (REGISTER_VALID(noc_info->sfty_ctl_hw[idx]->ufltc_high))
		noc_info->syndrome.sfty_ctl[idx].UFLTC_HIGH =
			NOC_IN32(NOC_REG_ADDR(noc_info->sfty_ctl_addrs[idx],
					      noc_info->sfty_ctl_hw[idx]->ufltc_high));

	ERROR("(%x %s %x %x %x %x %x %x %x %x %x %x %x %x %x)\n",
	      NOC_SFTY_NAME_SYNDROME_REG, noc_info->name, idx,
	      noc_info->syndrome.sfty_ctl[idx].CFLTA_LOW,
	      noc_info->syndrome.sfty_ctl[idx].CFLTA_HIGH,
	      noc_info->syndrome.sfty_ctl[idx].UFLTA_LOW,
	      noc_info->syndrome.sfty_ctl[idx].UFLTA_HIGH,
	      noc_info->syndrome.sfty_ctl[idx].CFLTB_LOW,
	      noc_info->syndrome.sfty_ctl[idx].CFLTB_HIGH,
	      noc_info->syndrome.sfty_ctl[idx].UFLTB_LOW,
	      noc_info->syndrome.sfty_ctl[idx].UFLTB_HIGH,
	      noc_info->syndrome.sfty_ctl[idx].CFLTC_LOW,
	      noc_info->syndrome.sfty_ctl[idx].CFLTC_HIGH,
	      noc_info->syndrome.sfty_ctl[idx].UFLTC_LOW,
	      noc_info->syndrome.sfty_ctl[idx].UFLTC_HIGH);
}

/*
 * Per-vector ISR registered through qti_interrupt_svc_register(). Walks the
 * platform NoC list, logs syndrome on a faulted instance, and panics on a
 * fatal fault. Returns @ctx as required by qti_int_svc_isr_t.
 */
static void *qti_noc_error_handle_interrupt(uint32_t int_num, void *ctx)
{
	uint32_t noc_idx, idx, val = 0u;
	uint32_t intr_vector = int_num;
	struct nocerr_info *noc_info = NULL;
	struct nocerr_info_oem *noc_info_oem = NULL;
	bool fatal_fault_detected = false;
	bool any_irq_match = false;

	spin_lock(&isr_log_sync_lock);

	/* Validate global data structures early */
	if (nocerr_propdata == NULL || noc_info_list == NULL || noc_info_oem_list == NULL) {
		ERROR("NOC error handler not properly initialized\n");
		spin_unlock(&isr_log_sync_lock);
		return ctx;
	}

	/* Do FFI safety map before writing to noc regions */
	qti_noc_error_ffi_mem_map_wrap(true);

	/*
	 * Find which NOC generated interrupt
	 * Potentially multiple on a single IRQ.
	 */
	for (noc_idx = 0; noc_idx < nocerr_propdata->len; noc_idx++) {
		bool fault_detected = false;
		bool safety_fault_detected = false;
		bool target_delay_fatal_unused = false;
		bool filter_delay_fatal_unused = true;
		bool obs_err_valid = false;
		bool handle_target_fatal = false;

		if (noc_info_list[noc_idx].intr_vector == intr_vector) {
			any_irq_match = true;
			/*
			 * If a summary interrupt status register is present,
			 * use it to determine which NoC actually faulted.
			 */
			if (noc_info_list[noc_idx].summary_intr_status_addr != NULL) {
				if (noc_info_oem_list[noc_idx].summary_intr_enable_bit_set != 0u) {
					uint32_t status = NOC_IN32(
						noc_info_list[noc_idx].summary_intr_status_addr);

					if ((status &
					     noc_info_oem_list[noc_idx].summary_intr_enable_bit_set)
					    != 0u) {
						noc_info = &noc_info_list[noc_idx];
						noc_info_oem = &noc_info_oem_list[noc_idx];
					}
				}
			} else {
				noc_info = &noc_info_list[noc_idx];
				noc_info_oem = &noc_info_oem_list[noc_idx];
			}
		}

		/* No match: noc_info/noc_info_oem are NULL, go on to the next. */
		if (noc_info == NULL || noc_info_oem == NULL) {
			continue;
		}

		/* Skip NoCs whose SKU parts are disabled. */
		if (noc_info->is_part_disabled) {
			continue;
		}

		if (noc_info->base_addr != NULL) {
			/* Log OBS block syndrome registers if fault detected */
			if (NOC_IN32(NOC_REG_ADDR(noc_info->base_addr,
						  noc_info->hw->err_valid_low))) {
				fault_detected = true;
				obs_err_valid = true;
				qti_noc_error_log_obs(noc_info);
			}
		}

		/* Log MSI encoder faults if MSI info exists */
		if (noc_info->msi_info != NULL) {
			for (idx = 0; idx < noc_info->msi_info->num_msis; idx++) {
				struct noc_msi_hw *mhw =
					noc_info->msi_info->msi_hw[idx];

				if (REGISTER_VALID(mhw->msienc_errorsts_low)) {
					val = NOC_IN32(NOC_REG_ADDR(
						noc_info->msi_info->msi_base_addrs[idx],
						mhw->msienc_errorsts_low));
					if ((val & 0x1) != 0) {
						fault_detected = true;
						qti_noc_error_log_msi(noc_info, idx);
					}
				}
			}
		}

		/* Check for safety controller errors, if present. */
		for (idx = 0; idx < noc_info_list[noc_idx].num_sfty_ctl; idx++) {
			val = NOC_IN32(NOC_REG_ADDR(
				noc_info_list[noc_idx].sfty_ctl_addrs[idx],
				noc_info_list[noc_idx].sfty_ctl_hw[idx]->status_low));
			noc_info->syndrome.sfty_ctl[idx].STATUS_LOW = val;
			/* Detect cflt / uflt error */
			if ((val & 0x3) != 0) {
				fault_detected = true;
				safety_fault_detected = true;
				qti_noc_error_log_safety(noc_info, idx);
			}
		}

		/* Log Sideband Manager syndrome info if fault detected */
		for (idx = 0; idx < noc_info->num_sbms; idx++) {
			if (REGISTER_VALID(noc_info->sb_hw[idx]->faultin_status0_low)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_status0_low));
				noc_info->syndrome.sbms[idx].FAULTINSTATUS0_LOW = val;
				if (val != 0) {
					fault_detected = true;
				}
			}
			if (REGISTER_VALID(noc_info->sb_hw[idx]->faultin_status0_high)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_status0_high));
				noc_info->syndrome.sbms[idx].FAULTINSTATUS0_HIGH = val;
				if (val != 0) {
					fault_detected = true;
				}
			}
			if (REGISTER_VALID(noc_info->sb_hw[idx]->faultin_status1_low)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_status1_low));
				noc_info->syndrome.sbms[idx].FAULTINSTATUS1_LOW = val;
				if (val != 0) {
					fault_detected = true;
				}
			}
			if (REGISTER_VALID(noc_info->sb_hw[idx]->faultin_status1_high)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_status1_high));
				noc_info->syndrome.sbms[idx].FAULTINSTATUS1_HIGH = val;
				if (val != 0) {
					fault_detected = true;
				}
			}
			/* FAULTIN2 (optional, only present on newer SBMs). */
			if (REGISTER_VALID(noc_info->sb_hw[idx]->faultin_status2_low)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_status2_low));
				noc_info->syndrome.sbms[idx].FAULTINSTATUS2_LOW = val;
				if (val != 0) {
					fault_detected = true;
				}
			}
			if (REGISTER_VALID(noc_info->sb_hw[idx]->faultin_status2_high)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_status2_high));
				noc_info->syndrome.sbms[idx].FAULTINSTATUS2_HIGH = val;
				if (val != 0) {
					fault_detected = true;
				}
			}

			if (fault_detected) {
				ERROR("(%x %s %x %x %x %x %x %x %x)\n", NOC_FAULT_NAME_SBMS,
				      noc_info->name, idx,
				      noc_info->syndrome.sbms[idx].FAULTINSTATUS0_LOW,
				      noc_info->syndrome.sbms[idx].FAULTINSTATUS0_HIGH,
				      noc_info->syndrome.sbms[idx].FAULTINSTATUS1_LOW,
				      noc_info->syndrome.sbms[idx].FAULTINSTATUS1_HIGH,
				      noc_info->syndrome.sbms[idx].FAULTINSTATUS2_LOW,
				      noc_info->syndrome.sbms[idx].FAULTINSTATUS2_HIGH);
			}
		}

		/* Log Point-of-Serialization faults */
		for (idx = 0; idx < noc_info->num_pos; idx++) {
			if (REGISTER_VALID(noc_info->pos_hw[idx]->errlog_low)) {
				val = NOC_IN32(NOC_REG_ADDR(noc_info->pos_base_addrs[idx],
							    noc_info->pos_hw[idx]->errlog_low));
				noc_info->syndrome.pos[idx].ERRLOG_LOW = val;

				/* Check ErrVld specifically */
				if ((val & 0x2) != 0) {
					fault_detected = true;
				} else {
					continue;
				}
			}
			if (REGISTER_VALID(noc_info->pos_hw[idx]->errlog_high))
				noc_info->syndrome.pos[idx].ERRLOG_HIGH =
					NOC_IN32(NOC_REG_ADDR(noc_info->pos_base_addrs[idx],
							      noc_info->pos_hw[idx]->errlog_high));

			ERROR("(%x %s %x %x %x)\n", NOC_POS_NAME_SYNDROME_REG,
			      noc_info->name, idx,
			      noc_info->syndrome.pos[idx].ERRLOG_LOW,
			      noc_info->syndrome.pos[idx].ERRLOG_HIGH);
		}

		for (idx = 0; idx < noc_info->num_poc; idx++) {
			if (REGISTER_VALID(noc_info->poc_hw[idx]->errstatus_low)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->poc_base_addrs[idx],
					noc_info->poc_hw[idx]->errstatus_low));

				/* Check ErrVld specifically */
				if ((val & 0x1) != 0) {
					noc_info->syndrome.poc[idx].ERRLOGSTATUS_LOW = val;
					fault_detected = true;
				} else {
					continue;
				}
			}
			if (REGISTER_VALID(noc_info->poc_hw[idx]->errlogmain_low)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->poc_base_addrs[idx],
					noc_info->poc_hw[idx]->errlogmain_low));
				noc_info->syndrome.poc[idx].ERRLOGMAIN_LOW = val;
			}
			if (REGISTER_VALID(noc_info->poc_hw[idx]->errlogmain_high)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->poc_base_addrs[idx],
					noc_info->poc_hw[idx]->errlogmain_high));
				noc_info->syndrome.poc[idx].ERRLOGMAIN_HIGH = val;
			}

			if (REGISTER_VALID(noc_info->poc_hw[idx]->errlogaddr_low)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->poc_base_addrs[idx],
					noc_info->poc_hw[idx]->errlogaddr_low));
				noc_info->syndrome.poc[idx].ERRLOGADDR_LOW = val;
			}
			if (REGISTER_VALID(noc_info->poc_hw[idx]->errlogaddr_high)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->poc_base_addrs[idx],
					noc_info->poc_hw[idx]->errlogaddr_high));
				noc_info->syndrome.poc[idx].ERRLOGADDR_HIGH = val;
			}

			if (REGISTER_VALID(noc_info->poc_hw[idx]->errloguser_low)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->poc_base_addrs[idx],
					noc_info->poc_hw[idx]->errloguser_low));
				noc_info->syndrome.poc[idx].ERRLOGUSER_LOW = val;
			}
			if (REGISTER_VALID(noc_info->poc_hw[idx]->errloguser_high)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->poc_base_addrs[idx],
					noc_info->poc_hw[idx]->errloguser_high));
				noc_info->syndrome.poc[idx].ERRLOGUSER_HIGH = val;
			}
			/* ERRLOGMISC (optional, only present on newer PoCs). */
			if (REGISTER_VALID(noc_info->poc_hw[idx]->errlogmisc_low)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->poc_base_addrs[idx],
					noc_info->poc_hw[idx]->errlogmisc_low));
				noc_info->syndrome.poc[idx].ERRLOGMISC_LOW = val;
			}
			if (REGISTER_VALID(noc_info->poc_hw[idx]->errlogmisc_high)) {
				val = NOC_IN32(NOC_REG_ADDR(
					noc_info->poc_base_addrs[idx],
					noc_info->poc_hw[idx]->errlogmisc_high));
				noc_info->syndrome.poc[idx].ERRLOGMISC_HIGH = val;
			}

			ERROR("(%x %s %x %x %x %x %x %x %x %x %x %x)\n",
			      NOC_POC_NAME_SYNDROME_REG, noc_info->name, idx,
			      noc_info->syndrome.poc[idx].ERRLOGSTATUS_LOW,
			      noc_info->syndrome.poc[idx].ERRLOGMAIN_LOW,
			      noc_info->syndrome.poc[idx].ERRLOGMAIN_HIGH,
			      noc_info->syndrome.poc[idx].ERRLOGADDR_LOW,
			      noc_info->syndrome.poc[idx].ERRLOGADDR_HIGH,
			      noc_info->syndrome.poc[idx].ERRLOGUSER_LOW,
			      noc_info->syndrome.poc[idx].ERRLOGUSER_HIGH,
			      noc_info->syndrome.poc[idx].ERRLOGMISC_LOW,
			      noc_info->syndrome.poc[idx].ERRLOGMISC_HIGH);
		}

		/* Clear OBS error status */
		if (noc_info->base_addr != NULL) {
			NOC_OUT32(NOC_REG_ADDR(noc_info->base_addr,
					       noc_info->hw->err_clear_low), 0x1);
		}

		/* Clear any PoS error status */
		for (idx = 0; idx < noc_info->num_pos; idx++) {
			if (REGISTER_VALID(noc_info->pos_hw[idx]->errlogclr_low))
				NOC_OUT32(NOC_REG_ADDR(noc_info->pos_base_addrs[idx],
						       noc_info->pos_hw[idx]->errlogclr_low),
					  0x1);
		}

		for (idx = 0; idx < noc_info->num_poc; idx++) {
			if (REGISTER_VALID(noc_info->poc_hw[idx]->errack_low))
				NOC_OUT32(NOC_REG_ADDR(noc_info->poc_base_addrs[idx],
						       noc_info->poc_hw[idx]->errack_low),
					  0x1);
		}

		/* Clear MSI encoder errors */
		if (noc_info->msi_info != NULL) {
			for (idx = 0; idx < noc_info->msi_info->num_msis; idx++) {
				struct noc_msi_hw *mhw =
					noc_info->msi_info->msi_hw[idx];

				if (REGISTER_VALID(mhw->msienc_errorclr_low))
					NOC_OUT32(NOC_REG_ADDR(
						noc_info->msi_info->msi_base_addrs[idx],
						mhw->msienc_errorclr_low),
						0x1);
			}
		}

		/* Clear any sideband managers. */
		for (idx = 0; idx < noc_info->num_sbms; idx++) {
			if (REGISTER_VALID(noc_info->sb_hw[idx]->faultin_status0_low)) {
				val = NOC_IN32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_status0_low));
				NOC_OUT32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_en0_low),
					(NOC_IN32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_en0_low)) &
					(~val | noc_info_oem->obs_mask[idx].faultin_en0_low)));
			}
			if (REGISTER_VALID(noc_info->sb_hw[idx]->faultin_status0_high)) {
				val = NOC_IN32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_status0_high));

				NOC_OUT32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_en0_high),
					(NOC_IN32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_en0_high)) &
					(~val | noc_info_oem->obs_mask[idx].faultin_en0_high)));
			}
			if (REGISTER_VALID(noc_info->sb_hw[idx]->faultin_status1_low)) {
				val = NOC_IN32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_status1_low));

				NOC_OUT32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_en1_low),
					(NOC_IN32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_en1_low)) &
					(~val |
					noc_info_oem->obs_mask[idx].faultin_en1_low)));
			}
			if (REGISTER_VALID(noc_info->sb_hw[idx]->faultin_status1_high)) {
				val = NOC_IN32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_status1_high));

				NOC_OUT32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_en1_high),
					(NOC_IN32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_en1_high)) &
					(~val |
					noc_info_oem->obs_mask[idx].faultin_en1_high)));
			}
			/* FAULTIN2 (optional, only present on newer SBMs). */
			if (REGISTER_VALID(noc_info->sb_hw[idx]->faultin_status2_low)) {
				val = NOC_IN32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_status2_low));

				NOC_OUT32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_en2_low),
					(NOC_IN32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_en2_low)) &
					(~val |
					noc_info_oem->obs_mask[idx].faultin_en2_low)));
			}
			if (REGISTER_VALID(noc_info->sb_hw[idx]->faultin_status2_high)) {
				val = NOC_IN32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_status2_high));

				NOC_OUT32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_en2_high),
					(NOC_IN32(NOC_REG_ADDR(noc_info->sb_base_addrs[idx],
					noc_info->sb_hw[idx]->faultin_en2_high)) &
					(~val |
					noc_info_oem->obs_mask[idx].faultin_en2_high)));
			}
		}

		/*
		 * Don't set error fatal until we're sure we're done.
		 * Do record that we need to, though.
		 */
		handle_target_fatal = qti_noc_error_handle_target(noc_info,
								  noc_info_oem,
								  &target_delay_fatal_unused);
		if ((fault_detected && noc_info_oem->error_fatal) ||
		    safety_fault_detected) {
			if (qti_noc_error_handle_filter(noc_info, noc_info_oem,
							obs_err_valid,
							&filter_delay_fatal_unused) &&
			    handle_target_fatal) {
				fatal_fault_detected = true;
			}
		}
	}

	/* Do FFI safety unmap after writing into noc registers */
	qti_noc_error_ffi_mem_map_wrap(false);

	if (fatal_fault_detected) {
		ERROR("Fatal NOC error detected!\n");
		spin_unlock(&isr_log_sync_lock);
		panic();
	} else if (!any_irq_match) {
		ERROR("NOC Invalid Interrupt Vector!\n");
	}
	spin_unlock(&isr_log_sync_lock);

	return ctx;
}

void qti_icb_error_init(void)
{
	uint32_t i, idx, j;

	nocerr_propdata = qti_noc_error_platform_get_propdata();
	if (nocerr_propdata == NULL)
		return;

	nocerr_propdata_oem = qti_noc_error_platform_get_propdata_oem();
	if (nocerr_propdata_oem == NULL)
		return;

	noc_info_list = nocerr_propdata->noc_info_list;
	if (noc_info_list == NULL)
		return;

	noc_info_oem_list = nocerr_propdata_oem->noc_info_oem_list;
	if (noc_info_oem_list == NULL)
		return;

	qti_noc_error_init_target(noc_info_list, nocerr_propdata->len, noc_info_oem_list);

	/* Enable timeout clocks */
	for (i = 0; i < nocerr_propdata->num_clock_regs; i++)
		NOC_OUT32(nocerr_propdata->clock_reg_addrs[i],
			  nocerr_propdata_oem->clock_reg_vals[i]);

	for (i = 0; i < nocerr_propdata->len; i++) {
		/*
		 * If every SKU part associated with this NoC is disabled, skip
		 * programming its registers. Platforms with no SKU fusing leave
		 * num_qultivate_parts == 0 and is_part_disabled == false.
		 */
		for (j = 0; j < noc_info_list[i].num_qultivate_parts; j++) {
			struct noc_qtv *qtv = &noc_info_list[i].qultivate_parts[j];

			noc_info_list[i].is_part_disabled =
				qti_noc_error_is_part_disabled(qtv->qultivate_part_type,
							       qtv->idx);
			/*
			 * If any one of the parts is not disabled, we should not
			 * skip programming NOC registers.
			 */
			if (!noc_info_list[i].is_part_disabled)
				break;
		}

		/* Skip the program if all of the parts are disabled. */
		if (noc_info_list[i].is_part_disabled)
			continue;

		/* Only register if we haven't already. */
		if (!is_qti_noc_error_isr_registered(i)) {
			int err;

			err = qti_interrupt_svc_register(
				noc_info_list[i].intr_vector,
				qti_noc_error_handle_interrupt,
				(void *)(uintptr_t)noc_info_list[i].intr_vector);
			if (err != 0) {
				ERROR("icbuerr: ISR registration failed for vec %u (%d)\n",
				      (uint32_t)noc_info_list[i].intr_vector, err);
				/* Roll back: unregister anything we already registered. */
				for (uint32_t k = 0; k < i; k++) {
					if (noc_info_list[k].intr_vector != NO_INTERRUPT) {
						(void)qti_interrupt_svc_unregister(
							noc_info_list[k].intr_vector);
					}
				}
				return;
			}
		}

		/* Enable Interrupts (Set FAULTEN) */
		if (noc_info_oem_list[i].intr_enable) {
			/* Enable summary interrupt bit, if present. */
			if (noc_info_list[i].summary_intr_enable_addr != NULL) {
				NOC_OUT32(noc_info_list[i].summary_intr_enable_addr,
					  NOC_IN32(noc_info_list[i].summary_intr_enable_addr) |
					  noc_info_oem_list[i].summary_intr_enable_bit_set);
			}

			/* Write sideband configuration */
			for (idx = 0; idx < noc_info_list[i].num_sbms; idx++) {
				if (REGISTER_VALID(noc_info_list[i].sb_hw[idx]->faultin_en0_low)) {
					NOC_OUT32(NOC_REG_ADDR(
						noc_info_list[i].sb_base_addrs[idx],
						noc_info_list[i].sb_hw[idx]->faultin_en0_low),
						noc_info_oem_list[i].sbms[idx].faultin_en0_low);
				}
				if (REGISTER_VALID(noc_info_list[i].sb_hw[idx]->faultin_en0_high))
					NOC_OUT32(NOC_REG_ADDR(
						noc_info_list[i].sb_base_addrs[idx],
						noc_info_list[i].sb_hw[idx]->faultin_en0_high),
						noc_info_oem_list[i].sbms[idx].faultin_en0_high);

				if (REGISTER_VALID(noc_info_list[i].sb_hw[idx]->faultin_en1_low))
					NOC_OUT32(NOC_REG_ADDR(
						noc_info_list[i].sb_base_addrs[idx],
						noc_info_list[i].sb_hw[idx]->faultin_en1_low),
						noc_info_oem_list[i].sbms[idx].faultin_en1_low);

				if (REGISTER_VALID(noc_info_list[i].sb_hw[idx]->faultin_en1_high))
					NOC_OUT32(NOC_REG_ADDR(
						noc_info_list[i].sb_base_addrs[idx],
						noc_info_list[i].sb_hw[idx]->faultin_en1_high),
						noc_info_oem_list[i].sbms[idx].faultin_en1_high);

				/* FAULTIN2 (optional, only present on newer SBMs). */
				if (REGISTER_VALID(noc_info_list[i].sb_hw[idx]->faultin_en2_low))
					NOC_OUT32(NOC_REG_ADDR(
						noc_info_list[i].sb_base_addrs[idx],
						noc_info_list[i].sb_hw[idx]->faultin_en2_low),
						noc_info_oem_list[i].sbms[idx].faultin_en2_low);

				if (REGISTER_VALID(noc_info_list[i].sb_hw[idx]->faultin_en2_high))
					NOC_OUT32(NOC_REG_ADDR(
						noc_info_list[i].sb_base_addrs[idx],
						noc_info_list[i].sb_hw[idx]->faultin_en2_high),
						noc_info_oem_list[i].sbms[idx].faultin_en2_high);
			}

			/* Write MSI configuration */
			if (noc_info_list[i].msi_info != NULL) {
				struct msi_info *msi =
					noc_info_list[i].msi_info;

				for (idx = 0; idx < msi->num_msis; idx++) {
					struct noc_msi_hw *mhw =
						msi->msi_hw[idx];

					if (REGISTER_VALID(mhw->msienc_errorset_low))
						NOC_OUT32(NOC_REG_ADDR(
							msi->msi_base_addrs[idx],
							mhw->msienc_errorset_low),
							0x1);
				}
			}

			/* Write PoS configuration. */
			for (idx = 0; idx < noc_info_list[i].num_pos; idx++) {
				if (REGISTER_VALID(noc_info_list[i].pos_hw[idx]->errlog_low))
					NOC_OUT32(NOC_REG_ADDR(
						noc_info_list[i].pos_base_addrs[idx],
						noc_info_list[i].pos_hw[idx]->errlog_low),
						noc_info_oem_list[i].pos[idx].enable ? 1 : 0);
			}

			/* Write PoC (Point of Coherency) configuration. */
			for (idx = 0; idx < noc_info_list[i].num_poc; idx++) {
				if (REGISTER_VALID(noc_info_list[i].poc_hw[idx]->errset_low)) {
					uint32_t mask = 0x1;

					NOC_OUTM32(NOC_REG_ADDR(
						noc_info_list[i].poc_base_addrs[idx],
						noc_info_list[i].poc_hw[idx]->errset_low), mask,
						(noc_info_oem_list[i].poc[idx].enable ? 1 : 0));
				}
			}

			/* Write timeout enable configuration */
			for (idx = 0; idx < noc_info_list[i].num_tos; idx++)
				NOC_OUT32(noc_info_list[i].to_addrs[idx],
					  noc_info_oem_list[i].to_reg_vals[idx]);

			/* Write out Safety configuration. */
			for (idx = 0; idx < noc_info_list[i].num_sfty_ctl; idx++) {
				if (REGISTER_VALID(noc_info_list[i].sfty_ctl_hw[idx]->outen_low))
					NOC_OUT32(NOC_REG_ADDR(
						noc_info_list[i].sfty_ctl_addrs[idx],
						noc_info_list[i].sfty_ctl_hw[idx]->outen_low),
						noc_info_oem_list[i].sfty_ctl[idx].outen_low);
			}

			/* Enable faults and stall-until-serviced fault network flag. */
			if (noc_info_list[i].base_addr != NULL) {
				NOC_OUT32(NOC_REG_ADDR(noc_info_list[i].base_addr,
						       noc_info_list[i].hw->main_ctl_low),
					  0x3);
			}
		}
	}
}
