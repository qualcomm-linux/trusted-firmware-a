#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# PMIC / SPMI ARB driver
#

# PMIC_ARB_VERSION selects which drivers/qti/pmic/<version>/ backend this
# platform builds against. Set from the platform's own platform.mk.
PMIC_ARB_VERSION	?=	pmicarb7

QTI_PMIC_SOURCES	:=	drivers/qti/pmic/${PMIC_ARB_VERSION}/spmi_arb_hal.c	\
				drivers/qti/pmic/pm_pon.c

PLAT_INCLUDES		+=	-Iinclude/drivers/qti/pmic			\
				-Iinclude/drivers/qti/pmic/${PMIC_ARB_VERSION}	\
				-Iinclude/drivers/qti/pmic/${CHIPSET}

BL31_SOURCES		+=	${QTI_PMIC_SOURCES}
