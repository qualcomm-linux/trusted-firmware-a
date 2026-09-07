# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
#
# Make for Nord QTI platform.

PLAT_QTI_ROOT		:=	plat/qti
QTI_PLAT_PATH		:=	plat/qti/wildcat
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

#Enable errata configs for cortex_a78 and cortex_a55
#ERRATA_A55_1530923 		:=	1
#ERRATA_A78_1941498 		:=	1
#ERRATA_A78_1951500 		:=	1
#ERRATA_A78_2132060 		:=	1

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
ENABLE_FEAT_RNG := 1

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

# Enable assertiong.
ENABLE_ASSERTIONS		:=	1

# Enable backtrace dumps.
ENABLE_BACKTRACE		:=	1

QTI_EXTERNAL_INCLUDES	:=	-I${QTI_PLAT_PATH}/${CHIPSET}/inc			\
				-I${QTI_PLAT_PATH}/common/inc				\
				-I${QTI_PLAT_PATH}/common/inc/$(ARCH)			\
				-I${PLAT_QTI_ROOT}/common/inc/$(ARCH)			\
				-I${QTI_PLAT_PATH}/bl31qtilib/inc			\
				-I${PLAT_QTI_ROOT}/common/inc				\

QTI_BL31_SOURCES	:=	$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_helpers.S	\
				$(PLAT_QTI_ROOT)/common/src/$(ARCH)/qti_uart_console.S	\
				$(PLAT_QTI_ROOT)/common/src/$(ARCH)/qti_ringbuf_console.S	\
				$(PLAT_QTI_ROOT)/common/src/qti_ringbuf_console_helper.c	\
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_pauth.c         \
				$(QTI_PLAT_PATH)/common/src/$(ARCH)/qti_unhandled_exception_asm.S \
				$(PLAT_QTI_ROOT)/common/src/qti_stack_protector.c	\
				$(QTI_PLAT_PATH)/common/src/wildcat_bl31_setup.c	\
				$(QTI_PLAT_PATH)/common/src/wildcat_common.c		\
				$(PLAT_QTI_ROOT)/common/src/qti_gic_v3.c		\
				$(PLAT_QTI_ROOT)/common/src/qti_interrupt_svc.c		\
				$(QTI_PLAT_PATH)/common/src/qti_syscall.c		\
				$(QTI_PLAT_PATH)/common/src/qti_err_log.c		\
				$(QTI_PLAT_PATH)/common/src/qti_tlb.c			\
				$(QTI_PLAT_PATH)/common/src/qti_topology.c		\
				$(QTI_PLAT_PATH)/common/src/qti_pm.c			\
				$(QTI_PLAT_PATH)/common/src/qti_rng.c			\
				$(PLAT_QTI_ROOT)/common/src/spmi_arb.c			\
				$(QTI_PLAT_PATH)/bl31qtilib/src/bl31qtilib_cb_interface.c	\
				$(QTI_PLAT_PATH)/common/src/qti_plat_helpers.c \
				drivers/qti/crypto/rng.c


# Ensure Widevine is not being used
ifeq ($(CROS_WIDEVINE_SMC), 0)
QTI_BL31_SOURCES		+=	$(QTI_PLAT_PATH)/common/src/qti_oem_svc.c	\
					$(QTI_PLAT_PATH)/common/src/qti_fuseprov_platform.c
endif

PLAT_INCLUDES		:=	-Iinclude/plat/common/					\

PLAT_INCLUDES		+=	${QTI_EXTERNAL_INCLUDES}

include lib/xlat_tables_v2/xlat_tables.mk
include drivers/qti/smem/smem.mk
include drivers/qti/chipinfo/chipinfo.mk
include drivers/qti/qgic/qgic.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}					\
				plat/common/aarch64/crash_console_helpers.S    \
				common/desc_image_load.c			\
				lib/bl_aux_params/bl_aux_params.c		\

include lib/coreboot/coreboot.mk

#### GIC related configuration ####
# GIC-700 is based on GICv4.1
GIC_ENABLE_V4_EXTN	:=	1
# GIC support extended PPI and SPI interrupt ranges
GIC_EXT_INTID=1
# GIC-600 configuration
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

CPU_SOURCES		:=	$(QTI_PLAT_PATH)/common/src/aarch64/cortex_phoenix.S

BL31_SOURCES		+=	${QTI_BL31_SOURCES}				\
				${GIC_SOURCES}					\
				${TIMER_SOURCES}				\
				${CPU_SOURCES}					\

