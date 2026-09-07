/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Fallback/default version strings
 *
 * These are weak symbols that will be used if the version_builder.py
 * script does not generate the actual version strings. This allows
 * the build to succeed even without the generated version files.
 *
 * If version_builder.py generates qc_version.c and oem_version.c,
 * those strong symbols will override these weak defaults.
 */

#include <cdefs.h>

#define BL31QTILIB_WEAK	__attribute__((__weak__))

char BL31QTILIB_WEAK __used __section(".rodata")
QC_IMAGE_VERSION_STRING_AUTO_UPDATED[] =
	"QC_IMAGE_VERSION_STRING=UNKNOWN";

char BL31QTILIB_WEAK __used __section(".rodata")
IMAGE_VARIANT_STRING_AUTO_UPDATED[] = "IMAGE_VARIANT_STRING=UNKNOWN";

char BL31QTILIB_WEAK __used __section(".rodata")
OEM_IMAGE_VERSION_STRING_AUTO_UPDATED[] =
	"OEM_IMAGE_VERSION_STRING=UNKNOWN";

char BL31QTILIB_WEAK __used __section(".rodata")
OEM_IMAGE_UUID_STRING_AUTO_UPDATED[] = "OEM_IMAGE_UUID_STRING=UNKNOWN";

char BL31QTILIB_WEAK __used __section(".rodata")
OEM_HOST_TIMESTAMP_STRING_AUTO_UPDATED[] =
	"OEM_HOST_TIMESTAMP_STRING=UNKNOWN";

#undef BL31QTILIB_WEAK
