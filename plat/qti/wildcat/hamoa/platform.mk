# Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Make for Hamoa QTI platform.

QTI_PLAT_PATH	:= plat/qti/wildcat
QTI_COMMON_PATH	:= plat/qti/common
CHIPSET		:= hamoa
PLATFORM	:= hamoa

ARM_ARCH_MAJOR := 9

SEPARATE_CODE_AND_RODATA	:= 1
USE_COHERENT_MEM		:= 1
WARMBOOT_ENABLE_DCACHE_EARLY	:= 1
HW_ASSISTED_COHERENCY		:= 0

CRASH_REPORTING			:= 0

BRANCH_PROTECTION := 1

ENABLE_FEATURE_PAUTH := 1

FEATURE_DETECTION := 1

ENABLE_PLAT_COMPAT	:= 0

PSCI_EXTENDED_STATE_ID	:= 1
ARM_RECOM_STATE_ID_ENC	:= 1
PSCI_OS_INIT_MODE	:= 1

COLD_BOOT_SINGLE_CPU		:= 1
PROGRAMMABLE_RESET_ADDRESS	:= 1

RESET_TO_BL31		:= 0

MULTI_CONSOLE_API	:= 1

TRNG_SUPPORT := 1

ENABLE_FEAT_RNG := 1

ENABLE_FEAT_HCX := 1
ENABLE_FEAT_AMU := 1
ENABLE_FEAT_FGT := 1
ENABLE_FEAT_ECV := 1
ENABLE_TRF_FOR_NS := 0

# Overriding default value, so that EL1 context registers are initialized
SPMD_SPM_AT_SEL2 := 0
ENABLE_FEAT_SEL2 := 0

QTI_HWTRACE_SUPPORT := 0

QTI_SDI_BUILD := 1

QTI_UART_CONSOLE := 1

CTX_INCLUDE_AARCH32_REGS := 0

WORKAROUND_CVE_2017_5715 := 0
WORKAROUND_CVE_2018_3639 := 1
DYNAMIC_WORKAROUND_CVE_2018_3639 := 1
WORKAROUND_CVE_2022_23960 := 0
WORKAROUND_CVE_2024_5660 := 0

CROS_WIDEVINE_SMC := 0

ENABLE_STACK_PROTECTOR := strong

ENABLE_ASSERTIONS := 1

ENABLE_BACKTRACE := 1

QTI_EXTERNAL_INCLUDES :=					\
	-I${QTI_PLAT_PATH}/${CHIPSET}/inc			\
	-I${QTI_PLAT_PATH}/common/inc				\
	-I${QTI_PLAT_PATH}/common/inc/$(ARCH)			\
	-I${QTI_PLAT_PATH}/bl31qtilib/inc			\
	-I${QTI_COMMON_PATH}/inc				\
	-I${QTI_COMMON_PATH}/inc/$(ARCH)

QTI_BL31_SOURCES :=						\
	$(QTI_PLAT_PATH)/common/src/$(ARCH)/wildcat_helpers.S	\
	$(QTI_COMMON_PATH)/src/$(ARCH)/qti_uart_console.S	\
	$(QTI_COMMON_PATH)/src/$(ARCH)/qti_ringbuf_console.S \
	$(QTI_COMMON_PATH)/src/qti_ringbuf_console_helper.c \
	$(QTI_PLAT_PATH)/common/src/$(ARCH)/wildcat_pauth.c	\
	$(QTI_PLAT_PATH)/common/src/$(ARCH)/wildcat_unhandled_exception_asm.S \
	$(QTI_COMMON_PATH)/src/pm_ps_hold.c			\
	$(QTI_COMMON_PATH)/src/qti_stack_protector.c		\
	$(QTI_COMMON_PATH)/src/qti_common.c			\
	$(QTI_PLAT_PATH)/common/src/wildcat_bl31qtiblob_setup.c	\
	$(QTI_COMMON_PATH)/src/qti_gic_v3.c			\
	$(QTI_COMMON_PATH)/src/qti_interrupt_svc.c		\
	$(QTI_COMMON_PATH)/src/qti_interrupt_svc_el3.c		\
	$(QTI_COMMON_PATH)/src/qti_syscall.c			\
	$(QTI_PLAT_PATH)/common/src/wildcat_sip_bl31qtilib.c	\
	$(QTI_PLAT_PATH)/common/src/wildcat_topology.c		\
	$(QTI_PLAT_PATH)/common/src/wildcat_bl31qtilib_pm.c	\
	$(QTI_PLAT_PATH)/common/src/wildcat_rng.c		\
	drivers/qti/crypto/rng.c				\
	$(QTI_COMMON_PATH)/src/spmi_arb.c			\
	$(QTI_PLAT_PATH)/bl31qtilib/src/bl31qtilib_cb_interface.c \
	$(QTI_PLAT_PATH)/common/src/wildcat_plat_helpers.c

ifeq ($(CROS_WIDEVINE_SMC),0)
QTI_BL31_SOURCES += $(QTI_PLAT_PATH)/common/src/wildcat_oem_svc.c
endif

PLAT_INCLUDES := -Iinclude/plat/common/

PLAT_INCLUDES += ${QTI_EXTERNAL_INCLUDES}

include lib/xlat_tables_v2/xlat_tables.mk
include drivers/qti/smem/smem.mk
include drivers/qti/chipinfo/chipinfo.mk
PLAT_BL_COMMON_SOURCES +=					\
	${XLAT_TABLES_LIB_SRCS}					\
	plat/common/aarch64/crash_console_helpers.S		\
	common/desc_image_load.c				\
	lib/bl_aux_params/bl_aux_params.c

include lib/coreboot/coreboot.mk

GIC_ENABLE_V4_EXTN := 1
GIC_EXT_INTID := 1
GICV3_SUPPORT_GIC600 := 1
include drivers/arm/gic/v3/gicv3.mk

GIC_SOURCES :=					\
	plat/common/plat_gicv3.c		\
	${GICV3_SOURCES}

TIMER_SOURCES :=					\
	drivers/delay_timer/generic_delay_timer.c	\
	drivers/delay_timer/delay_timer.c

QTI_NCC_CPU := 1

$(eval $(call add_define, QTI_NO_SMCC_ARCH_SOC_ID))

CPU_SOURCES := $(QTI_PLAT_PATH)/common/src/aarch64/qcom_oryon_v1.S

BL31_SOURCES +=			\
	${QTI_BL31_SOURCES}	\
	${GIC_SOURCES}		\
	${TIMER_SOURCES}	\
	${CPU_SOURCES}

LIB_QTI_PATH := ${QTI_PLAT_PATH}/bl31qtilib/lib/${CHIPSET}

# Override this on the command line to point to the bl31qtilib library
BL31QTILIB_PATH ?=
ifeq ($(BL31QTILIB_PATH),)
# if No lib then use stub implementation for bl31qtilib interface
$(warning BL31QTILIB_PATH is not provided while building, using stub \
	implementation. THIS FIRMWARE WILL NOT BOOT!)
BL31_SOURCES +=							\
	${QTI_PLAT_PATH}/bl31qtilib/src/bl31qtilib_interface_stub.c \
	${QTI_PLAT_PATH}/bl31qtilib/src/bl31qtilib_version_strings_stub.c
else
# use library provided by BL31QTILIB_PATH
LDFLAGS += -L $(dir $(BL31QTILIB_PATH))
LDLIBS += -l$(patsubst lib%.a,%,$(notdir $(BL31QTILIB_PATH)))

endif

include $(QTI_PLAT_PATH)/common/wildcat_common.mk
