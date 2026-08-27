/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ac_cfg.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <xpu4.h>
#include <xpu4_hal.h>
#include <xpu_common.h>

#include <qti_interrupt_svc.h>

/*
 * Index of each interrupt within the descriptor array supplied by the config
 * image. Shared with the VMIDMT driver, which owns slots 2 to 5.
 */
#define XPU4_INTR_XPU_SEC		0U
#define XPU4_INTR_XPU_RA		1U
#define XPU4_INTR_ID_MAX		6U

/* Bits in one RA XPU error status register. */
#define XPU4_RA_ERR_NUM_PER_REG		32U

/* An RA XPU with this interrupt number has no individual line of its own. */
#define XPU4_RA_INTR_NUM_NONE		0xFFFFFFFFU

/* Increment without wrapping. */
#define UINT32_INC_SATURATE(c)			\
	do {					\
		if ((c) < UINT32_MAX) {		\
			(c)++;			\
		}				\
	} while (false)

/*
 * Silent logging.
 *
 * When enabled, a violation is recorded and cleared so execution continues;
 * when disabled a violation is fatal. The state is latched from the config
 * image by xpu4_set_ra_static_signals(), which also selects which write-once
 * static-signal registers get programmed.
 */
static bool silent_logging_enabled;

/* Backing store for the silent log. */
static struct xpu4_violation silent_log_buffer[XPU4_SILENT_LOG_ENTRIES];

/* Serialises the violation handlers against each other. */
static spinlock_t xpu4_log_lock;

/* Cumulative count of XPU errors seen. */
static volatile uint32_t xpu4_err_count;

/* RA XPU summary-interrupt decode tables, from the config image. */
static const struct ac_xpu_intr_reg *ra_intr_status_reg;
static const struct ac_xpu_intr_reg *ra_intr_enable_reg;
static const enum xpu4_id (*ra_err_pos_to_xpu_map)[XPU4_RA_ERR_NUM_PER_REG];
static uint32_t ra_intr_regs_count;

/*
 * Write a static-signal register and confirm it took.
 *
 * The write happens even when the value already matches, because some of these
 * bits are write-once and must not be left open for a later writer to change.
 */
static enum xpu4_status xpu4_write_static_signal_reg(uint32_t addr,
						     uint32_t mask,
						     uint32_t value)
{
	uint32_t hw_value;

	hw_value = mmio_read_32(addr);
	hw_value = (hw_value & ~mask) | (value & mask);

	mmio_write_32(addr, hw_value);

	if ((mmio_read_32(addr) & mask) != (value & mask)) {
		ERROR("xpu4: static signal 0x%x did not take\n", addr);
		return XPU4_ERR_RG_WRITE_MISMATCH;
	}

	return XPU4_OK;
}

/*
 * Report one violation.
 *
 * With silent logging on, the syndrome is appended to the log and the violation
 * cleared so the offending master can continue. With it off the syndrome is
 * printed and the caller escalates to a panic.
 */
static void xpu4_handle_violation(enum xpu4_id xpu_id)
{
	struct ac_xpu4_priv_info *xpu;

	xpu = xpu4_hal_get_xpu_info(xpu_id);
	if (xpu == NULL) {
		ERROR("xpu4: violation on unknown xpu_id=%u\n",
		      (uint32_t)xpu_id);
		return;
	}

	UINT32_INC_SATURATE(xpu4_err_count);

	if (silent_logging_enabled) {
		xpu4_hal_append_silent_log(xpu);
		xpu4_hal_clear_violation(xpu);
	} else {
		xpu4_hal_log_violation(xpu);
	}
}

void xpu_print_log(void *ctx)
{
	const struct ac_ra_xpu_info *info = ctx;

	if (info == NULL) {
		return;
	}

	xpu4_handle_violation((enum xpu4_id)info->xpu_id);
}

