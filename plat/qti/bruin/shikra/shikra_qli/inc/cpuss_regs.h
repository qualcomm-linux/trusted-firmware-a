/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPUSS_REGS_H
#define CPUSS_REGS_H

/*----------------------------------------------------------------------------*/
/* APSS INTU - interrupt type configuration registers                         */
/* HWIO_APSS_SHARED_SPI_CONFIG_n_ADDR(0) = APSS_INTU_BASE + 0x00F0           */
/* (APSS_HM_BASE=0x0F000000, INTU offset=0x400000)                           */
/*----------------------------------------------------------------------------*/
#define APSS_INTU_BASE			0x0F400000
#define APSS_INTU_SIZE			0x1000
#define APSS_SPI_CONFIG_BASE		(APSS_INTU_BASE + 0x00F0U)	/* 0x0F4000F0 */
#define APSS_SPI_NUM_WORDS		19U			/* SPI[0:607] */

#endif /* CPUSS_REGS_H */