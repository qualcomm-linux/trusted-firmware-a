#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

$(eval $(call add_define,QTI_SMMU_ENABLED))

PLAT_DRIVERS_PATH :=	drivers/qti
BL31_SOURCES	+=	$(PLAT_DRIVERS_PATH)/smmu/$(CHIPSET)/smmu_cfg.c	\
			$(PLAT_DRIVERS_PATH)/smmu/smmu.c
