/*
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

#ifndef QTI_INTERRUPT_SVC_H
#define QTI_INTERRUPT_SVC_H

#define MAX_INTR_EL3 64

/*
 * Type definition for an interrupt handling function.
 * void *fn(void *ctx)
 *
 * @param[in]    ctx         Pointer to the context provided when registering
 *                           the interrupt @see int_register_isr
 *
 * @return  Currently unused pointer to return data.
 *          Interrupt handlers may return NULL.
 */
typedef void *(*qti_interrupt_handler_t)(void *ctx);

/*
 * Initialize the EL3 Interrupt Service.
 *
 * @return 0 on success, error code on failure.
 */
int qti_interrupt_svc_init(void);

/*
 * Register an interrupt handler for EL3 (Group 0).
 *
 * @param[in]    intr        Interrupt number
 * @param[in]    handler     Pointer to interrupt handling function
 * @param[in]    ctx         Pointer to context to pass to interrupt handler
 *
 * @return 0 on success, error code on failure.
 */
int qti_register_intr_type_el3(unsigned int intr,
			       qti_interrupt_handler_t handler, void *ctx);

/*
 * Unregister an interrupt handler for EL3 (Group 0).
 *
 * @param[in]    intr        Interrupt number
 *
 * @return 0 on success, error code on failure.
 */
void qti_unregister_intr_type_el3(unsigned int intr);

/*
 * Disable an EL3 (Group 0) interrupt.
 *
 * @param[in]    intr        Interrupt number
 */
void qti_disable_intr_type_el3(unsigned int intr);

/*
 * Enable an EL3 (Group 0) interrupt.
 *
 * @param[in]    intr        Interrupt number
 */
void qti_enable_intr_type_el3(unsigned int intr);

/*
 * Entry point to EL3 common interrupt handler for SPD
 * when dealing with a managed IRQ exit from SEL1
 * (when EL3 interrupt occurs while SEL1 is executing)
 *
 * @param[in]    handle      Pointer to secure world CPU context
 */
void qti_el3_interrupt_handler_to_ns(void *handle);

#endif /* QTI_INTERRUPT_SVC_H */