/*
 * Interrupt handler for targets where all RA XPUs share one summary line. The
 * status registers have to be decoded to find which instance faulted.
 */
static void *xpu4_isr_ra_summary(uint32_t int_num, void *ctx)
{
	bool has_violation = false;
	uint32_t reg_num;
	uint32_t bit_pos;
	uint32_t status;

	spin_lock(&xpu4_log_lock);

	/* First pass: decide whether this interrupt is real. */
	for (reg_num = 0U; reg_num < ra_intr_regs_count; reg_num++) {
		status = mmio_read_32(ra_intr_status_reg[reg_num].addr) &
			 ra_intr_status_reg[reg_num].mask;
		if (status != 0U) {
			has_violation = true;
		}
	}

	/* A spurious interrupt is not an error. */
	if (!has_violation) {
		spin_unlock(&xpu4_log_lock);
		return ctx;
	}

	/* Second pass: map each set bit back to the XPU that raised it. */
	for (reg_num = 0U; reg_num < ra_intr_regs_count; reg_num++) {
		status = mmio_read_32(ra_intr_status_reg[reg_num].addr) &
			 ra_intr_status_reg[reg_num].mask;
		if (status == 0U) {
			continue;
		}

		for (bit_pos = 0U; bit_pos < XPU4_RA_ERR_NUM_PER_REG;
		     bit_pos++) {
			if ((status & (UINT32_C(1) << bit_pos)) == 0U) {
				continue;
			}

			xpu4_handle_violation(
				ra_err_pos_to_xpu_map[reg_num][bit_pos]);
		}
	}

	/* Release the lock before panicking; panic() never returns. */
	spin_unlock(&xpu4_log_lock);

	if (!silent_logging_enabled) {
		ERROR("xpu4: unrecoverable XPU violation\n");
		panic();
	}

	return ctx;
}

/*
 * Interrupt handler for targets where each RA XPU has its own line. The context
 * identifies the instance directly, so no decode is needed.
 */
static void *xpu4_isr(uint32_t int_num, void *ctx)
{
	const struct ac_ra_xpu_info *info = ctx;

	if (info == NULL) {
		return ctx;
	}

	spin_lock(&xpu4_log_lock);

	xpu4_handle_violation((enum xpu4_id)info->xpu_id);

	/* Release the lock before panicking; panic() never returns. */
	spin_unlock(&xpu4_log_lock);

	if (!silent_logging_enabled) {
		ERROR("xpu4: unrecoverable XPU violation\n");
		panic();
	}

	return ctx;
}

/* Unmask the RA XPU error bits in the interrupt aggregator. */
static enum xpu4_status xpu4_enable_ra_tcsr_interrupts(void)
{
	uint32_t reg_num;

	for (reg_num = 0U; reg_num < ra_intr_regs_count; reg_num++) {
		uint32_t addr = ra_intr_enable_reg[reg_num].addr;
		uint32_t mask = ra_intr_enable_reg[reg_num].mask;

		mmio_setbits_32(addr, mask);

		if ((mmio_read_32(addr) & mask) != mask) {
			ERROR("xpu4: cannot enable RA interrupt at 0x%x\n",
			      addr);
			return XPU4_ERR_RG_WRITE_MISMATCH;
		}
	}

	return XPU4_OK;
}

/*
 * Program the write-once static-signal registers for every RA XPU.
 *
 * The silent-logging register is written first: it is write-once and can alias
 * the others, so it has to be settled before they are touched.
 */
