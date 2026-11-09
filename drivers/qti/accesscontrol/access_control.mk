#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_DRIVERS_PATH		:=	drivers/qti
PLAT_DRIVERS_INCLUDE_PATH	:=	include/drivers/qti

ACCESSCONTROL_PATH		:=	$(PLAT_DRIVERS_PATH)/accesscontrol

##
## XPU_VERSION selects the XPU hardware generation. Exactly one generation is
## linked into a build.
##
##   3 - Configuration comes from compile-time tables in cfg/${CHIPSET}.
##   4 - Configuration is fetched at runtime from the access-control config
##       image, whose base address the platform supplies. XPU4 targets have no
##       cfg/${CHIPSET} directory at all, so nothing under cfg/ may be
##       referenced from the XPU4 block below.
##
XPU_VERSION ?= 3

ifeq ($(filter ${XPU_VERSION},3 4),)
    $(error XPU_VERSION must be 3 or 4, got '${XPU_VERSION}')
endif

PLAT_INCLUDES +=	-I$(ACCESSCONTROL_PATH) \
			-I$(PLAT_DRIVERS_INCLUDE_PATH)/accesscontrol \
			-I$(ACCESSCONTROL_PATH)/src

## Access control. Revision-agnostic: talks to the selected XPU driver through
## the interface in src/xpu_common.h.
BL31_SOURCES +=		$(ACCESSCONTROL_PATH)/src/access_control.c

## VMIDMT generic sources. Where its configuration comes from is revision
## specific, so the provider that implements src/vmidmt/vmidmt_cfg.h is added
## by the XPU_VERSION block below.
PLAT_INCLUDES +=	-I$(ACCESSCONTROL_PATH)/src/vmidmt

BL31_SOURCES +=		$(ACCESSCONTROL_PATH)/src/vmidmt/vmidmt.c \
			$(ACCESSCONTROL_PATH)/src/vmidmt/vmidmt_hal.c

ifeq (${XPU_VERSION},4)

## XPU4: every table is parsed out of the access-control config image at
## runtime, so no cfg/${CHIPSET} include path or source is used here. The
## VMIDMT configuration comes from the same image.
PLAT_INCLUDES +=	-I$(ACCESSCONTROL_PATH)/src/xpu4

BL31_SOURCES +=		$(ACCESSCONTROL_PATH)/src/ac_cfg.c \
			$(ACCESSCONTROL_PATH)/src/vmidmt/vmidmt_cfg_ac.c \
			$(ACCESSCONTROL_PATH)/src/xpu4/xpu4.c \
			$(ACCESSCONTROL_PATH)/src/xpu4/xpu4_hal.c \
			$(ACCESSCONTROL_PATH)/src/xpu4/xpu4_isr.c

else

## XPU3: compile-time configuration tables under cfg/${CHIPSET}. That directory
## also provides the VMIDMT configuration and its target register addresses.
ifeq (${CHIPSET},)
    $(error CHIPSET must be set when XPU_VERSION is 3)
endif

PLAT_INCLUDES +=	-I$(ACCESSCONTROL_PATH)/src/xpu3 \
			-I$(ACCESSCONTROL_PATH)/cfg/${CHIPSET}

BL31_SOURCES +=		$(ACCESSCONTROL_PATH)/src/xpu3/xpu3.c \
			$(ACCESSCONTROL_PATH)/src/xpu3/xpu3_hal.c \
			$(ACCESSCONTROL_PATH)/src/xpu3/xpu3_isr.c \
			$(ACCESSCONTROL_PATH)/cfg/${CHIPSET}/vmidmt_static_config.c \
			$(ACCESSCONTROL_PATH)/cfg/${CHIPSET}/xpu_static_config.c \
			$(ACCESSCONTROL_PATH)/cfg/${CHIPSET}/xpu_target_info.c

endif
