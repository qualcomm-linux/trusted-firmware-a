/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _QTI_CPU_ID_H_
#define _QTI_CPU_ID_H_

//Partnum ids to distinguish between power and perf cores
#define MIDR_PARTNUM_PERF     0x001
#define MIDR_PARTNUM_POWER    0x002

#define MIDR_REV_CLEAR_MASK         ~MIDR_REV_MASK
#define MIDR_VAR_REV_MASK           0x00F0000F
#define MIDR_VAR_REV_CLEAR_MASK     ~MIDR_VAR_REV_MASK
#define MIDR_PARTNUM_MASK           0x0000FFF0
#define CLUSTERIDR_VAR_REV_MASK     0x000000FF

/* Variant <var> and Revision <rev> denoted as - "MIDR_VERSION_R<var>P<rev>" */
#define MIDR_VERSION_R0P0           0x00000000
#define MIDR_VERSION_R0P1           0x00000001
#define MIDR_VERSION_R0P2           0x00000002
#define MIDR_VERSION_R0P3           0x00000003
#define MIDR_VERSION_R1P0           0x00100000
#define MIDR_VERSION_R1P1           0x00100001
#define MIDR_VERSION_R1P2           0x00100002
#define MIDR_VERSION_R2P0           0x00200000
#define MIDR_VERSION_R2P1           0x00200001
#define MIDR_VERSION_R3P0           0x00300000
#define MIDR_VERSION_R3P1           0x00300001
#define MIDR_VERSION_R3P2           0x00300002
#define MIDR_VERSION_R3P3           0x00300003
#define MIDR_VERSION_R3P4           0x00300004
#define MIDR_VERSION_R4P0           0x00400000
#define MIDR_VERSION_R4P1           0x00400001
#define MIDR_VERSION_R4P2           0x00400002
#define MIDR_VERSION_R4P3           0x00400003
#define MIDR_VERSION_R4P4           0x00400004

#define CLUSTERIDR_VERSION_R0P0     0x00000000
#define CLUSTERIDR_VERSION_R1P0     0x00000010
#define CLUSTERIDR_VERSION_R2P0     0x00000020
#define CLUSTERIDR_VERSION_R2P1     0x00000021
#define CLUSTERIDR_VERSION_R3P0     0x00000030
#define CLUSTERIDR_VERSION_R3P1     0x00000031
#define CLUSTERIDR_VERSION_R4P0     0x00000040
#endif // _QTI_CPU_ID_H_
