/*
 * Copyright (c) 2018,2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCOM_INTERRUPT_SVC_H
#define QCOM_INTERRUPT_SVC_H

typedef  void* (*qcom_int_svc_isr_t)(uint32_t id, void *ctx);

int qcom_interrupt_svc_register(uint32_t id, qcom_int_svc_isr_t func, void *ctx);
int qcom_interrupt_svc_unregister(uint32_t id);
int qcom_interrupt_svc_init(bool have_sel1);

#endif /* QCOM_INTERRUPT_SVC_H */