BL31_SOURCES		+=	${QGIC_DRV_PATH}/qgic_intr_el3.c

LIB_QTI_PATH	:=	${QTI_PLAT_PATH}/bl31qtilib/lib/${CHIPSET}

# Override this on the command line to point to the bl31qtilib library
BL31QTILIB_PATH ?=
ifeq ($(BL31QTILIB_PATH),)
# if No lib then use stub implementation for bl31qtilib interface
$(warning BL31QTILIB_PATH is not provided while building, using stub \
		implementation. THIS FIRMWARE WILL NOT BOOT!)
BL31_SOURCES	+=	${QTI_PLAT_PATH}/bl31qtilib/src/bl31qtilib_interface_stub.c \
			${QTI_PLAT_PATH}/bl31qtilib/src/bl31qtilib_version_strings_stub.c

PLAT_INCLUDES	+=	-Iinclude/drivers/qti/qtimer/${CHIPSET} \
			-Iinclude/drivers/qti/watchdog/${CHIPSET}

QTI_USE_QTIMER		:=	1
QTI_USE_NCC_QTIMER	:=	1
$(eval $(call add_define,QTI_USE_NCC_QTIMER))

# Select the NORD secure watchdog implementation.
QTI_WDOG_VARIANT	?=	windowed
ifeq ($(QTI_WDOG_VARIANT),apsec)
$(eval $(call add_define,WDOG_USE_APSEC))
QTI_WDOG_VER_SRC	:=	drivers/qti/watchdog/v2/watchdog_ver.c
else ifeq ($(QTI_WDOG_VARIANT),windowed)
QTI_WDOG_VER_SRC	:=	drivers/qti/watchdog/v2/watchdog_ver_windowed.c
else
$(error QTI_WDOG_VARIANT must be windowed or apsec, got $(QTI_WDOG_VARIANT))
endif

BL31_SOURCES	+=	drivers/qti/qtimer/qtimer.c \
			drivers/qti/qtimer/qtimer_ncc.c \
			$(QTI_PLAT_PATH)/common/src/qti_qtimer_platform.c \
			drivers/qti/watchdog/watchdog.c \
			$(QTI_WDOG_VER_SRC) \
			$(QTI_PLAT_PATH)/common/src/qti_watchdog_platform.c
else
# use library provided by BL31QTILIB_PATH
LDFLAGS += -L $(dir $(BL31QTILIB_PATH))
LDLIBS += -l$(patsubst lib%.a,%,$(notdir $(BL31QTILIB_PATH)))

endif

# Override this on the command line to point to the SysINI library.
SYSINIQTILIB_PATH ?= $(QTI_PLAT_PATH)/sysini/lib/$(CHIPSET)/libsysini.a
ifeq ($(SYSINIQTILIB_PATH),)
$(error Please provide path to libsysini.a in SYSINIQTILIB_PATH)
else
$(eval $(call add_define,QTI_USE_SYSINI_LIB))
LDFLAGS += -L $(dir $(SYSINIQTILIB_PATH))
LDLIBS += -l$(patsubst lib%.a,%,$(notdir $(SYSINIQTILIB_PATH)))
endif

# Always use the SPD-agnostic adapter for NORD. These stubs do not forward
# BL31 QTI calls to the selected SPD.
BL31_SOURCES +=	${QTI_PLAT_PATH}/bl31qtilib/src/bl31qtilib_spd_agnostic_stub.c \
			${QTI_PLAT_PATH}/bl31qtilib/src/bl31qtilib_spd_agnostic_panic_stub.S

# QCOM MBOX
QCOM_MBOX	:=	1
QCOM_MBOX_QMP	:=	1

include drivers/qti/mbox/mbox.mk
PLAT_INCLUDES	+=	-Idrivers/qti/mbox/
BL31_SOURCES	+=	${QTI_PLAT_PATH}/$(CHIPSET)/src/qcom_mbox_plat.c

# TMECOM
QTI_TMECOM	:=	1
$(eval $(call add_define,QTI_USE_TMECOM))
$(eval $(call add_define,QTI_TMECOM_TEST))
include drivers/qti/tme/tme.mk

# Fuseprov driver: parses SEC.DAT and blows fuses via the TME transport.
include drivers/qti/fuseprov/fuseprov.mk

include $(QTI_PLAT_PATH)/common/common.mk
