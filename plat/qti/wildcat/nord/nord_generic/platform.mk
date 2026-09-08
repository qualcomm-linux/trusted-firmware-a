#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Makefile for Nord based QCS/EVK QTI platform (Wildcat architecture).

PLAT_PATH				:=	plat/qti
CHIPSET					:=	nord

RESET_TO_BL2				:=	1

# Turn On Separate code & data.
SEPARATE_CODE_AND_RODATA		:=	1
USE_COHERENT_MEM			:=	0
WARMBOOT_ENABLE_DCACHE_EARLY		:=	1
HW_ASSISTED_COHERENCY			:=	1

# Enable PSCI v1.0 extended state ID format
PSCI_EXTENDED_STATE_ID			:=	1
ARM_RECOM_STATE_ID_ENC			:=	1
PSCI_OS_INIT_MODE			:=	1

# GIC-700 configuration
GIC_ENABLE_V4_EXTN			:= 1
GIC_EXT_INTID				:= 1

GICV3_SUPPORT_GIC600			:= 1

# Route the BL31 console to the physical GENI UART (UART_BASE_ADDR=0x884000)
# so EL3 NOTICE/INFO logs appear on the serial console.
QTI_UART_CONSOLE			:=	1
$(eval $(call add_define,QTI_UART_CONSOLE))

COLD_BOOT_SINGLE_CPU			:=	1
PROGRAMMABLE_RESET_ADDRESS		:=	1

# Enable the dynamic translation tables library
PLAT_XLAT_TABLES_DYNAMIC		:=	1
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))

# disable CTX_INCLUDE_AARCH32_REGS to support nord cores
override CTX_INCLUDE_AARCH32_REGS	:=	0
# Enable stack protector.
ENABLE_STACK_PROTECTOR := strong

# Oryon implements FEAT_HCX (Armv8.7). The normal-world EL2 payload (Linux
# init_el2) reads ID_AA64MMFR1_EL1.HCX, sees it implemented, and writes
# HCRX_EL2. That access is UNDEFINED at EL2 unless EL3 sets SCR_EL3.HXEn, so
# BL31 must enable FEAT_HCX (which sets HXEn and context-saves HCRX_EL2).
# Without this the OS takes an undefined-instruction abort at init_el2+0xe4
# (msr hcrx_el2) during early boot. 2 = detect at runtime via ID registers.
ENABLE_FEAT_HCX				:=	2

# Oryon implements the Armv8.6+ trap/control feature set used by the EL2 OS
# (Linux init_el2 / finalise_el2). Each is gated by an ID-register check in the
# kernel, then accesses the corresponding EL2 register, which is UNDEFINED at
# EL2 unless EL3 enables it (SCR_EL3 / fine-grained-trap enables). Enable them
# runtime-checked (2) so BL31 grants EL2 access and context-saves the regs.
ENABLE_FEAT_FGT				:=	2
ENABLE_FEAT_FGT2			:=	2
ENABLE_FEAT_ECV				:=	2
ENABLE_FEAT_TCR2			:=	2
ENABLE_FEAT_SCTLR2			:=	2
ENABLE_FEAT_S1PIE			:=	2
ENABLE_FEAT_S2PIE			:=	2
ENABLE_FEAT_S1POE			:=	2
ENABLE_FEAT_S2POE			:=	2
ENABLE_FEAT_MOPS			:=	2
ENABLE_FEAT_AMU				:=	2

PLAT_INCLUDES		:=	-Iinclude/plat/common/					\
				-I${PLAT_PATH}/wildcat/${CHIPSET}/inc			\
				-I${PLAT_PATH}/wildcat/${CHIPSET}/${PLAT}/inc		\
				-I${PLAT_PATH}/common/inc				\
				-I${PLAT_PATH}/common/inc/$(ARCH)			\
				-I${PLAT_PATH}/wildcat/${CHIPSET}/inc

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	common/desc_image_load.c				\
				drivers/qti/crypto/rng.c				\
				lib/cpus/aarch64/oryon.S				\
				lib/bl_aux_params/bl_aux_params.c			\
				plat/common/aarch64/crash_console_helpers.S		\
				$(PLAT_PATH)/common/src/$(ARCH)/qti_uart_console.S	\
				$(PLAT_PATH)/common/src/qti_stack_protector.c		\
				$(PLAT_PATH)/common/src/qti_common.c			\
				${XLAT_TABLES_LIB_SRCS}

BL2_SOURCES		+=	drivers/io/io_fip.c					\
				drivers/io/io_memmap.c					\
				drivers/io/io_storage.c					\
				$(PLAT_PATH)/common/src/$(ARCH)/qti_bl2_helpers.S	\
				$(PLAT_PATH)/common/src/qti_bl2_setup.c			\
				$(PLAT_PATH)/common/src/qti_image_desc.c		\
				$(PLAT_PATH)/common/src/qti_io_storage.c

include drivers/arm/gic/v3/gicv3.mk
include drivers/qti/chipinfo/chipinfo.mk
include drivers/qti/smem/smem.mk
BL31_SOURCES		+=	drivers/delay_timer/generic_delay_timer.c		\
				drivers/delay_timer/delay_timer.c			\
				plat/common/plat_gicv3.c				\
				${GICV3_SOURCES}					\
				plat/common/plat_psci_common.c				\
				$(PLAT_PATH)/wildcat/common/$(ARCH)/wildcat_helpers.S	\
				$(PLAT_PATH)/wildcat/common/wildcat_bl31_setup.c	\
				$(PLAT_PATH)/common/src/qti_bl31_setup.c		\
				$(PLAT_PATH)/common/src/pm_ps_hold.c			\
				$(PLAT_PATH)/common/src/qti_gic_v3.c			\
				$(PLAT_PATH)/wildcat/${CHIPSET}/src/nord_gicv3.c	\
				$(PLAT_PATH)/common/src/qti_interrupt_svc.c		\
				$(PLAT_PATH)/common/src/qti_topology.c			\
				$(PLAT_PATH)/common/src/qti_pm.c			\
				$(PLAT_PATH)/wildcat/common/wildcat_pm.c		\
				$(PLAT_PATH)/common/src/qti_syscall.c			\
				$(PLAT_PATH)/common/src/spmi_arb.c			\
				drivers/qti/accesscontrol/access_control_stub.c

# Secondary-core remap for the Oryon cores.
BL31_SOURCES		+=	drivers/qti/sec_core/sec_core_nord.c

# APSS Interrupt Unit (INTU) per-SPI type configuration.
BL31_SOURCES		+=	$(PLAT_PATH)/wildcat/${CHIPSET}/src/nord_intu.c

# Architected timer (qtimer).
PLAT_INCLUDES		+=	-Iinclude/drivers/qti/qtimer/${CHIPSET}
BL31_SOURCES		+=	drivers/qti/qtimer/qtimer.c

# Secure watchdog.
PLAT_INCLUDES		+=	-Iinclude/drivers/qti/watchdog/${CHIPSET}
BL31_SOURCES		+=	drivers/qti/watchdog/watchdog.c

# CPUCP host driver: notify CPUCP of per-core power-state changes during
# PSCI CPU_ON (see plat_qti_pwr_domain_on() in wildcat_pm.c).
include drivers/qti/cpucp/cpucp.mk

# SMMU configuration.
include drivers/qti/smmu/smmu.mk

# PDC (Power Domain Controller) configuration. Nord has no TCS/wake-resource
# path (PDC_ENABLE_TCS=0 in nord/pdc_config.mk); low-power-mode entry is driven
# by a branch mask instead, so cmd_db is not needed here.
include drivers/qti/pdc/pdc.mk

