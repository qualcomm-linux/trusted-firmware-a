#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# SAW4 AVS (Adaptive Voltage Scaling) driver
#

SAW_DRV_PATH := drivers/qti/saw

PLAT_INCLUDES += \
	-Iinclude/drivers/qti/saw \
	-I$(SAW_DRV_PATH)/$(CHIPSET)

BL31_SOURCES += \
	$(SAW_DRV_PATH)/common/saw.c
