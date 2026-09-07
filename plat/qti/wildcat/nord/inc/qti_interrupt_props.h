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

enum qti_interrupt_owner {
	QTI_INTR_OWNER_EL3 = 0,
	QTI_INTR_OWNER_SECURE_DISPATCHER,
};

struct qti_interrupt_prop_owner {
	interrupt_prop_t prop;
	enum qti_interrupt_owner owner;
};

#define QTI_INTR_EL3(num, pri, grp, cfg)	\
	{ INTR_PROP_DESC(num, pri, grp, cfg), QTI_INTR_OWNER_EL3 }

#define QTI_INTR_SECURE_DISPATCHER(num, pri, grp, cfg)	\
	{ INTR_PROP_DESC(num, pri, grp, cfg),		\
	  QTI_INTR_OWNER_SECURE_DISPATCHER }

static const struct qti_interrupt_prop_owner qti_interrupt_props[] = {
	QTI_INTR_EL3(29, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	QTI_INTR_EL3(72, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	QTI_INTR_EL3(73, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	QTI_INTR_EL3(74, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	QTI_INTR_EL3(75, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	QTI_INTR_EL3(80, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		GIC_INTR_CFG_EDGE),
	QTI_INTR_EL3(4943, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		GIC_INTR_CFG_EDGE),
	QTI_INTR_EL3(4946, GIC_HIGHEST_SEC_PRIORITY, INTR_TYPE_EL3,
		GIC_INTR_CFG_EDGE),
};

#endif /* QTI_INTERRUPT_PROPS_H */
