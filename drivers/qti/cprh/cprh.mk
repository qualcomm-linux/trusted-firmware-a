#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES	+=	-Iinclude/drivers/qti/cprh	\
			-Iinclude/drivers/qti/vpp

$(eval $(call add_define,QTI_CPRH_ENABLED))

BL31_SOURCES	+=	drivers/qti/cprh/cprh.c		\
			drivers/qti/cprh/$(CHIPSET)/cprh_cfg.c
