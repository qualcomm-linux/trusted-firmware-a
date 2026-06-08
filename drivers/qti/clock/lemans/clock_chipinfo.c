/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Chip-info seam for the lemans (qcs9075) clock driver.
 *
 * The clock framework asks for the SoC id, device id and chip (major.minor)
 * revision. These are read from the TCSR SoC-hardware-version and JTAG-id
 * registers, mirroring the vendor chipinfo driver.
 */

#include <stdint.h>

#include <drivers/qti/clock/qti_sec_chipinfo.h>
#include <lib/mmio.h>

/* TCSR SoC hardware version register (CORE_TOP_CSR). */
#define TCSR_SOC_HW_VERSION			0x01FC8000U
#define TCSR_SOC_HW_VERSION_FAMILY_NUMBER_BMSK	0xF0000000U
#define TCSR_SOC_HW_VERSION_FAMILY_NUMBER_SHFT	28U
#define TCSR_SOC_HW_VERSION_DEVICE_NUMBER_BMSK	0x0FFF0000U
#define TCSR_SOC_HW_VERSION_DEVICE_NUMBER_SHFT	16U
#define TCSR_SOC_HW_VERSION_MAJOR_VERSION_BMSK	0x0000FF00U
#define TCSR_SOC_HW_VERSION_MAJOR_VERSION_SHFT	8U
#define TCSR_SOC_HW_VERSION_MINOR_VERSION_BMSK	0x000000FFU
#define TCSR_SOC_HW_VERSION_MINOR_VERSION_SHFT	0U

static uint32_t soc_hw_version_field(uint32_t bmsk, uint32_t shft)
{
	return (mmio_read_32(TCSR_SOC_HW_VERSION) & bmsk) >> shft;
}

uint32_t qti_sec_get_soc_id(void)
{
	return (soc_hw_version_field(TCSR_SOC_HW_VERSION_FAMILY_NUMBER_BMSK,
				     TCSR_SOC_HW_VERSION_FAMILY_NUMBER_SHFT) << 28) |
	       (soc_hw_version_field(TCSR_SOC_HW_VERSION_DEVICE_NUMBER_BMSK,
				     TCSR_SOC_HW_VERSION_DEVICE_NUMBER_SHFT) << 16) |
	       (soc_hw_version_field(TCSR_SOC_HW_VERSION_MAJOR_VERSION_BMSK,
				     TCSR_SOC_HW_VERSION_MAJOR_VERSION_SHFT) << 8) |
	       (soc_hw_version_field(TCSR_SOC_HW_VERSION_MINOR_VERSION_BMSK,
				     TCSR_SOC_HW_VERSION_MINOR_VERSION_SHFT));
}

uint32_t qti_sec_get_device_id(void)
{
	return soc_hw_version_field(TCSR_SOC_HW_VERSION_DEVICE_NUMBER_BMSK,
				    TCSR_SOC_HW_VERSION_DEVICE_NUMBER_SHFT);
}

uint32_t qti_sec_get_chip_version(void)
{
	uint32_t major = soc_hw_version_field(
				TCSR_SOC_HW_VERSION_MAJOR_VERSION_BMSK,
				TCSR_SOC_HW_VERSION_MAJOR_VERSION_SHFT);
	uint32_t minor = soc_hw_version_field(
				TCSR_SOC_HW_VERSION_MINOR_VERSION_BMSK,
				TCSR_SOC_HW_VERSION_MINOR_VERSION_SHFT);

	return CHIPINFO_VERSION(major, minor);
}
