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
	$(CLOCK_DRV_PATH)/$(CHIPSET)/clock_chipinfo.c

# Link the default no-op ICB seam unless the platform builds the real ICB
# micro-arbiter, in which case the real driver supplies the same symbols.
ifneq ($(ICB_NOC_BCM_VOTE),1)
BL31_SOURCES += $(CLOCK_DRV_PATH)/stub/clock_icbuarb.c
endif
