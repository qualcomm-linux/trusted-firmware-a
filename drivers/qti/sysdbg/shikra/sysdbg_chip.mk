#
# Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Shikra part of the SDI driver; included from drivers/qti/sysdbg/sysdbg.mk.

SYSDBG_DCC		?=	1

# Optional build-time overrides of the boot image's DDR self-refresh entry
# points (see sysdbg_target_config.h for the defaults).
ifneq (${SDI_DDR_INIT_ADDR},)
$(eval $(call add_define,SDI_DDR_INIT_ADDR))
endif
ifneq (${SDI_DDR_EXIT_ADDR},)
$(eval $(call add_define,SDI_DDR_EXIT_ADDR))
endif

SYSDBG_CHIP_SOURCES	:=	${SYSDBG_PATH}/shikra/sysdbg_reset_check.S	\
				${SYSDBG_PATH}/shikra/sysdbg_imem_entry.S	\
				${SYSDBG_PATH}/shikra/sysdbg_ddr.c		\
				${SYSDBG_PATH}/shikra/sysdbg_ddr_entry.S	\
				${SYSDBG_PATH}/shikra/sysdbg_reset.c		\
				${SYSDBG_PATH}/shikra/sysdbg_err.c		\
				${SYSDBG_PATH}/shikra/sysdbg_target.c		\
				${SYSDBG_PATH}/shikra/sysdbg_target_data.c

ifeq (${SYSDBG_DCC},1)
SYSDBG_CHIP_SOURCES	+=	${SYSDBG_PATH}/shikra/sysdbg_dcc_cfg.c
endif
