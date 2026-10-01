/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FUSEPROV_TRANSPORT_H
#define FUSEPROV_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>

/* QFPROM address space used for a fuse operation. */
typedef enum {
        FUSEPROV_ADDR_RAW  = 0,
        FUSEPROV_ADDR_CORR = 1,
} fuseprov_addr_space_t;

/* Errors returned by a fuse transport. */
typedef enum {
        FUSEPROV_OK = 0,
        FUSEPROV_ERR_ADDR_INVALID,
        FUSEPROV_ERR_NOT_READABLE,
        FUSEPROV_ERR_NOT_WRITEABLE,
        FUSEPROV_ERR_FEC_ENABLED_NOT_WRITEABLE,
        FUSEPROV_ERR_VERIFY_FAILED,
        FUSEPROV_ERR_ROW_BOUNDARY,
        FUSEPROV_ERR_TRANSPORT,
        FUSEPROV_ERR_UNKNOWN,
} fuseprov_err_t;

/* Transport contract for QFPROM row access. */
typedef struct {
        /* Read one row.
         * @param ctx port-specific context
         * @param addr fuse row address
         * @param space raw or corrected address space
         * @param out output words in LSB, MSB order
         * @return FUSEPROV_OK on success, otherwise a transport error
         */
        fuseprov_err_t (*read_row)(void *ctx, uint32_t addr,
                                   fuseprov_addr_space_t space,
                                   uint32_t out[2]);

        /* Write multiple rows.
         * @param ctx port-specific context
         * @param addr row-address array
         * @param data row-data array
         * @param count number of rows
         * @param addr_err optional failed-address output
         * @return FUSEPROV_OK on success, otherwise a transport error
         */
        fuseprov_err_t (*write_rows)(void *ctx, const uint32_t addr[],
                                     const uint64_t data[], uint32_t count,
                                     uintptr_t *addr_err);

        /* Port-specific context passed to the callbacks. */
        void *ctx;
} fuseprov_transport_t;

#endif /* FUSEPROV_TRANSPORT_H */
