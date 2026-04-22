#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_DRIVERS_PATH		:=	drivers/qti
PLAT_DRIVERS_INCLUDE_PATH	:= 	include/drivers/qti

## MPROC SMEM
PLAT_INCLUDES +=	-I$(PLAT_DRIVERS_INCLUDE_PATH)/smem

BL31_SOURCES +=	$(PLAT_DRIVERS_PATH)/smem/smem.c \
			$(PLAT_DRIVERS_PATH)/smem/smem_os.c \
			$(PLAT_DRIVERS_PATH)/smem/smem_partition.c \
			$(PLAT_DRIVERS_PATH)/smem/smem_target.c
