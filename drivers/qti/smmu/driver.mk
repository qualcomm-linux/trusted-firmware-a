#---------------------------------------------------------------------------
# Copyright (c) 2025 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#---------------------------------------------------------------------------


$(info    Included smmu/driver.mk)

#================================================================
# Make Header.
#----------------------------------------------------------------
module = $(SUBDIR)
#================================================================

# Header paths and sources.
global_includes	:=	include/drivers/qti/smmu	\

module_includes	:=	${CHIPSET} \
			.	\

module_sources	:=	smmu.c	\


BL31_INCLUDES += -Iinclude -Iinclude/lib
BL31_SOURCES += drivers/qti/smmu/smmu.c drivers/qti/smmu/smmu_soc_cfg.c

#================================================================
# Make Footer.
#----------------------------------------------------------------
_LIB_SOURCES  += $(addprefix $(module), $(module_sources))
_LIB_INCLUDES += $(addprefix -I$(module), $(module_includes))
_LIB_INCLUDES += $(addprefix -I, $(global_includes))
#================================================================
