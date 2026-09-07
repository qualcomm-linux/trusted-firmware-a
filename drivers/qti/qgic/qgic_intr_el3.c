/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Wildcat implementation of the family-agnostic qgic interrupt shim.
 *
 * Wildcat's EL3 interrupt API already dispatches to a one-argument ISR of
 * type qti_interrupt_handler_t, which matches the shim's qti_isr_t. No
 * adapter is required; each shim function forwards directly to the shared
 * QTI interrupt service.
 *
 * Ported drivers use qgic.h instead of including the platform interrupt
 * service directly.
 */

#include <stdint.h>

#include <lib/utils_def.h>

#include <drivers/qti/qgic/qgic.h>
#include <qti_interrupt_svc.h>
#include <qti_plat.h>

int qti_gic_register_isr(uint32_t intr, qti_isr_t fn, void *ctx)
{
	return qti_register_intr_type_el3(intr,
					  (qti_interrupt_handler_t)fn, ctx);
}

void qti_gic_unregister_isr(uint32_t intr)
{
	qti_unregister_intr_type_el3(intr);
}

void qti_gic_enable_intr(uint32_t intr)
{
	qti_enable_intr_type_el3(intr);
}

void qti_gic_disable_intr(uint32_t intr)
{
	qti_disable_intr_type_el3(intr);
}

void qti_gic_set_spi_routing(uint32_t intr, unsigned int irm,
			     u_register_t mpidr)
{
	gic_set_spi_routing(intr, irm, mpidr);
}
