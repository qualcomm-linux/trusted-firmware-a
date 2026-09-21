/**********************************************************************
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *********************************************************************/

#ifndef QTI_ERRNO_H
#define QTI_ERRNO_H
/**
 * Reserving 0 so that 0000000 error code is not used.
 */
#define QTI_MODULE_BASE_RSVD (uint32_t)0

/**
 * Macros to create error code, with module name in it
 */
#define QTI_MSK_NEGATIVE_BIT_SHFT (uint32_t)(31U)
#define QTI_MSK_RSVD_BIT_SHFT (uint32_t)(30U)
#define QTI_MSK_MODULE_BIT_SHFT (uint32_t)(20U)

#define QTI_MSK_NEGATIVE_MSK \
	((uint32_t)UINT32_C(0x1) << QTI_MSK_NEGATIVE_BIT_SHFT)

#define QTI_MSK_RSVD_MSK ((uint32_t)UINT32_C(0x1) << QTI_MSK_RSVD_BIT_SHFT)

#define QTI_MSK_MODULE_MSK \
	((uint32_t)UINT32_C(0x3FF) << QTI_MSK_MODULE_BIT_SHFT)

#define QTI_MSK_CODE_MSK (0xFFFFFU)

#define QTI_NEG_ERR_CODE(QTI_MODULE, CODE)                           \
	(int)(((QTI_MSK_MODULE_MSK &                                 \
		((uint32_t)QTI_MODULE << QTI_MSK_MODULE_BIT_SHFT)) | \
	       (QTI_MSK_CODE_MSK & (CODE))) |                        \
	      (QTI_MSK_RSVD_MSK &                                    \
	       (QTI_MODULE_BASE_RSVD << QTI_MSK_RSVD_BIT_SHFT)) |    \
	      (QTI_MSK_NEGATIVE_MSK))

#define QTI_PFM_ARMLIB (0x11U) /* SSG Platform Armlib */

#define ARMLIB_E_TLB_CTRL_ALLEL1_NOT_ALLOWED \
	QTI_NEG_ERR_CODE(QTI_PFM_ARMLIB, 151U)

#define ARMLIB_E_TLB_CTRL_ALLEL1IS_NOT_ALLOWED \
	QTI_NEG_ERR_CODE(QTI_PFM_ARMLIB, 152U)

#define ARMLIB_E_TLB_CTRL_OP_NOT_SUPPORTED \
	QTI_NEG_ERR_CODE(QTI_PFM_ARMLIB, 153U)

/*
 * Error number shared with HYP
 * 0x4________ = vendor defined
 * 0x________1 = fatal error
 */
#define SDEI_FATAL_ERROR 0x40000001

#endif /* QTI_ERRNO_H */
