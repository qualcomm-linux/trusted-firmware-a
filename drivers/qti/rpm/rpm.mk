#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Legacy RPM request service driver. Include this on RPM-based targets only;
# RPMh-based targets should include drivers/qti/rpmh/rpmh.mk instead.
#

$(eval $(call add_define,QTI_RPM_ENABLED))

RPM_DRV_PATH := drivers/qti/rpm

PLAT_INCLUDES += \
	-Iinclude/drivers/qti/rpm \
	-I$(RPM_DRV_PATH)

BL31_SOURCES += \
	$(RPM_DRV_PATH)/rpm_client.c \
	$(RPM_DRV_PATH)/rpm_transport_stub.c
