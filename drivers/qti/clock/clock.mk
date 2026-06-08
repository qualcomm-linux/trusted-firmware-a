#
# Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Clock driver
#

$(eval $(call add_define,QTI_CLOCK_ENABLED))

CLOCK_DRV_PATH := drivers/qti/clock

PLAT_INCLUDES += \
	-Iinclude/drivers/qti/clock

BL31_SOURCES += \
	$(CLOCK_DRV_PATH)/clock_driver.c			\
	$(CLOCK_DRV_PATH)/clock_hal.c				\
	$(CLOCK_DRV_PATH)/clock_atf.c				\
	$(CLOCK_DRV_PATH)/clock_bsp.c				\
	$(CLOCK_DRV_PATH)/$(CHIPSET)/clock_chipinfo.c		\
	$(CLOCK_DRV_PATH)/$(CHIPSET)/clock_icbuarb.c
