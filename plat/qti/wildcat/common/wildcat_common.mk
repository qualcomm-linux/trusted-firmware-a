# Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
PLAT_EXTRA_LD_SCRIPT := 1
$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

# Keep -g flag for all builds (Debug and Release)
CFLAGS += -g

ASFLAGS += -D__ASSEMBLY__

$(eval $(call assert_boolean,QTI_HWTRACE_SUPPORT))
$(eval $(call add_define,QTI_HWTRACE_SUPPORT))

$(eval $(call assert_boolean,QTI_SDI_BUILD))
$(eval $(call add_define,QTI_SDI_BUILD))

ifeq (${QTI_UART_CONSOLE},1)
$(eval $(call add_define,QTI_UART_CONSOLE))
endif

ifeq (${QTI_NCC_CPU},1)
$(eval $(call add_define, QTI_NCC_CPU))
endif

ifeq (${QTEE_BL31_SHARED_DATA},1)
$(eval $(call add_define, QTEE_BL31_SHARED_DATA))
endif

# Print all compiler -D defines (used by tfa_qti CMake build to sync defines)
.PHONY: print-defines
print-defines:
	@echo $(DEFINES)
