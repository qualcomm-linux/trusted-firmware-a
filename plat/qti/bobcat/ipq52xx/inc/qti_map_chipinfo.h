/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */
#ifndef QTI_MAP_CHIPINFO_H
#define QTI_MAP_CHIPINFO_H

#include <qti_plat.h>

#define QTI_JTAG_ID_REG		U(0xA607C)
#define QTI_JTAG_ID_SHIFT	U(12)

#define QTI_JTAG_ID_IPQ5210	U(0x0349)
#define QTI_CHIPINFO_ID_IPQ5210	U(0x2FE)
#define QTI_DEFAULT_CHIPINFO_ID	U(0xFFFF)

static const chip_id_info_t g_map_jtag_chipinfo_id[] = {
	{QTI_JTAG_ID_IPQ5210, QTI_CHIPINFO_ID_IPQ5210},
};
#endif /* QTI_MAP_CHIPINFO_H */
