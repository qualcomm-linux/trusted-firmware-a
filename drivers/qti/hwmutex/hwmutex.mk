#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Hardware Mutex (HWMUTEX) driver for TF-A.
#
# Provides:
#   hwmutex.c   - low-level HW mutex acquire/release (mmio-based)
#   hwm_lock.c  - ticket-lock and LPM-sync lock built on top of hwmutex
#
# Consumers must also set:
#   PLAT_INCLUDES += -Iinclude/drivers/qti/cpucp/<chipset>
# so that <cpucp_hwio.h> (which defines APSS_CPUCP_S_MUTEX_BASE and
# CPU_LPM_SYNC_DATA_BASE) can be found.  This is typically done in the
# chipset-specific platform.mk before including this file.
#

HWMUTEX_SOURCES	:=	drivers/qti/hwmutex/hwmutex.c		\
			drivers/qti/hwmutex/hwm_lock.c

# Expose the public hwmutex header to all TF-A components.
HWMUTEX_INCLUDES :=	-Iinclude/drivers/qti/hwmutex		\
			-Idrivers/qti/hwmutex

BL31_SOURCES	+=	${HWMUTEX_SOURCES}
PLAT_INCLUDES	+=	${HWMUTEX_INCLUDES}
