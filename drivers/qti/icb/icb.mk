#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# QTI ICB (Interconnect Bus) NOC error logger driver
#

$(eval $(call add_define,QTI_ICB_ENABLED))

ICB_BASE        :=      drivers/qti/icb

PLAT_INCLUDES   +=      -I$(ICB_BASE)
PLAT_INCLUDES   +=      -I$(ICB_BASE)/include

BL31_SOURCES    +=      $(ICB_BASE)/icb.c               \
                        $(ICB_BASE)/noc_error.c         \
                        $(ICB_BASE)/icbuarb.c           \
                        $(ICB_BASE)/icbcfg.c            \
                        $(ICB_BASE)/icbcfg_query.c
#
# Platform back-end. Each chipset directory under drivers/qti/icb/<CHIPSET>/
# must provide:
#   - noc_error_data.c      (defines nocerr_config_info)
#   - noc_error_oem_data.c  (defines nocerr_config_info_oem)
#   - noc_error_target.c    (defines qti_noc_error_init_target,
#                            qti_noc_error_handle_target,
#                            qti_noc_error_ffi_mem_map_wrap and
#                            qti_noc_error_is_part_disabled)
#   - icbuarb_target.c      (defines icbuarb_target_get_info() and
#                            icbuarb_target_init())
#
# When CHIPSET is unset a target stub with default (no-op) behaviour is
# used so the common driver can still be built and linked.
#
ifneq ($(CHIPSET),)
PLAT_INCLUDES   +=      -I$(ICB_BASE)/$(CHIPSET)
BL31_SOURCES    +=      $(ICB_BASE)/$(CHIPSET)/noc_error_data.c         \
                        $(ICB_BASE)/$(CHIPSET)/noc_error_oem_data.c     \
                        $(ICB_BASE)/$(CHIPSET)/noc_error_target.c       \
                        $(ICB_BASE)/$(CHIPSET)/icbuarb_target.c         \
                        $(ICB_BASE)/$(CHIPSET)/u_data.c                 \
                        $(ICB_BASE)/$(CHIPSET)/icbcfg_query_data.c
else
BL31_SOURCES    +=      $(ICB_BASE)/noc_error_target_stub.c
endif

#
# Optional: NoC bus-rail BCM voting dependency (ICB micro-arbiter).
#
# Some targets must vote the NoC bus rails (BCM resources) ON before the
# NOC error registers can be programmed. Setting ICB_NOC_BCM_VOTE := 1
# pulls in the ICB micro-arbiter (which transitively requires the RPMh
# command service and cmd_db drivers); the platform's noc_error_target.c
# is then expected to call qti_icbuarb_init() / icbuarb_create_client() /
# icbuarb_issue_request() from qti_noc_error_init_target().
#
ifeq ($(ICB_NOC_BCM_VOTE),1)
$(eval $(call add_define,ICB_NOC_BCM_VOTE))
endif
