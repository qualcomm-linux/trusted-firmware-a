#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Inter-processor communication controller driver
#

$(eval $(call add_define,QTI_IPCC_ENABLED))

IPCC_DRV_PATH := drivers/qti/ipcc

PLAT_INCLUDES += \
	-I$(IPCC_DRV_PATH)				\
	-I$(IPCC_DRV_PATH)/$(CHIPSET)

BL31_SOURCES += \
	$(IPCC_DRV_PATH)/ipcc_core.c			\
	$(IPCC_DRV_PATH)/ipcc_backends.c		\
	$(IPCC_DRV_PATH)/$(CHIPSET)/ipcc_config.c
