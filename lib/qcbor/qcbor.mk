#
# Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Vendored copy of QCBOR v1.2 (github.com/laurencelundblade/QCBOR).
# Provides QCBOR_SOURCES and QCBOR_INCLUDES for platforms that need CBOR
# encode/decode (e.g. the QTI TME driver). Include this file once from a
# platform.mk, then fold ${QCBOR_SOURCES} into BL31_SOURCES and
# ${QCBOR_INCLUDES} into PLAT_INCLUDES.

ifndef qcbor-mk
        qcbor-mk := $(lastword $(MAKEFILE_LIST))
        qcbor-root := $(patsubst %/,%,$(dir $(qcbor-mk)))

        QCBOR_SOURCES	+=	$(qcbor-root)/src/qcbor_encode.c	\
				$(qcbor-root)/src/qcbor_decode.c	\
				$(qcbor-root)/src/UsefulBuf.c

        # v1.2 has no top-level inc/qcbor.h shim; the real headers live under
        # inc/qcbor, so both dirs are needed for #include "qcbor.h" to resolve.
        QCBOR_INCLUDES	+=	-I$(qcbor-root)/inc		\
				-I$(qcbor-root)/inc/qcbor

        # Floating point numbers are not used, so disable the support.
        # This reduces the library size as well.
        $(eval $(call add_define,QCBOR_DISABLE_FLOAT_HW_USE))
        $(eval $(call add_define,USEFULBUF_DISABLE_ALL_FLOAT))
        $(eval $(call add_define,QCBOR_DISABLE_PREFERRED_FLOAT))
endif
