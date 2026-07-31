/*
 * Copyright (c) 2015-2024, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2024, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <drivers/arm/gicv3.h>

#include <platform.h>
#include <platform_def.h>
#include <qcom_plat.h>

/* The GICv3 driver only needs to be initialized in EL3 */
static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

static gicv3_driver_data_t qcom_gic_data = {
	.gicd_base = QCOM_GICD_BASE,
	.gicr_base = QCOM_GICR_BASE,
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = plat_qcom_core_pos_by_mpidr
};

void plat_qcom_gic_driver_init(void)
{
	qcom_gic_data.interrupt_props =
		plat_qcom_get_interrupt_props(&qcom_gic_data.interrupt_props_num);

	/*
	 * The GICv3 driver is initialized in EL3 and does not need
	 * to be initialized again in SEL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
	gicv3_driver_init(&qcom_gic_data);
}

/******************************************************************************
 * ARM common helper to initialize the GIC. Only invoked by BL31
 *****************************************************************************/
void plat_qcom_gic_init(void)
{
	unsigned int i;

	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());

	/* Route secure spi interrupt to ANY. */
	for (i = 0; i < qcom_gic_data.interrupt_props_num; i++) {
		unsigned int int_id = qcom_gic_data.interrupt_props[i].intr_num;

		if (plat_ic_is_spi(int_id)) {
			gicv3_set_spi_routing(int_id, GICV3_IRM_ANY, 0x0);
		}
	}
}

void gic_set_spi_routing(unsigned int id, unsigned int irm, u_register_t target)
{
	gicv3_set_spi_routing(id, irm, target);
}

/******************************************************************************
 * ARM common helper to enable the GIC CPU interface
 *****************************************************************************/
void plat_qcom_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helper to disable the GIC CPU interface
 *****************************************************************************/
void plat_qcom_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helper to initialize the per-CPU redistributor interface in GICv3
 *****************************************************************************/
void plat_qcom_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helpers to power GIC redistributor interface
 *****************************************************************************/
void plat_qcom_gic_redistif_on(void)
{
	gicv3_rdistif_on(plat_my_core_pos());
}

void plat_qcom_gic_redistif_off(void)
{
	gicv3_rdistif_off(plat_my_core_pos());
}
