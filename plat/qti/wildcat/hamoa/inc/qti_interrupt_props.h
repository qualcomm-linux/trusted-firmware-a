/**********************************************************************
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *********************************************************************/
#ifndef QTI_INTERRUPT_PROPS_H
#define QTI_INTERRUPT_PROPS_H

#include <bl31/interrupt_mgmt.h>
#include <common/interrupt_props.h>

/*
 * Array of interrupts to be configured by the gic driver
 *
 * NOTE: all EL3 interrupts must be added here in order to
 *
 * to configure the interrupt priority, group and type (level/edge)
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
	/* SGI(8) to Turn CPU ON */
	QTI_INTR_EL3(8,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_EDGE),
#ifdef QTI_BL31_WITH_TEST
	/* SGI(11) is unused in QTEE */
	QTI_INTR_EL3(11,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_EDGE),
#endif
	/* AC_XPU_SEC_INTR */
	QTI_INTR_SECURE_DISPATCHER(195,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_EDGE),
	/* AC_XPU_RA_INTR */
	QTI_INTR_SECURE_DISPATCHER(5068,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_EDGE),
	/* SPI VMIDMT ERR Interrupt */
	QTI_INTR_SECURE_DISPATCHER(198,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_EDGE),

	/* ICB NOC Error Interrupts */
	/* a1noc_obs_mainFault, */
	QTI_INTR_SECURE_DISPATCHER(201,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* a2noc_obs_mainFault, */
	QTI_INTR_SECURE_DISPATCHER(234,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* apss_noc_irq, */
	QTI_INTR_SECURE_DISPATCHER(54,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* cnoc_sbm_Fault, */
	QTI_INTR_SECURE_DISPATCHER(226,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* dc_noc_obs_mainFault, */
	QTI_INTR_SECURE_DISPATCHER(290,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* ddrss_apps_interrupt[7], */
	QTI_INTR_SECURE_DISPATCHER(299,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* gem_noc_mainFault, */
	QTI_INTR_SECURE_DISPATCHER(108,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* snoc_sbm_Fault, */
	QTI_INTR_SECURE_DISPATCHER(200,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* lpass_ag_noc_sidebandmanagerfault, */
	QTI_INTR_SECURE_DISPATCHER(323,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* mnoc_obs_mainFault, */
	QTI_INTR_SECURE_DISPATCHER(186,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* pcie_anoc_obs_mainFault, */
	QTI_INTR_SECURE_DISPATCHER(238,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* nsi_noc_irq, */
	QTI_INTR_SECURE_DISPATCHER(53,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* nsp_Fault, */
	QTI_INTR_SECURE_DISPATCHER(328,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* lpi_noc_SidebandManagerFault, */
	QTI_INTR_SECURE_DISPATCHER(730,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),

	/* ICB DDRSS Error Interrupts */
	/* ddrss_apps_interrupt[2] */
	QTI_INTR_SECURE_DISPATCHER(614,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* ddrss_apps_interrupt[4] */
	QTI_INTR_SECURE_DISPATCHER(257,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* CP15 timer */
	QTI_INTR_EL3(29,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* TIMER_SEC_QTMR_INT_ID QTMR_qgicFrm3PhyIrq[0] */
	QTI_INTR_EL3(42,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* TIMER_SEC_QTMR_FR3_INT_ID QTMR_qgicFrm3PhyIrq[0] */
	QTI_INTR_EL3(43,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* TIMER_SEC_QTMR_FR4_INT_ID QTMR_qgicFrm3PhyIrq[0] */
	QTI_INTR_EL3(44,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
	/* TIMER_SEC_QTMR_FR5_INT_ID QTMR_qgicFrm3PhyIrq[0] */
	QTI_INTR_EL3(45,
		GIC_HIGHEST_SEC_PRIORITY,
		INTR_TYPE_EL3,
		GIC_INTR_CFG_LEVEL),
};

#endif /* QTI_INTERRUPT_PROPS_H */
