# Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
#
# Make for Nord QTI platform.

PLAT_QTI_ROOT		:=	plat/qti
QTI_PLAT_PATH		:=	plat/qti/wildcat
QTI_COMMON_PATH		:=	plat/qti/common
CHIPSET			:=	nord
PLATFORM		:=	nord

ARM_ARCH_MAJOR := 9

# Turn On Separate code & data.
SEPARATE_CODE_AND_RODATA	:=	1
USE_COHERENT_MEM		:=	1
WARMBOOT_ENABLE_DCACHE_EARLY	:=	1
HW_ASSISTED_COHERENCY		:=	0

# Disable console crash reporting
CRASH_REPORTING			:= 	0

BRANCH_PROTECTION := 1

ENABLE_FEATURE_PAUTH := 1
FEATURE_DETECTION := 1

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT		:=	0

# Enable PSCI v1.0 extended state ID format
PSCI_EXTENDED_STATE_ID	:=  1
ARM_RECOM_STATE_ID_ENC  :=  1
PSCI_OS_INIT_MODE	:=  1

COLD_BOOT_SINGLE_CPU		:=	1
PROGRAMMABLE_RESET_ADDRESS	:=	1

RESET_TO_BL31			:=	0

MULTI_CONSOLE_API		:=	1

# Support for TRNG SVC
TRNG_SUPPORT := 1

# Use the SoC QRNG for early boot entropy.
$(eval $(call add_define, QTI_USE_SEC_PRNG))

# Enable RNDR RNG feature
ENABLE_FEAT_RNG := 0

# Enabled HCX
ENABLE_FEAT_HCX := 1
# Enabled AMU
ENABLE_FEAT_AMU := 1
# Enable FGT
ENABLE_FEAT_FGT := 1
# Enable ECV
ENABLE_FEAT_ECV := 1
# Disable TRF access for NS
ENABLE_TRF_FOR_NS := 0

# Overriding default value, so that EL1 context registers are initialized
SPMD_SPM_AT_SEL2 := 0
ENABLE_FEAT_SEL2 := 0

# HWTRACE Support
QTI_HWTRACE_SUPPORT := 0

QTI_SDI_BUILD := 1

QTI_UART_CONSOLE := ${DEBUG}

CTX_INCLUDE_AARCH32_REGS		:=	0

# Disable WORKAROUND_CVE_2017_5715
WORKAROUND_CVE_2017_5715		:=	0
# Enable WORKAROUND_CVE_2018_3639
WORKAROUND_CVE_2018_3639		:=	1
# Enable DYNAMIC_WORKAROUND_CVE_2018_3639
DYNAMIC_WORKAROUND_CVE_2018_3639	:=	1
# Disable WORKAROUND_CVE_2022_23960
WORKAROUND_CVE_2022_23960		:=	0
# Disable WORKAROUND_CVE_2024_5660
WORKAROUND_CVE_2024_5660		:=	0

# Disable Widewine
CROS_WIDEVINE_SMC 			:=	0

# Enable runtime debug configuration (allows platform to control debug
# settings at runtime)
PLAT_RUNTIME_DEBUG_CFG		:= 1

# Enable stack protector.
ENABLE_STACK_PROTECTOR := strong

# Enable assertions.
ENABLE_ASSERTIONS		:=	1

# Enable backtrace dumps.
ENABLE_BACKTRACE		:=	1

QTI_EXTERNAL_INCLUDES	:=	-I${QTI_PLAT_PATH}/${CHIPSET}/inc			\
				-I${QTI_PLAT_PATH}/common/inc/$(ARCH)			\
				-I${QTI_COMMON_PATH}/inc/$(ARCH)			\
				-I${QTI_COMMON_PATH}/inc				\
				-I${QTI_PLAT_PATH}/common/inc				\

