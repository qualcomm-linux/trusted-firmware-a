/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_INTERRUPT_PROPS_H
#define QTI_INTERRUPT_PROPS_H

#include <bl31/interrupt_mgmt.h>
#include <common/interrupt_props.h>

/*
 * Array of interrupts to be configured by the GIC driver.
 *
 * All EL3 interrupts must be added here to configure the interrupt
 * priority, group and type in early platform initialization.
 *
 * This does not enable the interrupt or configure the interrupt target.
 * Interrupt handler registration, etc. must be done at runtime.
 *
 * To add a new interrupt here, use INTR_PROP_DESC with the following
 * arguments:
 *
 * - Interrupt number
 * - Interrupt priority: typically GIC_HIGHEST_SEC_PRIORITY
 *   - For EL3 interrupts, a number between GIC_HIGHEST_SEC_PRIORITY and
 *     GIC_HIGHEST_NS_PRIORITY.
 *   - The higher the number, the lower the priority.
 *
 * - Interrupt type: typically INTR_TYPE_EL3
 *   - INTR_TYPE_EL3 for an EL3 interrupt
 *   - INTR_TYPE_S_EL1 for Secure Group 1
 *   - INTR_TYPE_NS for Non-Secure Group 1
 *
 * - Interrupt configuration: GIC_INTR_CFG_LEVEL or GIC_INTR_CFG_EDGE
 */
static const interrupt_prop_t qti_interrupt_props[] = {
	/* SGI(8) to turn CPU on */
	INTR_PROP_DESC(8,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_EDGE),
	/* CP15 timer */
	INTR_PROP_DESC(29,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* TIMER_SEC_QTMR_INT_ID QTMR_qgicFrm3PhyIrq[0] */
	INTR_PROP_DESC(42,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* TIMER_SEC_QTMR_FR3_INT_ID QTMR_qgicFrm3PhyIrq[0] */
	INTR_PROP_DESC(43,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* TIMER_SEC_QTMR_FR4_INT_ID QTMR_qgicFrm3PhyIrq[0] */
	INTR_PROP_DESC(44,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* TIMER_SEC_QTMR_FR5_INT_ID QTMR_qgicFrm3PhyIrq[0] */
	INTR_PROP_DESC(45,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
};

#endif /* QTI_INTERRUPT_PROPS_H */
