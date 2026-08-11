#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# QTI Coreinit 
#

$(eval $(call add_define,QTI_COREINIT_ENABLED))

COREINIT_BASE	:=	drivers/qti/coreinit

PLAT_INCLUDES	+=	-I$(COREINIT_BASE)
PLAT_INCLUDES   +=      -I$(COREINIT_BASE)/include

BL31_SOURCES	+=	$(COREINIT_BASE)/coreinit.c

ifneq ($(CHIPSET),)
PLAT_INCLUDES	+=	-I$(COREINIT_BASE)/$(CHIPSET)
BL31_SOURCES	+=	$(COREINIT_BASE)/$(CHIPSET)/coreinit_data.c	
else
BL31_SOURCES	+=	$(COREINIT_BASE)/coreinit_stub.c
endif
