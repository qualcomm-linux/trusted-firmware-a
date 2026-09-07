#
# Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# QTI TME (Trust Management Engine) driver. Provides the tmecom transport
# layer and the tmeintf message/interface APIs. Include this from a
# platform.mk guarded by QTI_TME_DRIVER_SUPPORT.

TME_DRIVER_PATH := drivers/qti/tme

# Vendored QCBOR (CBOR encode/decode) used by tmeintf/TmeMessage.c.
include lib/qcbor/qcbor.mk

# NOTE: qcom_mbox_stub.c is deliberately NOT listed here.  drivers/qti/mbox/
# mbox.mk provides the real transport (qcom_mbox.c + qcom_mbox_qmp.c) on this
# target, so linking the stub as well would duplicate every qcom_mbox_* symbol.
#
# Do not "comment it out" with a '#' inside the list below: make treats a
# comment as running to the end of the *logical* line, so a '#' on a
# backslash-continued line silently discards every remaining entry.
BL31_SOURCES	+=	${TME_DRIVER_PATH}/tmecom_mbox.c				\
			${TME_DRIVER_PATH}/tme_tfa_glue.c			\
			${TME_DRIVER_PATH}/tmecom/src/tmecom.c			\
			${TME_DRIVER_PATH}/tmecom/src/tmecom_crc.c		\
			${TME_DRIVER_PATH}/tmecom/src/tmecom_interfaces.c	\
			${TME_DRIVER_PATH}/tmecom/src/tmecom_os_al.c		\
			${TME_DRIVER_PATH}/tmeintf/TmeForwardRequest.c		\
			${TME_DRIVER_PATH}/tmeintf/TmeMessage.c			\
			${TME_DRIVER_PATH}/tmeintf/TmeNotifyErrFatal.c		\
			${TME_DRIVER_PATH}/tmeintf/TmePassthroughAwait.c	\
			${TME_DRIVER_PATH}/tmeintf/TmePassthroughCmd.c		\
			${TME_DRIVER_PATH}/tmeintf/TmeSHADigest.c		\
			${QCBOR_SOURCES}

# TME include dirs, the vendored QCBOR headers, and the copied noship-only
# headers under inc/ (IxErrno.h, tzbsp_log.h, stringl/stringl.h, ...).
# bl31qtilib_cb_interface.h is picked up from plat/qti/v2/bl31qtilib/inc.
# include/drivers/qti provides qcom_mbox.h and the new tmecom public header.
PLAT_INCLUDES	+=	-I${TME_DRIVER_PATH}/tmecom/inc				\
			-I${TME_DRIVER_PATH}/tmeintf				\
			-I${TME_DRIVER_PATH}/inc				\
			-I${TME_DRIVER_PATH}/inc/common				\
			-I${TME_DRIVER_PATH}/inc/common/inc			\
			-I${TME_DRIVER_PATH}/inc/common/stringl			\
			-I${QTI_PLAT_PATH}/bl31qtilib/inc			\
			${QCBOR_INCLUDES}

# nord uses a 64-bit HSDMA descriptor layout in tmeintf.
ifeq (${CHIPSET},nord)
$(eval $(call add_define,FEATURE_64_BIT_HSDMA))
endif
