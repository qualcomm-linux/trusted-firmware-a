#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Power utils (vlvl <-> hlvl) driver
#

$(eval $(call add_define,QCOM_PWR_UTILS_ENABLED))

PWR_UTILS_DRV_PATH := drivers/qcom/pwr_utils
CMD_DB_DRV_PATH := drivers/qcom/cmd_db

BL31_SOURCES += \
	$(PWR_UTILS_DRV_PATH)/pwr_utils.c			\
	$(CMD_DB_DRV_PATH)/cmd_db.c