int xpu4_set_ra_static_signals(bool enable_silent_logging)
{
	const struct ac_ra_xpu_info *info = NULL;
	uint32_t count = 0U;
	uint32_t i;

	silent_logging_enabled = enable_silent_logging;

	if (ac_cfg_get_ra_xpu_info(&info, &count) != 0) {
		/* No RA XPUs described; nothing to program. */
		return 0;
	}

	for (i = 0U; i < count; i++) {
		const struct ac_addr_value_tuple *sig;
		uint32_t k;

		sig = info[i].static_signal_silent_logging;
		if (enable_silent_logging && sig != NULL) {
			if (xpu4_write_static_signal_reg(sig->addr, sig->mask,
							 sig->value) !=
			    XPU4_OK) {
				return -1;
			}
		}

		sig = info[i].static_signal_others;
		for (k = 0U; sig != NULL &&
			     k < info[i].static_signal_others_count; k++) {
			if (xpu4_write_static_signal_reg(sig[k].addr,
							 sig[k].mask,
							 sig[k].value) !=
			    XPU4_OK) {
				return -1;
			}
		}
	}

	return 0;
}

int xpu_register_interrupts(void)
{
	const struct ac_ra_xpu_info *ra_info = NULL;
	struct ac_interrupt_desc *desc = NULL;
	bool has_summary_irq = false;
	bool has_individual_irq = false;
	uint32_t desc_count = 0U;
	uint32_t ra_count = 0U;
	bool silent_logging = false;
	uint32_t i;
	int err;

	if (ac_cfg_get_xpu_vmidmt_intr_info(&desc, &desc_count) != 0 ||
	    desc == NULL || desc_count < XPU4_INTR_ID_MAX) {
		ERROR("xpu4: interrupt descriptors missing from config image\n");
		return -1;
	}

	/* Install the silent log before any violation can be recorded. */
	xpu4_hal_set_silent_log_buffer(silent_log_buffer,
				       sizeof(silent_log_buffer));

	if (ac_cfg_get_silent_logging_enabled(&silent_logging) != 0) {
		silent_logging = false;
	}

	if (xpu4_set_ra_static_signals(silent_logging) != 0) {
		return -1;
	}

	/*
	 * Register the inter-domain XPU interrupt but leave it masked: on this
	 * family those violations are handled by TME_FW, not by TF-A. Only the
	 * RA XPU interrupts below are serviced here.
	 */
	err = qti_interrupt_svc_register(desc[XPU4_INTR_XPU_SEC].intr_num,
					xpu4_isr, NULL);
	if (err != 0) {
		return err;
	}

	if (ac_cfg_get_ra_xpu_info(&ra_info, &ra_count) != 0 ||
	    ra_info == NULL) {
		/* No RA XPUs on this target. */
		return 0;
	}

	/*
	 * A target either gives every RA XPU its own line or routes them all
	 * through one summary line. Mixing the two is not supported.
	 */
	for (i = 0U; i < ra_count; i++) {
		if (ra_info[i].intr_num == XPU4_RA_INTR_NUM_NONE) {
			has_summary_irq = true;
		} else {
			has_individual_irq = true;
		}
	}

	if (has_summary_irq && has_individual_irq) {
		ERROR("xpu4: mixed RA interrupt topology unsupported\n");
		return -1;
	}

	if (has_individual_irq) {
		for (i = 0U; i < ra_count; i++) {
			err = qti_interrupt_svc_register(ra_info[i].intr_num,
							 xpu4_isr,
							 (void *)&ra_info[i]);
			if (err != 0) {
				return err;
			}
		}

		return 0;
	}

	if (!has_summary_irq) {
		return 0;
	}

	if (ac_cfg_get_ra_xpu_interrupt_details(&ra_intr_status_reg,
						&ra_intr_enable_reg,
						&ra_err_pos_to_xpu_map,
						&ra_intr_regs_count) != 0) {
		ERROR("xpu4: RA interrupt details missing from config image\n");
		return -1;
	}

	err = qti_interrupt_svc_register(desc[XPU4_INTR_XPU_RA].intr_num,
					 xpu4_isr_ra_summary, NULL);
	if (err != 0) {
		return err;
	}

	if (xpu4_enable_ra_tcsr_interrupts() != XPU4_OK) {
		return -1;
	}

	return 0;
}
