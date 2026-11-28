#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# CPRH (CPR hardened) closed-loop AVS controller driver
#

CPRH_DRV_PATH := drivers/qti/cprh

PLAT_INCLUDES += \
	-Iinclude/drivers/qti/cprh \
	-I$(CPRH_DRV_PATH)/$(CHIPSET)

BL31_SOURCES += \
	$(CPRH_DRV_PATH)/common/cprh_init.c \
	$(CPRH_DRV_PATH)/common/cprh_hal.c \
	$(CPRH_DRV_PATH)/$(CHIPSET)/cprh_cfg.c
