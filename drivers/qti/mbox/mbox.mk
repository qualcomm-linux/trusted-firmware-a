#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Qualcomm mailbox framework
#
# Usage from platform makefile:
#
# QCOM_MBOX	:=	1   # enable common core (required)
# QCOM_MBOX_QMP	:=	1   # enable QMP transport
#

ifeq (${QCOM_MBOX},1)

$(eval $(call add_define,QCOM_MBOX))

MBOX_DRV_PATH	:= drivers/qti/mbox

PLAT_INCLUDES	+= -I${MBOX_DRV_PATH}
PLAT_INCLUDES	+= -Iinclude/drivers/qti/mbox

BL31_SOURCES	+= ${MBOX_DRV_PATH}/qcom_mbox.c

ifeq (${QCOM_MBOX_QMP},1)
$(eval $(call add_define,QCOM_MBOX_QMP))
BL31_SOURCES	+= ${MBOX_DRV_PATH}/qcom_mbox_qmp.c
endif

endif # QCOM_MBOX