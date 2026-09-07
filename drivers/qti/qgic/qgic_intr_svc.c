/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Hoya implementation of the family-agnostic qgic interrupt shim.
 *
 * Hoya's platform interrupt service (qti_interrupt_svc_*) dispatches to a
 * two-argument ISR of type qti_int_svc_isr_t: void *(*)(uint32_t id,
 * void *ctx). The shim exposes a single-argument ISR type (qti_isr_t:
 * void *(*)(void *ctx)).
 *
 * A per-slot adapter table bridges the two. Each slot stores the caller's
 * one-argument fn and its ctx. A single trampoline is registered with the hoya
 * service; the slot index is handed to the service as the opaque ctx, so on
 * dispatch the trampoline recovers the slot, invokes the caller's fn with the
 * caller's ctx, and preserves the index for subsequent dispatches.
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <drivers/qti/qgic/qgic.h>
#include <qti_interrupt_svc.h>
#include <qti_plat.h>

/* Sized to hoya's ISR table depth (see qti_interrupt_svc.c ISR_TABLE_LEN). */
#define QGIC_ADAPTER_SLOTS	20U

#define QGIC_SLOT_FREE		UINT32_MAX

static struct qgic_adapter {
	struct qgic_slot {
		qti_isr_t fn;
		void *ctx;
		uint32_t intr;
	} slot[QGIC_ADAPTER_SLOTS];

	spinlock_t lock;
} qgic_adapter = {
	.slot[0 ... QGIC_ADAPTER_SLOTS - 1] = { .intr = QGIC_SLOT_FREE },
};

/*
 * Trampoline registered with the hoya interrupt service. The opaque ctx handed
 * back by the service is the adapter slot index. Recover the slot, dispatch the
 * caller's one-argument ISR, and return the same index so the service keeps the
 * slot stable across dispatches.
 */
static void *qgic_trampoline(uint32_t id, void *ctx)
{
	uintptr_t idx = (uintptr_t)ctx;

	(void)id;
	assert(idx < QGIC_ADAPTER_SLOTS);

	qgic_adapter.slot[idx].ctx = qgic_adapter.slot[idx].fn(
		qgic_adapter.slot[idx].ctx);

	return ctx;
}

int qti_gic_register_isr(uint32_t intr, qti_isr_t fn, void *ctx)
{
	int ret = -ENOTCAPABLE;

	spin_lock(&qgic_adapter.lock);

	for (uintptr_t i = 0U; i < QGIC_ADAPTER_SLOTS; i++) {
		if (qgic_adapter.slot[i].intr != QGIC_SLOT_FREE) {
			continue;
		}

		qgic_adapter.slot[i].fn = fn;
		qgic_adapter.slot[i].ctx = ctx;
		qgic_adapter.slot[i].intr = intr;

		ret = qti_interrupt_svc_register(intr, qgic_trampoline,
						 (void *)i);
		if (ret != 0) {
			/* Roll back the slot reservation on failure. */
			qgic_adapter.slot[i].fn = NULL;
			qgic_adapter.slot[i].ctx = NULL;
			qgic_adapter.slot[i].intr = QGIC_SLOT_FREE;
		}
		break;
	}

	spin_unlock(&qgic_adapter.lock);

	return ret;
}

void qti_gic_unregister_isr(uint32_t intr)
{
	spin_lock(&qgic_adapter.lock);

	for (uintptr_t i = 0U; i < QGIC_ADAPTER_SLOTS; i++) {
		if (qgic_adapter.slot[i].intr != intr) {
			continue;
		}

		qti_interrupt_svc_unregister(intr);
		qgic_adapter.slot[i].fn = NULL;
		qgic_adapter.slot[i].ctx = NULL;
		qgic_adapter.slot[i].intr = QGIC_SLOT_FREE;
		break;
	}

	spin_unlock(&qgic_adapter.lock);
}

void qti_gic_enable_intr(uint32_t intr)
{
	plat_ic_enable_interrupt(intr);
}

void qti_gic_disable_intr(uint32_t intr)
{
	plat_ic_disable_interrupt(intr);
}

void qti_gic_set_spi_routing(uint32_t intr, unsigned int irm,
			     u_register_t mpidr)
{
	gic_set_spi_routing(intr, irm, mpidr);
}
