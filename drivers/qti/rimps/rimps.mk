#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# RIMPS CPU PLL bring-up + OSM/DCVS runtime driver
#

RIMPS_DRV_PATH := drivers/qti/rimps

PLAT_INCLUDES += \
	-Iinclude/drivers/qti/rimps

BL31_SOURCES += \
	$(RIMPS_DRV_PATH)/$(CHIPSET)/rimps_init.c \
	$(RIMPS_DRV_PATH)/$(CHIPSET)/rimps_fw.c \
	$(RIMPS_DRV_PATH)/$(CHIPSET)/rimps.c
