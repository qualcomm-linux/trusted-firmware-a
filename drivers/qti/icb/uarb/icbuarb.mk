#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
#

# QTI ICB micro-arbiter: aggregates interconnect bandwidth requests and
# commits BCM votes via RPMh. Depends on cmd_db and rpmh (pulled in by the
# platform) and a platform-provided icbuarb_target description.

ICBUARB_BASE		:=	drivers/qti/icb/uarb

ICBUARB_SOURCES		:=	${ICBUARB_BASE}/icbuarb.c

# Platform back-end: drivers/qti/icb/<CHIPSET>/icbuarb_target.c must define
# icbuarb_target_get_info() and icbuarb_target_init().
ifneq ($(CHIPSET),)
ICBUARB_SOURCES		+=	drivers/qti/icb/$(CHIPSET)/icbuarb_target.c
endif

PLAT_INCLUDES		+=	-I${ICBUARB_BASE}

BL31_SOURCES		+=	${ICBUARB_SOURCES}
