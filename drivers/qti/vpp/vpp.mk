#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# VPP (Voltage Plan Processor) driver
#

VPP_DRV_PATH := drivers/qti/vpp

PLAT_INCLUDES += \
	-Iinclude/drivers/qti/vpp \
	-I$(VPP_DRV_PATH) \
	-I$(VPP_DRV_PATH)/common \
	-I$(VPP_DRV_PATH)/$(CHIPSET)

BL31_SOURCES += \
	$(VPP_DRV_PATH)/common/vpp.c \
	$(VPP_DRV_PATH)/common/vpp_interpolation.c \
	$(VPP_DRV_PATH)/$(CHIPSET)/vpp_voltage_plan.c \
	$(VPP_DRV_PATH)/$(CHIPSET)/vpp_voltage_tables.c
