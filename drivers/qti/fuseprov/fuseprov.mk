#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
#

FUSEPROV_SOURCES	:= drivers/qti/fuseprov/src/fuseprov_core.c \
			   drivers/qti/fuseprov/src/fuseprov_sec_elf_v3.c \
			   drivers/qti/fuseprov/port/stub/fuseprov_port_stub.c \
			   drivers/qti/fuseprov/port/tme/fuseprov_port_tme.c

BL31_SOURCES	+=	${FUSEPROV_SOURCES}
