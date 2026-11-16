#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_DRIVERS_PATH		:=	drivers/qti
PLAT_DRIVERS_INCLUDE_PATH	:=	include/drivers/qti

ACCESSCONTROL_PATH		:=	$(PLAT_DRIVERS_PATH)/accesscontrol


PLAT_INCLUDES +=	-I$(ACCESSCONTROL_PATH) \
			-I$(PLAT_DRIVERS_INCLUDE_PATH)/accesscontrol

## Access control. Revision-agnostic: talks to the selected XPU driver through
## the interface in xpu_common.h.
BL31_SOURCES +=		$(ACCESSCONTROL_PATH)/access_control.c

## VMIDMT generic sources. Configuration provider implementing vmidmt_cfg.h is
## supplied by the cfg/${CHIPSET} directory.
PLAT_INCLUDES +=	-I$(ACCESSCONTROL_PATH)/vmidmt

BL31_SOURCES +=		$(ACCESSCONTROL_PATH)/vmidmt/vmidmt.c \
			$(ACCESSCONTROL_PATH)/vmidmt/vmidmt_hal.c

## XPU3: compile-time configuration tables under cfg/${CHIPSET}. That directory
## also provides the VMIDMT configuration and its target register addresses.
ifeq (${CHIPSET},)
    $(error CHIPSET must be set for accesscontrol)
endif

PLAT_INCLUDES +=	-I$(ACCESSCONTROL_PATH)/xpu3 \
			-I$(ACCESSCONTROL_PATH)/cfg/${CHIPSET}

BL31_SOURCES +=		$(ACCESSCONTROL_PATH)/xpu3/xpu3.c \
			$(ACCESSCONTROL_PATH)/xpu3/xpu3_hal.c \
			$(ACCESSCONTROL_PATH)/xpu3/xpu3_isr.c \
			$(ACCESSCONTROL_PATH)/cfg/${CHIPSET}/vmidmt_static_config.c \
			$(ACCESSCONTROL_PATH)/cfg/${CHIPSET}/xpu_static_config.c \
			$(ACCESSCONTROL_PATH)/cfg/${CHIPSET}/xpu_target_info.c
