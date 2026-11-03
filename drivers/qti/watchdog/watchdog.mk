#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

QTI_WATCHDOG_ENABLE ?= 0

$(eval $(call add_define,QTI_WATCHDOG_ENABLE))

ifeq ($(QTI_WATCHDOG_ENABLE),1)
ifneq ($(strip $(QTI_WDOG_VARIANT)),)
ifeq ($(strip $(QTI_WDOG_DEFS)),)
$(error QTI_WDOG_DEFS must be set when QTI_WDOG_VARIANT is selected)
endif
ifeq ($(strip $(QTI_WDOG_PLATFORM_SRC)),)
$(error QTI_WDOG_PLATFORM_SRC must be set when QTI_WDOG_VARIANT is selected)
endif

PLAT_INCLUDES += -Iinclude/drivers/qti/watchdog/$(QTI_WDOG_DEFS)

ifeq ($(QTI_WDOG_VARIANT),apsslegacywdog)
QTI_WDOG_VARIANT_SRC := drivers/qti/watchdog/apsslegacywdog/watchdog_ver.c
else ifeq ($(QTI_WDOG_VARIANT),apsec)
$(eval $(call add_define,WDOG_USE_APSEC))
QTI_WDOG_VARIANT_SRC := drivers/qti/watchdog/apsswdog/watchdog_ver.c
else ifeq ($(QTI_WDOG_VARIANT),windowed)
QTI_WDOG_VARIANT_SRC := drivers/qti/watchdog/apsswdog/watchdog_ver_windowed.c
else
$(error Unsupported QTI_WDOG_VARIANT: $(QTI_WDOG_VARIANT))
endif

BL31_SOURCES += drivers/qti/watchdog/watchdog.c \
			$(QTI_WDOG_VARIANT_SRC) \
			plat/qti/common/src/qti_watchdog_common.c \
			$(QTI_WDOG_PLATFORM_SRC)
endif
endif
