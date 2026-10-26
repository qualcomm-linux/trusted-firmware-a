#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
# Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
# Changes from Qualcomm Technologies, Inc. are provided under the following license:
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Make for ipq52xx QTI platform.

QTI_PLAT_PATH		:=	plat/qti
QTI_ARCH_PATH		:=	${QTI_PLAT_PATH}/bobcat
CHIPSET			:=	${PLAT}

# Turn On Separate code & data.
SEPARATE_CODE_AND_RODATA	:=	1
USE_COHERENT_MEM		:=	0
WARMBOOT_ENABLE_DCACHE_EARLY	:=	1

# Enable errata for Kryo-3 Silver (Cortex-A53).
# MIDR_EL1 = 0x51AF8014 (variant 0xA, revision 4). Hermosa uses Cortex-A53
# r0p4 as the errata baseline.
ERRATA_A53_835769		:=	1
ERRATA_A53_843419		:=	1
ERRATA_A53_855873		:=	1
ERRATA_A53_1530924		:=	1

# Enable PSCI v1.0 extended state ID format
PSCI_EXTENDED_STATE_ID	:=	1
ARM_RECOM_STATE_ID_ENC	:=	1
PSCI_OS_INIT_MODE	:=	1

COLD_BOOT_SINGLE_CPU		:=	1
PROGRAMMABLE_RESET_ADDRESS	:=	1

# GIC driver version used by the platform
QTI_USE_GIC_DRIVER		:=	2
$(eval $(call add_define,QTI_USE_GIC_DRIVER))

INIT_UNUSED_NS_EL2		:=	1

# Stack protector stays disabled until an RNG source is ported for ipq52xx
ENABLE_STACK_PROTECTOR	:=	0

# Enable platform-specific linker script for IMEM region
PLAT_EXTRA_LD_SCRIPT := 1
$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

QTI_EXTERNAL_INCLUDES	:=	-I${QTI_ARCH_PATH}/${CHIPSET}/inc			\
				-I${QTI_PLAT_PATH}/common/inc				\
				-I${QTI_PLAT_PATH}/common/inc/$(ARCH)

QTI_BL31_SOURCES	:=	$(QTI_ARCH_PATH)/common/$(ARCH)/bobcat_helpers.S	\
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_kryo3_silver.S	\
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_uart_console.S	\
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_ringbuf_console.S	\
				$(QTI_PLAT_PATH)/common/src/qti_ringbuf_console_helper.c	\
				$(QTI_ARCH_PATH)/common/src/bobcat_common.c		\
				$(QTI_ARCH_PATH)/common/src/bobcat_bl31_setup.c	\
				$(QTI_PLAT_PATH)/common/src/qti_gic_v2.c		\
				$(QTI_ARCH_PATH)/${CHIPSET}/ipq52xx_gicv2.c		\
				$(QTI_ARCH_PATH)/common/src/bobcat_topology.c		\
				$(QTI_ARCH_PATH)/common/src/bobcat_syscall.c		\
				$(QTI_PLAT_PATH)/common/src/qti_pm.c			\
				$(QTI_ARCH_PATH)/common/src/bobcat_pm.c		\
				drivers/qti/sec_core/sec_core.c			\
				drivers/qti/sec_core/${CHIPSET}/sec_core_cfg.c		\
				drivers/qti/watchdog/apss_watchdog.c			\
				drivers/qti/clock/${CHIPSET}/clock_init.c		\
				drivers/qti/qtimer/qtimer.c

PLAT_INCLUDES		:=	-Iinclude/plat/common/

PLAT_INCLUDES		+=	${QTI_EXTERNAL_INCLUDES}
PLAT_INCLUDES		+=	-Iinclude/drivers/qti/qtimer/${CHIPSET}
PLAT_INCLUDES		+=	-Iinclude/drivers/qti/watchdog/${CHIPSET}
PLAT_INCLUDES		+=	-Iinclude/drivers/qti/sec_core/${CHIPSET}

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}				\
				plat/common/aarch64/crash_console_helpers.S	\
				common/desc_image_load.c

# PSCI Sources.
PSCI_SOURCES		:=	plat/common/plat_psci_common.c

# Timer sources
TIMER_SOURCES		:=	drivers/delay_timer/generic_delay_timer.c	\
				drivers/delay_timer/delay_timer.c

# Include GICv2 driver files
include drivers/arm/gic/v2/gicv2.mk
# GIC sources.
GIC_SOURCES		:=	plat/common/plat_gicv2.c			\
				${GICV2_SOURCES}

# CPU sources
CPU_SOURCES		:=	lib/cpus/aarch64/cortex_a53.S

# Prohibit using deprecated interfaces. We rely on this for this platform.
ERROR_DEPRECATED	:=	1

BL31_SOURCES		+=	${QTI_BL31_SOURCES}				\
				${PSCI_SOURCES}					\
				${GIC_SOURCES}					\
				${TIMER_SOURCES}				\
				${CPU_SOURCES}
