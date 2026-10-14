#
# Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# QTI TME (Trust Management Engine) driver. Provides the tmecom transport
# layer and the tmeintf message/interface APIs. Include this from a
# platform.mk guarded by QTI_TME_DRIVER_SUPPORT.

TME_DRIVER_PATH := drivers/qti/tme

# Vendored QCBOR (CBOR encode/decode) used by tmeintf/tme_message.c.
include lib/qcbor/qcbor.mk

BL31_SOURCES	+=	${TME_DRIVER_PATH}/tmecom/src/tmecom.c			\
			${TME_DRIVER_PATH}/tmecom/src/tmecom_crc.c		\
			${TME_DRIVER_PATH}/tmecom/src/tmecom_interfaces.c	\
			${TME_DRIVER_PATH}/tmecom/src/tmecom_os_al.c		\
			${TME_DRIVER_PATH}/tmeintf/tme_passthrough.c		\
			${TME_DRIVER_PATH}/tmeintf/tme_fuse_read.c		\
			${TME_DRIVER_PATH}/tmeintf/tme_fuse_write_multiple.c	\
			${TME_DRIVER_PATH}/tmeintf/tme_get_pil_image_regions.c	\
			${TME_DRIVER_PATH}/tmeintf/tme_get_signed_image_ids.c	\
			${TME_DRIVER_PATH}/tmeintf/tme_invoke_ac.c		\
			${TME_DRIVER_PATH}/tmeintf/tme_message.c			\
			${TME_DRIVER_PATH}/tmeintf/tme_notify_err_fatal.c		\
			${TME_DRIVER_PATH}/tmeintf/tme_sha_digest.c		\
			${TME_DRIVER_PATH}/tmeintf/tme_set_xpu_dbgar.c		\
			${TME_DRIVER_PATH}/tmeintf/tme_update_rollback_version.c	\
			${TME_DRIVER_PATH}/tmeintf/tme_write_config_register.c	\
			${QCBOR_SOURCES}

# bl31qtilib_cb_interface.h is picked up from plat/qti/v2/bl31qtilib/inc.
# include/drivers/qti provides qti_mbox.h and the new tmecom public header.
PLAT_INCLUDES	+=	-I${TME_DRIVER_PATH}/tmecom/inc				\
			-I${TME_DRIVER_PATH}/tmeintf				\
			-I${QTI_PLAT_PATH}/bl31qtilib/inc			\
			${QCBOR_INCLUDES}

# honu uses a 64-bit HSDMA descriptor layout in tmeintf.
ifeq (${CHIPSET},honu)
$(eval $(call add_define,FEATURE_64_BIT_HSDMA))
endif
