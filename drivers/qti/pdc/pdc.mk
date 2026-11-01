#
# Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# PDC (Power Domain Controller) driver
#

$(eval $(call add_define,QTI_PDC_ENABLED))

PDC_DRV_PATH := drivers/qti/pdc

# Lemans and Monaco share one set of apps PDC tables.
ifneq ($(filter lemans monaco,$(CHIPSET)),)
PDC_TABLES := $(PDC_DRV_PATH)/hoya
else
PDC_TABLES := $(PDC_DRV_PATH)/$(CHIPSET)
endif

-include $(PDC_TABLES)/pdc_config.mk

PDC_ENABLE_TCS ?= 1

$(eval $(call assert_boolean,PDC_ENABLE_TCS))
$(eval $(call add_define,PDC_ENABLE_TCS))

PLAT_INCLUDES += \
	-I$(PDC_TABLES)

BL31_SOURCES += \
	$(PDC_DRV_PATH)/pdc.c					\
	$(PDC_DRV_PATH)/pdc_seq.c				\
	$(PDC_TABLES)/pdc_seq_cfg.c				\
	$(PDC_TABLES)/interrupt_table.c				\
	$(PDC_TABLES)/gpio_table.c

ifeq ($(PDC_ENABLE_TCS),1)
BL31_SOURCES += \
	$(PDC_DRV_PATH)/pdc_tcs.c				\
	$(PDC_TABLES)/tcs_resource.c
endif
