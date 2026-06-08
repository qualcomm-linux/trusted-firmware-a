/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_NOC_ERROR_PLATFORM_H
#define QTI_NOC_ERROR_PLATFORM_H

#include "noc_error.h"

struct nocerr_propdata *qti_noc_error_platform_get_propdata(void);
struct nocerr_propdata_oem *qti_noc_error_platform_get_propdata_oem(void);

#endif /* QTI_NOC_ERROR_PLATFORM_H */
