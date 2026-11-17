#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES	+=	-Iinclude/drivers/qti/vpp

$(eval $(call add_define,QTI_VPP_ENABLED))

BL31_SOURCES	+=	drivers/qti/vpp/vpp.c		\
			drivers/qti/vpp/$(CHIPSET)/vpp_cfg.c