QTI_BL31_SOURCES	:=	$(QTI_PLAT_PATH)/common/src/$(ARCH)/wildcat_helpers.S	\
				$(PLAT_QTI_ROOT)/common/src/$(ARCH)/qti_uart_console.S	\
				$(PLAT_QTI_ROOT)/common/src/$(ARCH)/qti_ringbuf_console.S	\
				$(PLAT_QTI_ROOT)/common/src/qti_ringbuf_console_helper.c	\
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/wildcat_pauth.c     \
				$(QTI_COMMON_PATH)/src/pm_ps_hold.c			\
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/wildcat_unhandled_exception_asm.S \
				$(PLAT_QTI_ROOT)/common/src/qti_stack_protector.c	\
				$(QTI_COMMON_PATH)/src/qti_bl31_setup.c		\
				$(QTI_COMMON_PATH)/src/qti_common.c			\
				$(QTI_COMMON_PATH)/src/qti_interrupt_svc_el3.c	\
				$(PLAT_QTI_ROOT)/common/src/qti_gic_v3.c		\
				$(PLAT_QTI_ROOT)/common/src/qti_interrupt_svc.c		\
				$(QTI_COMMON_PATH)/src/qti_syscall.c			\
				$(QTI_COMMON_PATH)/src/qti_sip_platform.c	\
				$(QTI_COMMON_PATH)/src/qti_pm.c			\
				$(QTI_PLAT_PATH)/common/src/wildcat_topology.c	\
				$(QTI_PLAT_PATH)/common/src/wildcat_pm.c \
				$(QTI_PLAT_PATH)/common/src/wildcat_rng.c		\
				$(QTI_COMMON_PATH)/src/spmi_arb.c			\
				$(QTI_PLAT_PATH)/common/src/wildcat_plat_helpers.c \
				$(QTI_PLAT_PATH)/common/src/wildcat_bl31_setup.c \
				$(QTI_PLAT_PATH)/${CHIPSET}/src/plat_cpuss_config.c \
				drivers/qti/accesscontrol/access_control_stub.c \
				drivers/qti/sec_core/sec_core_stub.c \
				drivers/qti/crypto/rng.c

PLAT_INCLUDES		:=	-Iinclude/plat/common/					\

PLAT_INCLUDES		+=	${QTI_EXTERNAL_INCLUDES}
PLAT_INCLUDES		+=	-I${QTI_COMMON_PATH}/inc/$(ARCH)

include lib/xlat_tables_v2/xlat_tables.mk
include drivers/qti/smem/smem.mk
include drivers/qti/chipinfo/chipinfo.mk
PLAT_INCLUDES += -Iinclude/drivers/qti/qtimer/${CHIPSET}

QTI_USE_NCC_QTIMER	:=	1
$(eval $(call add_define,QTI_USE_NCC_QTIMER))
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}					\
				plat/common/aarch64/crash_console_helpers.S    \
				common/desc_image_load.c			\
				lib/bl_aux_params/bl_aux_params.c		\

include lib/coreboot/coreboot.mk

#### GIC related configuration ####
# GIC-700 is based on GICv4.1
GIC_ENABLE_V4_EXTN	:=	1
# GIC support extended PPI and SPI interrupt ranges
GIC_EXT_INTID		:=	1
# GIC-700 uses the GIC-600 driver with extended INTID support
GICV3_SUPPORT_GIC600	:=	1
# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

#GIC sources.
GIC_SOURCES		:=	plat/common/plat_gicv3.c			\
				${GICV3_SOURCES}
#### End GIC related configuration ####

#Timer sources
TIMER_SOURCES		:=	drivers/delay_timer/generic_delay_timer.c	\
				drivers/delay_timer/delay_timer.c		\

QTI_NCC_CPU		:= 1

#Disable SMCCC_ARCH_SOC_ID support to ensure legacy Linux kernel socinfo
#driver can expose older soc_id format
$(eval $(call add_define, QTI_NO_SMCC_ARCH_SOC_ID))

CPU_SOURCES		:=	$(QTI_PLAT_PATH)/common/src/aarch64/qcom_oryon_v1.S

BL31_SOURCES		+=	${QTI_BL31_SOURCES}				\
				${GIC_SOURCES}					\
				${TIMER_SOURCES}				\
				${CPU_SOURCES}					\

BL31_SOURCES += drivers/qti/qtimer/qtimer.c \
			drivers/qti/qtimer/qtimer_ncc.c \
			$(PLAT_QTI_ROOT)/common/src/qti_qtimer_platform.c


QTI_WATCHDOG_ENABLE := 1
QTI_WDOG_DEFS := nord
QTI_WDOG_VARIANT ?= windowed
QTI_WDOG_PLATFORM_SRC := $(QTI_PLAT_PATH)/common/src/qti_watchdog_platform.c

include $(QTI_PLAT_PATH)/common/wildcat_common.mk

include drivers/qti/watchdog/watchdog.mk
