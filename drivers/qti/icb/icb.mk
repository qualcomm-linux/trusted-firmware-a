#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# QTI ICB (Interconnect Bus) drivers.
#
# This covers three independent sub-drivers, each gated by its own flag so
# a platform can enable any subset of them:
#
#   QTI_ICB_NOCERR_ENABLED  - NoC error logger (noc_error.c)
#   QTI_ICB_CONFIG_ENABLED  - ICB one-time register configuration (icbcfg.c)
#   QTI_ICB_ENABLED         - ICB micro-arbiter / NoC bus-rail BCM voting
#                             (icbuarb.c)
#
# Set the relevant flag(s) to 1 in platform.mk before including this file,
# e.g.:
#   QTI_ICB_NOCERR_ENABLED := 1
#   QTI_ICB_CONFIG_ENABLED := 1
#   include drivers/qti/icb/icb.mk
#

ICB_BASE        :=      drivers/qti/icb

PLAT_INCLUDES   +=      -I$(ICB_BASE)
PLAT_INCLUDES   +=      -I$(ICB_BASE)/include
PLAT_INCLUDES   +=      -I$(ICB_BASE)/$(CHIPSET)

#
# NoC error logger. Each chipset directory under drivers/qti/icb/<CHIPSET>/
# must provide:
#   - noc_error_data.c      (defines nocerr_config_info)
#   - noc_error_oem_data.c  (defines nocerr_config_info_oem)
#   - noc_error_target.c    (defines qti_noc_error_init_target,
#                            qti_noc_error_handle_target,
#                            qti_noc_error_ffi_mem_map_wrap and
#                            qti_noc_error_is_part_disabled)
#
ifeq ($(QTI_ICB_NOCERR_ENABLED),1)
$(eval $(call add_define,QTI_ICB_NOCERR_ENABLED))

BL31_SOURCES    +=      $(ICB_BASE)/noc_error.c                         \
                        $(ICB_BASE)/$(CHIPSET)/noc_error_data.c         \
                        $(ICB_BASE)/$(CHIPSET)/noc_error_oem_data.c     \
                        $(ICB_BASE)/$(CHIPSET)/noc_error_target.c
endif

#
# ICB one-time register configuration. Each chipset directory under
# drivers/qti/icb/<CHIPSET>/ must provide:
#   - icbcfg_query_data.c   (defines icbcfg_info)
#
ifeq ($(QTI_ICB_CONFIG_ENABLED),1)
$(eval $(call add_define,QTI_ICB_CONFIG_ENABLED))

BL31_SOURCES    +=      $(ICB_BASE)/icbcfg.c                            \
                        $(ICB_BASE)/$(CHIPSET)/icbcfg_query_data.c
endif

#
# ICB micro-arbiter (NoC bus-rail BCM voting). Transitively requires the
# RPMh command service and cmd_db drivers. Some targets must vote the NoC
# bus rails ON before the NOC error registers can be programmed; the
# platform's noc_error_target.c is then expected to call
# qti_icbuarb_init() / icbuarb_create_client() / icbuarb_issue_request()
# from qti_noc_error_init_target(). Each chipset directory under
# drivers/qti/icb/<CHIPSET>/ must provide:
#   - icbuarb_target.c      (defines icbuarb_target_get_info() and
#                            icbuarb_target_init())
#   - u_data.c              (defines the ICB topology: masters, slaves,
#                            routes and BCM hw nodes)
#
ifeq ($(QTI_ICB_ENABLED),1)
$(eval $(call add_define,QTI_ICB_ENABLED))

BL31_SOURCES    +=      $(ICB_BASE)/icbuarb.c                           \
                        $(ICB_BASE)/$(CHIPSET)/icbuarb_target.c         \
                        $(ICB_BASE)/$(CHIPSET)/u_data.c

ifeq ($(ICB_NOC_BCM_VOTE),1)
$(eval $(call add_define,ICB_NOC_BCM_VOTE))
endif
endif
