#
# Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# System Debug Image (SDI) reset-path driver.
#
# Options:
#   QTI_USE_SYSDBG  1 (default): build the driver; 0: leave it out
#   SYSDBG_DCC      1: build the DCC support (default set per chipset)
#
# CHIPSET selects drivers/qti/sysdbg/${CHIPSET}/ and its sysdbg_chip.mk, which
# lists the chipset sources and its option defaults.

SYSDBG_PATH		:=	drivers/qti/sysdbg

PLAT_INCLUDES		+=	-Iinclude/drivers/qti/sysdbg			\
				-Iinclude/drivers/qti/sysdbg/${CHIPSET}

QTI_USE_SYSDBG		?=	1

ifeq (${QTI_USE_SYSDBG},1)
$(eval $(call add_define,QTI_USE_SYSDBG))

include ${SYSDBG_PATH}/${CHIPSET}/sysdbg_chip.mk

BL31_SOURCES		+=	${SYSDBG_PATH}/sysdbg_main.c			\
				${SYSDBG_PATH}/sysdbg_log.c			\
				${SYSDBG_PATH}/asm/sysdbg_save_ctxt_armv8.S	\
				${SYSDBG_CHIP_SOURCES}

ifeq (${SYSDBG_DCC},1)
$(eval $(call add_define,SYSDBG_DCC))
BL31_SOURCES		+=	${SYSDBG_PATH}/sysdbg_dcc.c			\
				${SYSDBG_PATH}/sysdbg_dcc_hal.c
endif
endif
