/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FUSEPROV_PORT_H
#define FUSEPROV_PORT_H

#include <stdint.h>
#include "fuseprov_transport.h"
#include "fuseprov_sec_elf_v3.h"

/* Read one QFPROM row through the selected transport.
 * @param t transport contract
 * @param addr fuse row address
 * @param space raw or corrected address space
 * @param out output words in LSB, MSB order
 * @return FUSEPROV_OK on success, otherwise a transport error
 */
fuseprov_err_t fuseprov_row_read(const fuseprov_transport_t *t,
                                 uint32_t addr,
                                 fuseprov_addr_space_t space,
                                 uint32_t out[2]);

/* Write multiple QFPROM rows through the selected transport.
 * @param t transport contract
 * @param addr row-address array
 * @param data row-data array, with LSB in bits [31:0]
 * @param count number of rows
 * @param addr_err optional failed-address output
 * @return FUSEPROV_OK on success, otherwise a transport error
 */
fuseprov_err_t fuseprov_rows_write(const fuseprov_transport_t *t,
                                   const uint32_t addr[],
                                   const uint64_t data[],
                                   uint32_t count,
                                   uintptr_t *addr_err);

/* Parse and provision an authenticated SEC.DAT v3 buffer.
 * @param t transport contract
 * @param buf SEC.DAT buffer
 * @param len SEC.DAT buffer length
 * @param did_program set true if a row was programmed
 * @return FUSEPROV_SUCCESS on success, otherwise a provisioning error
 */
fuseprov_error_etype fuseprov_blow_fuses_sec_elf_v3(
        const fuseprov_transport_t *t,
        uint8_t *buf,
        uint32_t len,
        bool *did_program);

#endif /* FUSEPROV_PORT_H */
