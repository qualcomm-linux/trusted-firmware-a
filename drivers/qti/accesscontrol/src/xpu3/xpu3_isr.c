/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <xpu3.h>
#include <xpu3_hwio.h>
#include <xpu_target_info.h>

#include <qti_interrupt_svc.h>

/*
 * XPU3 violation reporting.
 *
 * The two error interrupts share one handler and are told apart by the context
 * pointer they were registered with. A violation is decoded, logged and
 * cleared; XPU3 treats violations as non-fatal.
 */

/* Interrupt contexts distinguishing secure from non-secure XPU violations. */
static int xpu_err_non_sec_ctx = XPU_ERR_NON_SEC_CTX;
static int xpu_err_sec_ctx = XPU_ERR_SEC_CTX;

static void dump_log(enum xpu xpu, int type)
{
	char sec_char[2] = { ' ', 0 };
	uint32_t xpu_addr;
	uint32_t offset;

	switch (type) {
	case XPU_ERR_SEC_CTX:
		offset = XPU3_ERR_BLK_SEC;
		sec_char[0] = 'S';
		break;
	case XPU_ERR_NON_SEC_CTX:
		offset = XPU3_ERR_BLK_NONSEC;
		break;
	default:
		return;
	}

	for (size_t i = 0; i < g_xpu_base_addr_array_count; i++) {
		if (g_xpu_base_addr_array[i].e_xpu != xpu)
			continue;

		xpu_addr = g_xpu_base_addr_array[i].base_addr + offset;

		ERROR("%s_ear0 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_EAR0_OFFSET));
		ERROR("%s_ear1 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_EAR1_OFFSET));
		ERROR("%s_esr 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_ESR_OFFSET));
		ERROR("%s_esynr0 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_ESYNR0_OFFSET));
		ERROR("%s_esynr1 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_ESYNR1_OFFSET));
		ERROR("%s_esynr2 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_ESYNR2_OFFSET));
		ERROR("%s_esynr3 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_ESYNR3_OFFSET));
		ERROR("%s_esynr4 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_ESYNR4_OFFSET));

		/* clear error (SRRESTORE) as xpu errors are non fatal */
		mmio_write_32(xpu_addr + XPU3_SRRESTORE_OFFSET, 0);
		break;
	}
}

void xpu_print_log(void *ctx)
{
	uint32_t err_bitmask[ACC_XPU_ERR_INT_REG_NUM] = { 0 };
	const struct xpu_intr_reg_dtls *p = NULL;
	int err_type = *(int *)ctx;
	uint32_t addr;
	uint32_t mask;

	ERROR("xpu: ISR\n");
	switch (err_type) {
	case XPU_ERR_SEC_CTX:
		ERROR("XPU ERROR: secure\n");
		p = xpu_sec_intr_status_reg;
		break;
	case XPU_ERR_NON_SEC_CTX:
		ERROR("XPU ERROR: non secure\n");
		p = xpu_non_sec_intr_status_reg;
		break;
	default:
		return;
	}

	for (size_t i = 0; i < ACC_XPU_ERR_INT_REG_NUM; i++, p++) {
		addr = p->xpu_intr_reg_addr;
		mask = p->xpu_intr_reg_mask;

		err_bitmask[i] = mmio_read_32(addr) & mask;
	}

	if (ACC_XPU_ERR_INT_REG_NUM == 1) {
		ERROR("XPU INTR 0 >> %08x\n", err_bitmask[0]);
	} else {
		ERROR("XPU INTR 0:1 >> %08x:%08x\n", err_bitmask[0],
		      err_bitmask[1]);
	}

	for (size_t reg = 0; reg < ACC_XPU_ERR_INT_REG_NUM; reg++) {
		struct xpu_err_pos_to_hal_map *row;

		mask = err_bitmask[reg];
		if (!mask)
			continue;

		row = xpu_err_pos_to_hal_map[reg];

		for (size_t i = 0; row[i].bit_mask != 0; i++) {
			const struct xpu_err_pos_to_hal_map *m;

			if (i >= ACC_XPU_ERR_NUM_PER_REG)
				break;

			m = &row[i];

			if (!(m->bit_mask & mask))
				continue;

			if (m->xpu >= XPU_TYPE_COUNT)
				continue;

			if (m->xpu == XPU_TYPE_NONE)
				continue;

			dump_log(m->xpu, err_type);
		}
	}
}

void xpu3_enable_interrupts(const struct xpu_intr_reg_dtls *nsec,
			    const struct xpu_intr_reg_dtls *sec)
{
	for (size_t i = 0; i < ACC_XPU_ERR_INT_REG_NUM; i++) {
		if (nsec) {
			mmio_setbits_32(nsec->xpu_intr_reg_addr,
					nsec->xpu_intr_reg_mask);
			nsec++;
		}

		if (sec) {
			mmio_setbits_32(sec->xpu_intr_reg_addr,
					sec->xpu_intr_reg_mask);
			sec++;
		}
	}
}

static void *xpu_isr(uint32_t int_num, void *ctx)
{
	xpu_print_log(ctx);
	console_flush();

	return ctx;
}

int xpu_register_interrupts(void)
{
	int err;

	err = qti_interrupt_svc_register(QTISECLIB_INT_ID_XPU_SEC, xpu_isr,
					 &xpu_err_sec_ctx);
	if (err) {
		return err;
	}

	err = qti_interrupt_svc_register(QTISECLIB_INT_ID_XPU_NON_SEC, xpu_isr,
					 &xpu_err_non_sec_ctx);
	if (err) {
		qti_interrupt_svc_unregister(QTISECLIB_INT_ID_XPU_SEC);
	}

	return err;
}
