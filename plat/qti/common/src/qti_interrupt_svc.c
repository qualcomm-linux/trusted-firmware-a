/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <arch_helpers.h>
#include <bl31/interrupt_mgmt.h>
#include <common/debug.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <platform.h>

#include <qti_interrupt_svc.h>
#include <qti_plat.h>

#define ISR_TABLE_LEN				64

enum qti_isr_handler_type {
	QTI_ISR_HANDLER_ID_CTX,
	QTI_ISR_HANDLER_CTX_ONLY,
};

union qti_isr_func {
	void *(*id_ctx)(uint32_t id, void *ctx);
	void *(*ctx_only)(void *ctx);
};

static struct qti_isr_table {
	struct qti_isr {
		enum qti_isr_handler_type type;
		union qti_isr_func func;
		uint32_t id;
		void *ctx;
	} entry[ISR_TABLE_LEN];

	spinlock_t lock;
	uint8_t cnt;
} isr_table = {
	.entry[0 ... ISR_TABLE_LEN - 1] = { .id = QTI_INTR_INVALID_INT_NUM },
};

static bool qti_isr_func_is_null(enum qti_isr_handler_type type,
				 union qti_isr_func func)
{
	if (type == QTI_ISR_HANDLER_ID_CTX) {
		return func.id_ctx == NULL;
	}

	return func.ctx_only == NULL;
}

static int qti_interrupt_svc_register_common(uint32_t id,
					     enum qti_isr_handler_type type,
					     union qti_isr_func func,
					     void *ctx)
{
	struct qti_isr *p = isr_table.entry;
	int free_idx = -1;

	if (qti_isr_func_is_null(type, func)) {
		return -EINVAL;
	}

	spin_lock(&isr_table.lock);

	if (isr_table.cnt >= ISR_TABLE_LEN) {
		goto error;
	}

	for (size_t i = 0; i < ISR_TABLE_LEN; i++, p++) {
		if (p->id == id) {
			spin_unlock(&isr_table.lock);
			return -EALREADY;
		}

		if ((free_idx < 0) && (p->id == QTI_INTR_INVALID_INT_NUM)) {
			free_idx = (int)i;
		}
	}

	if (free_idx < 0) {
		goto error;
	}

	p = &isr_table.entry[free_idx];
	p->type = type;
	p->func = func;
	p->ctx = ctx;
	p->id = id;
	isr_table.cnt++;

	spin_unlock(&isr_table.lock);
	return 0;

error:
	spin_unlock(&isr_table.lock);
	return -ENOTCAPABLE;
}

static int qti_interrupt_svc_unregister_common(uint32_t id)
{
	struct qti_isr *p = isr_table.entry;

	spin_lock(&isr_table.lock);

	if (!isr_table.cnt) {
		goto error;
	}

	for (size_t i = 0; i < ISR_TABLE_LEN; i++, p++) {
		if (p->id != id) {
			continue;
		}

		memset(p, 0, sizeof(*p));
		p->id = QTI_INTR_INVALID_INT_NUM;
		isr_table.cnt--;

		spin_unlock(&isr_table.lock);
		return 0;
	}

error:
	spin_unlock(&isr_table.lock);
	return -ENOENT;
}

int qti_interrupt_svc_register(uint32_t id,
				void *(*func)(uint32_t id, void *callback_ctx),
				void *ctx)
{
	union qti_isr_func isr_func = { .id_ctx = func };

	return qti_interrupt_svc_register_common(id, QTI_ISR_HANDLER_ID_CTX,
						 isr_func, ctx);
}

int qti_interrupt_svc_unregister(uint32_t id)
{
	return qti_interrupt_svc_unregister_common(id);
}

int qti_register_intr_type_el3(unsigned int intr,
				void *(*handler)(void *callback_ctx), void *ctx)
{
	union qti_isr_func isr_func = { .ctx_only = handler };

	return qti_interrupt_svc_register_common(intr, QTI_ISR_HANDLER_CTX_ONLY,
						 isr_func, ctx);
}

void qti_unregister_intr_type_el3(unsigned int intr)
{
	(void)qti_interrupt_svc_unregister_common(intr);
}

void qti_interrupt_svc_dispatch(uint32_t id, void *handle)
{
	struct qti_isr *p = isr_table.entry;
	enum qti_isr_handler_type type;
	union qti_isr_func func;
	void *ctx = NULL;
	bool found = false;

	spin_lock(&isr_table.lock);

	if (!isr_table.cnt) {
		goto plat_dispatch;
	}

	for (size_t i = 0; i < ISR_TABLE_LEN; i++, p++) {
		if (p->id != id) {
			continue;
		}

		type = p->type;
		func = p->func;
		ctx = p->ctx;
		found = true;
		break;
	}

	if (!found) {
		goto plat_dispatch;
	}

	spin_unlock(&isr_table.lock);

	if (type == QTI_ISR_HANDLER_ID_CTX) {
		p->ctx = func.id_ctx(id, p->ctx);
	} else {
		(void)func.ctx_only(ctx);
	}

	return;

plat_dispatch:
	spin_unlock(&isr_table.lock);
	plat_qti_invoke_unhandled_isr(id, handle);
}

int qti_interrupt_svc_init(bool have_sel1)
{
	uint64_t flags = 0U;

	/*
	 * Route EL3 interrupts to EL3 when in Non-secure.
	 * Note: EL3 won't have interrupts enabled.
	 * When we have a Secure EL1 interrupt handler, allow it
	 * to handle Secure interrupts.
	 */
	set_interrupt_rm_flag(flags, NON_SECURE);
	if (!have_sel1) {
		set_interrupt_rm_flag(flags, SECURE);
	}

	qti_interrupt_svc_register_el3_owner(flags);

	return 0;
}
