#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# qgic -- family-agnostic GIC interrupt registration shim.
#
# Platform makefiles select the family implementation according to their
# external security-library configuration.
#

QGIC_DRV_PATH := drivers/qti/qgic

PLAT_INCLUDES += -Iinclude/drivers/qti/qgic
