/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_INTERRUPT_PROPS_H
#define QTI_INTERRUPT_PROPS_H

#include <bl31/interrupt_mgmt.h>
#include <common/interrupt_props.h>

/**
 * Array of interrupts to be configured by the gic driver
 *
 * NOTE: all EL3 interrupts must be added here in order to
 * configure the interrupt priority, group and type (level/edge)
 * in early platform init
 *
 * This does not enable the interrupt or configure the interrupt target.
 * Interrupt handler registration etc must also be done at runtime.
 *
 * To add a new interrupt here, use INIT_PROP_DESC with the following arguments:
 * - Interrupt Number
 * - Interrupt Priority - typically GIC_HIGHEST_SEC_PRIORITY
 *   - For EL3 interrupts, a number between GIC_HIGHEST_SEC_PRIORITY and
 *     GIC_HIGHEST_NS_PRIORITY.
 *   - The higher the number, the lower the priority.
 *
 * - Interrupt Type - typically INTR_TYPE_EL3
 *   - INTR_TYPE_EL3 for an EL3 interrupt
 *   - INTR_TYPE_S_EL1 for Secure Group 1
 *   - INTR_TYPE_NS for Non-Secure Group 1
 *
 * - Interrupt Configuration - GIC_INTR_CFG_LEVEL or GIC_INTR_CFG_EDGE)
 */
static const interrupt_prop_t qti_interrupt_props[] = {
	/* CP15 timer */
	INTR_PROP_DESC(29, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		       GIC_INTR_CFG_LEVEL),
	/* Qtimer frame2 */
	INTR_PROP_DESC(72, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		       GIC_INTR_CFG_LEVEL),
	/* Qtimer frame3 */
	INTR_PROP_DESC(73, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		       GIC_INTR_CFG_LEVEL),
	/* Qtimer frame4 */
	INTR_PROP_DESC(74, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		       GIC_INTR_CFG_LEVEL),
	/* Qtimer frame5 */
	INTR_PROP_DESC(75, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		       GIC_INTR_CFG_LEVEL),
	/* Windowed Secure Wdog bark */
	INTR_PROP_DESC(80, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		       GIC_INTR_CFG_EDGE),
	/* Non-Secure Wdog bite */
	INTR_PROP_DESC(4943, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		       GIC_INTR_CFG_EDGE),
	/* Secure Wdog bark */
	INTR_PROP_DESC(4946, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		       GIC_INTR_CFG_EDGE),
};

#endif /* QTI_INTERRUPT_PROPS_H */
