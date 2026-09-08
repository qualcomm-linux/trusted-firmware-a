/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NORD_INTS_H
#define NORD_INTS_H

/* Chipset specific secure interrupt number/ID defs. */
#define NORD_INT_ID_CPU_WAKEUP_SGI			(0x8)
#define NORD_INT_ID_RESET_SGI				(0xf)

/*
 * Values below are Nord GIC INTIDs (SPI + 32) taken from the IPCAT interrupt
 * map for nordschleife_2.0. Only the interrupts that map to a real Nord APSS
 * GIC destination are defined here; the Hoya VMIDMT config/client split and
 * the per-NoC error IDs have no Nord equivalent and were dropped.
 */
#define NORD_INT_ID_SEC_WDOG_BARK			(0x50)	/* WDT_barkInt_sec,  spi48 */
#define NORD_INT_ID_NON_SEC_WDOG_BITE			(0x39)	/* WDT_biteInt_nsec, spi25 */

#define NORD_INT_ID_VMIDMT_ERR_CLT_SEC			(0xE6)	/* vmidmt_grouped_summary_irq_apss, spi198 */

#define NORD_INT_ID_XPU_SEC				(0xE3)	/* xpu4_malicious_summary_irq_apss, spi195 */

#endif /* NORD_INTS_H */
