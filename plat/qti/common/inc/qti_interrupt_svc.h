/*
 * Copyright (c) 2018,2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_INTERRUPT_SVC_H
#define QTI_INTERRUPT_SVC_H

#include <stdbool.h>
#include <stdint.h>

typedef void *(*qti_int_svc_isr_t)(uint32_t id, void *ctx);
typedef void *(*qti_interrupt_handler_t)(void *ctx);

int qti_interrupt_svc_register(uint32_t id, qti_int_svc_isr_t func, void *ctx);
int qti_interrupt_svc_unregister(uint32_t id);

int qti_register_intr_type_el3(unsigned int intr,
			       qti_interrupt_handler_t handler, void *ctx);
void qti_unregister_intr_type_el3(unsigned int intr);
void qti_disable_intr_type_el3(unsigned int intr);
void qti_enable_intr_type_el3(unsigned int intr);
void qti_el3_interrupt_handler_to_ns(void *handle);

int qti_interrupt_svc_init(bool have_sel1);

#endif /* QTI_INTERRUPT_SVC_H */
