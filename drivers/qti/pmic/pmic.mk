#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# PMIC / SPMI ARB driver
#

# QTI_PMIC_ARB_VERSION selects which drivers/qti/pmic/<version>/ backend this
# platform builds against. Set from the platform's own platform.mk.
QTI_PMIC_ARB_VERSION	?=	pmicarb7

# QTI_PON_PBS_VERSION selects the PON-peripheral register layout (pm_pon.h vs.
# pm_pon5.h) this platform builds against, since PON_PS_HOLD_RESET_CTL/CTL2
# differ across PMIC generations. Set from the platform's own platform.mk;
# defaults to the PM8998-compatible layout (pm_pon.h).
QTI_PON_PBS_VERSION	?=	pon

ifeq (${QTI_PON_PBS_VERSION},pon5)
$(eval $(call add_define,QTI_PON5))
endif

QTI_PMIC_SOURCES	:=	drivers/qti/pmic/spmi_arb_hal.c				\
				drivers/qti/pmic/pm_pon.c

PLAT_INCLUDES		+=	-Iinclude/drivers/qti/pmic				\
				-Iinclude/drivers/qti/pmic/${QTI_PMIC_ARB_VERSION}

BL31_SOURCES		+=	${QTI_PMIC_SOURCES}
