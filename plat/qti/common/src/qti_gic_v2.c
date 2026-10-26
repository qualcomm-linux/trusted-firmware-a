/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Changes from Qualcomm Technologies, Inc. are provided under the following license:
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include <common/bl_common.h>
#include <drivers/arm/gicv2.h>
#include <platform.h>
#include <platform_def.h>
#include <qti_plat.h>

static unsigned int target_mask_array[PLATFORM_CORE_COUNT];

static gicv2_driver_data_t qti_gic_data = {
	.gicd_base = QTI_GICD_BASE,
	.gicc_base = QTI_GICC_BASE,
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};

void gic_set_spi_routing(unsigned int id, unsigned int irm, u_register_t target)
{
	gicv2_set_spi_routing(id, (int)target);
}

void plat_qti_gic_driver_init(void)
{
	/*
	 * Wire the platform's secure interrupt property table (SGIs/SPIs,
	 * e.g. the secure watchdog bark SPI) into the driver data so
	 * gicv2_distif_init()/gicv2_pcpu_distif_init() actually configure
	 * their group/priority/enable bits. Without this, interrupt_props
	 * stays NULL/0 and those interrupts are silently never enabled.
	 */
	qti_gic_data.interrupt_props =
		plat_qti_get_interrupt_props(&qti_gic_data.interrupt_props_num);

	/*
	 * The GICv2 driver is initialized in EL3 and does not need
	 * to be initialized again in SEL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
	gicv2_driver_init(&qti_gic_data);
}

/******************************************************************************
 * ARM common helper to initialize the GIC. Only invoked by BL31
 *****************************************************************************/
void plat_qti_gic_init(void)
{
	const interrupt_prop_t *interrupt_props;
	unsigned int interrupt_props_num;
	unsigned int i;

	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());
	gicv2_cpuif_enable();

	interrupt_props = plat_qti_get_interrupt_props(&interrupt_props_num);

	/* Route secure spi interrupt to ANY. */
	for (i = 0; i < interrupt_props_num; i++) {
		unsigned int int_id = interrupt_props[i].intr_num;

		if (plat_ic_is_spi(int_id)) {
			gicv2_set_spi_routing(int_id, -1);
		}
	}
}

/******************************************************************************
 * ARM common helper to enable the GIC CPU interface
 *****************************************************************************/
void plat_qti_gic_cpuif_enable(void)
{
	gicv2_cpuif_enable();
	gicv2_set_pe_target_mask(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helper to disable the GIC CPU interface
 *****************************************************************************/
void plat_qti_gic_cpuif_disable(void)
{
	gicv2_cpuif_disable();
}

/******************************************************************************
 * ARM common helper to initialize the per-CPU distributor interface in GICv2
 *****************************************************************************/
void plat_qti_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());
}
