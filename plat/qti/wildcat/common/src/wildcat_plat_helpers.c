/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/par.h>
#include <drivers/arm/gicv3.h>
#include <lib/smccc.h>
#include <lib/utils_def.h>

#include <plat/common/platform.h>
#include <platform_def.h>
#include <qti_interrupt_props.h>
#include <wildcat_plat.h>

int qti_ns_va_to_pa(uintptr_t va, unsigned int client_mode, uintptr_t *pa_out)
{
	sysreg_t par;
	u_register_t scr_el3;

	assert((client_mode == MODE_EL2) || (client_mode == MODE_EL1));
	assert(pa_out != NULL);

	scr_el3 = read_scr_el3();
	write_scr_el3((scr_el3 | SCR_NS_BIT) & ~SCR_NSE_BIT);
	isb();

	if (client_mode == MODE_EL2) {
		ats1e2r(va);
	} else {
		AT(ats12e1r, va);
	}
	isb();

	par = read_par_el1();
	write_scr_el3(scr_el3);
	isb();

	if ((par & PAR_F_MASK) != 0U) {
		return -1;
	}

	*pa_out = (uintptr_t)get_par_el1_pa(par);
	return 0;
}

struct console_ringbuf *plat_qti_get_bl31_ringbuf(void)
{
#if defined(TFA_IMEM_BASE)
	return (struct console_ringbuf *)TFA_BL31_RING_BUFFER_IN_TZ_IMEM_BASE;
#else
	static struct console_ringbuf qti_bl31_ringbuf;

	return &qti_bl31_ringbuf;
#endif
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int core_pos = plat_qti_core_pos_by_mpidr(mpidr);

	if (core_pos < PLATFORM_CORE_COUNT) {
		return (int)core_pos;
	}

	return -1;
}

void plat_error_handler(int error)
{
	ERROR("QTI fatal platform error: %d\n", error);
	panic();
}

void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle)
{
	(void)handle;

	ERROR("EL3 unhandled interrupt %u\n", id);
}

const interrupt_prop_t *plat_qti_get_interrupt_props(unsigned int *num_props)
{
	*num_props = ARRAY_SIZE(qti_interrupt_props);

	return qti_interrupt_props;
}
