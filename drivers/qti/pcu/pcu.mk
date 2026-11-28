#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# PCU/RIMPS hardware sequencer driver
#

PCU_DRV_PATH := drivers/qti/pcu

PLAT_INCLUDES += \
	-Iinclude/drivers/qti/pcu

BL31_SOURCES += \
	$(PCU_DRV_PATH)/$(CHIPSET)/pcu_init.c
