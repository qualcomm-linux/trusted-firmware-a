/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Changes from Qualcomm Technologies, Inc. are provided under the following
 * license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <bl31/interrupt_mgmt.h>
#include <drivers/arm/gic_common.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/spinlock.h>

#include <platform.h>
#include <qti_interrupt_svc.h>

#include <bl31qtilib_spd_agnostic.h>

#if SDEI_SUPPORT
#include <bl31/ehf.h>
#endif

/*
 * Structure for EL3 Interrupt handler table.
 */
struct qti_intr_info_type_el3 {
	unsigned int intr; /* Interrupt number */
	qti_interrupt_handler_t handler; /* Interrupt hander function */
	void *ctx; /* Context to pass to handler */
};

/* Interrupt handler table. */
static spinlock_t table_lock;
static struct qti_intr_info_type_el3 el3_intr_info_table[MAX_INTR_EL3];

/* Find the next free slot. */
static int qti_intr_info_table_find_slot(unsigned int intr)
{
	int i;
	int idx;
	int slot = -1;
	static int curr_slot;

	/* Cyclic slot allocator; detect duplication. */
	for (i = 0; i < MAX_INTR_EL3; i++) {
		idx = (curr_slot + i) % MAX_INTR_EL3;
		if ((slot == -1) && (el3_intr_info_table[idx].intr == 0)) {
			slot = idx;
		}

		if (el3_intr_info_table[idx].intr == intr) {
			return -EALREADY;
		}
	}

	if (slot == -1) {
		return -ENOSPC;
	}

	/* Set for the next search. */
	curr_slot = (slot + 1) % MAX_INTR_EL3;

	return slot;
}

int qti_register_intr_type_el3(unsigned int intr,
			       qti_interrupt_handler_t handler, void *ctx)
{
	int ret = -EINVAL;
	int slot = -1;

	spin_lock(&table_lock);
	if (handler == NULL) {
		goto out_failed;
	}

	slot = qti_intr_info_table_find_slot(intr);
	if (slot < 0) {
		ERROR("qti_intr_info_table_find_slot returned %d\n", slot);
		goto out_failed;
	}

	el3_intr_info_table[slot].intr = intr;
	el3_intr_info_table[slot].handler = handler;
	el3_intr_info_table[slot].ctx = ctx;
	dsb();

	ret = 0;

out_failed:
	spin_unlock(&table_lock);
	return ret;
}

void qti_unregister_intr_type_el3(unsigned int intr)
{
	int i;

	spin_lock(&table_lock);
	for (i = 0; i < MAX_INTR_EL3; i++) {
		if (el3_intr_info_table[i].intr == intr) {
			el3_intr_info_table[i].intr = 0;
			el3_intr_info_table[i].handler = NULL;
			el3_intr_info_table[i].ctx = NULL;
			break;
		}
	}
	dsb();
	spin_unlock(&table_lock);
}

void qti_disable_intr_type_el3(unsigned int intr)
{
	plat_ic_disable_interrupt(intr);
}

void qti_enable_intr_type_el3(unsigned int intr)
{
	plat_ic_enable_interrupt(intr);
}

static int qti_el3_impl_interrupt_handler(uint32_t id, uint32_t flags,
					  void *handle, void *cookie)
{
	qti_interrupt_handler_t handler = NULL;
	uint32_t intr_raw = id;
	unsigned int intr;
	void *ctx = NULL;
	int i;

	/*
	 * qti_el3_impl_interrupt_handler() is called on return path to
	 * NON_SECURE world when preempting the QTEE. Proceed with handling only
	 * for valid interrupt IDs for EL3. It is also required as there is a
	 * race between calling plat_ic_get_pending_interrupt_type() and
	 * handling the interrupt
	 */
	if (id == INTR_ID_UNAVAILABLE) {
		/* This scenario is invoked directly from qti_el3_interrupt_handler_to_ns */
		intr_raw = plat_ic_acknowledge_interrupt();
	}

	intr = plat_ic_get_interrupt_id(intr_raw);
	if (intr == INTR_ID_UNAVAILABLE) {
		return 0;
	}

	VERBOSE("EL3 interrupt %d\n", intr);

#ifdef QTI_BL31_WITH_TEST
	bl31qtilib_spd_store_intr_context(intr, flags, handle, cookie);
#endif

	spin_lock(&table_lock);
	for (i = 0; i < MAX_INTR_EL3; i++) {
		if (intr == el3_intr_info_table[i].intr) {
			handler = el3_intr_info_table[i].handler;
			ctx = el3_intr_info_table[i].ctx;
			break;
		}
	}
	spin_unlock(&table_lock);
	/* Run the interrupt handler. */
	if (handler != NULL) {
		(void)handler(ctx);
	} else {
		ERROR("EL3 unhandled interrupt %d\n", intr);
	}

	plat_ic_end_of_interrupt(intr);

	return 0;
}

void qti_el3_interrupt_handler_to_ns(void *handle)
{
	/* flag == 0: Assume it is coming from SECURE world. */
	qti_el3_impl_interrupt_handler(INTR_ID_UNAVAILABLE, 0, handle, NULL);
}

#if !SDEI_SUPPORT
/* When SDEI is not supported, TFA will use this generic handler for all
 * INTR_TYPE_EL3 interrupts. This will perform the gic operations and then call
 * our qti_el3_impl_interrupt_handler.
 * When SDEI is enabled, we have the EHF (exception handler framework) which
 * requires handler to be set per priority. In this case we leverage the EHF
 * main handler for GIC operations, and register our
 * qti_el3_impl_interrupt_handler for the required priorities.
 */

/*
 * Top-level EL3 interrupt handler.
 * This is only intended to be invoked when an EL3 interrupt occurs
 * when running in Non-Secure world and the interrupt traps to EL3.
 */
static uint64_t qti_el3_interrupt_handler(uint32_t id, uint32_t flags,
					  void *handle, void *cookie)
{
	/*
	 * Ensure the security state when the exception was generated
	 *  is Non-Secure
	 */
	assert(get_interrupt_src_ss(flags) == NON_SECURE);
	/* Sanity check the pointer to this cpu's context. */
	assert(handle == cm_get_context(NON_SECURE));
	assert(id == INTR_ID_UNAVAILABLE);

	qti_el3_impl_interrupt_handler(id, flags, handle, cookie);

	return (uint64_t)handle;
}
#endif /* !SDEI_SUPPORT */

int qti_interrupt_svc_init(void)
{
	int ret = 0;

#if SDEI_SUPPORT
	/*
	 * Register handler at GIC_HIGHEST_SEC_PRIORITY, which will be the only one
	 * we support
	 */
	ehf_register_priority_handler(GIC_HIGHEST_SEC_PRIORITY,
				      qti_el3_impl_interrupt_handler);
#else
	uint64_t flags = 0U;

	/* Route EL3 interrupts to EL3 when in Non-secure. */
	set_interrupt_rm_flag(flags, NON_SECURE);

	/* Register handler for EL3 interrupts */
	ret = register_interrupt_type_handler(INTR_TYPE_EL3,
					      qti_el3_interrupt_handler, flags);
	assert(ret == 0);
#endif /* SDEI_SUPPORT */
	return ret;
}
