/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Upstream placeholder log IDs for the TME driver.
 *
 * These are passed as the first argument to TFA_LOG_ERR / TFA_LOG_ERR_FATAL.
 * The values are arbitrary non-zero identifiers used only for log tracing;
 * they do not need to match any downstream tzbsp_errno.h numbering.
 */

#ifndef TME_LOG_IDS_H
#define TME_LOG_IDS_H

/* TmeMessage.c log IDs */
#define TZBSP_TME_MESSAGE_ENCODE_MESSAGE_QCBOR_ERROR    0x544D4501U
#define TZBSP_TME_MESSAGE_ENCODE_MESSAGE_LENGTH_ERROR   0x544D4502U
#define TZBSP_TME_MESSAGE_DECODE_MESSAGE_QCBOR_ERROR    0x544D4503U
#define TZBSP_TME_MESSAGE_DECODE_MESSAGE_TAG_ERROR      0x544D4504U
#define TZBSP_TME_MESSAGE_DECODE_MESSAGE_DATA_ERROR     0x544D4505U
#define TZBSP_TME_MESSAGE_DECODE_MESSAGE_TYPE_ERROR     0x544D4506U
#define TZBSP_TME_MESSAGE_DECODE_MESSAGE_BUF_COPY_ERROR 0x544D4507U
#define TZBSP_TME_MESSAGE_CLIENT_INVALID                0x544D4508U
#define TZBSP_TME_MESSAGE_ALLOCATE_FAILED               0x544D4509U
#define TZBSP_TME_MESSAGE_ENCODE_MESSAGE_ERROR          0x544D450AU
#define TZBSP_TME_MESSAGE_DECODE_MESSAGE_ERROR          0x544D450BU
#define TZBSP_TME_MESSAGE_TRANSCEIVE_FAIL               0x544D450CU

/* TmeNotifyErrFatal.c log IDs (used only when TMECOM_NOTIFY_ERROR_FATAL) */
#define TZBSP_TME_ERROR_FATAL                  0x544D4520U
#define TZBSP_TME_INTERRUPT_REGISTRATION_FAIL  0x544D4521U

/* Platform-specific interrupt number and description for TMECOM_NOTIFY_ERROR_FATAL.
 * Override per-platform before including tme.mk if needed. */
#ifndef TZBSP_INT_TME_ERR
#define TZBSP_INT_TME_ERR       0U
#define TZBSP_INT_TME_ERR_DESC  "tme_err_fatal"
#endif

#endif /* TME_LOG_IDS_H */
