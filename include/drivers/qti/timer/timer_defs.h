/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TIMER_DEFS_H
#define TIMER_DEFS_H

#include <stdint.h>
#include <lib/mmio.h>
#include <qtimer_defs.h>

/* Qtimer frequency (19.2MHz) in 100KHz */
#define QTMR_FREQ_IN_100KHZ 192ULL

/* Base address of frame 2 of primary/active Qtimer in TZ */
#define APSS_QTMR_FRAME_2_BASE \
	(APSS_PRIMARY_QTMR_BASE + PRIMARY_QTMR_FRAME2_OFFSET_FROM_BASE)

/*
 * Register: APSS_QTMR0_F2V1_QTMR_V1_CNTP_CTL
 *
 * [bits]  [Field Name]  [Field Values]
 *   2        ISTAT      0-INTERRUPT_NOT_PENDING / 1-INTERRUPT_PENDING
 *   1        IMSK       0-UNMASK_INTERRUPT / 1-MASK_INTERRUPT
 *   0        EN         0-DISABLED / 1-ENABLED
 */
#define APSS_QTMR0_F2V1_QTMR_V1_CNTP_CTL_ADDR \
	(APSS_QTMR_FRAME_2_BASE + 0x0000002c)
#define disable_qtimer() mmio_write_32(APSS_QTMR0_F2V1_QTMR_V1_CNTP_CTL_ADDR, 0)
#define enable_qtimer() mmio_write_32(APSS_QTMR0_F2V1_QTMR_V1_CNTP_CTL_ADDR, 1)

/* Register: APSS_QTMR0_F2V1_QTMR_V1_CNTPCT_HI - Holds upper [23:0] bytes of QTimer current count */
#define APSS_QTMR0_F2V1_QTMR_V1_CNTPCT_HI_RMSK 0xffffff
#define APSS_QTMR0_F2V1_QTMR_V1_CNTPCT_HI_ADDR \
	(APSS_QTMR_FRAME_2_BASE + 0x00000004)
#define read_qtimer_hi()                                        \
	(mmio_read_32(APSS_QTMR0_F2V1_QTMR_V1_CNTPCT_HI_ADDR) & \
		APSS_QTMR0_F2V1_QTMR_V1_CNTPCT_HI_RMSK)

/* Register: APSS_QTMR0_F2V1_QTMR_V1_CNTPCT_LO - Holds lower [31:0] bytes of QTimer current count */
#define APSS_QTMR0_F2V1_QTMR_V1_CNTPCT_LO_RMSK 0xffffffff
#define APSS_QTMR0_F2V1_QTMR_V1_CNTPCT_LO_ADDR \
	(APSS_QTMR_FRAME_2_BASE + 0x00000000)
#define read_qtimer_lo()                                        \
	(mmio_read_32(APSS_QTMR0_F2V1_QTMR_V1_CNTPCT_LO_ADDR) & \
		APSS_QTMR0_F2V1_QTMR_V1_CNTPCT_LO_RMSK)

/* Register: APSS_QTMR0_F2V1_QTMR_V1_CNTP_CVAL_LO - Lower [31:0] bytes to set QTimer count */
#define APSS_QTMR0_F2V1_QTMR_V1_CNTP_CVAL_LO_ADDR \
	(APSS_QTMR_FRAME_2_BASE + 0x00000020)
#define write_qtimer_lo(ticks) \
	mmio_write_32(APSS_QTMR0_F2V1_QTMR_V1_CNTP_CVAL_LO_ADDR, LS32(ticks))

/* Register: APSS_QTMR0_F2V1_QTMR_V1_CNTP_CVAL_HI - Higher [23:0] bytes to set QTimer count */
#define APSS_QTMR0_F2V1_QTMR_V1_CNTP_CVAL_HI_ADDR \
	(APSS_QTMR_FRAME_2_BASE + 0x00000024)
#define write_qtimer_hi(ticks) \
	mmio_write_32(APSS_QTMR0_F2V1_QTMR_V1_CNTP_CVAL_HI_ADDR, MS32(ticks))

/* ------- Qtimer Frame 2 macro definitions end ------- */

/* Base address of frame 3 of primary/active Qtimer */
#define APSS_QTMR_FRAME_3_BASE \
	(APSS_PRIMARY_QTMR_BASE + PRIMARY_QTMR_FRAME3_OFFSET_FROM_BASE)

/*
 * Register: APSS_QTMR0_F3V1_QTMR_V1_CNTP_CTL
 *
 * [bits]  [Field Name]  [Field Values]
 *   2        ISTAT      0-INTERRUPT_NOT_PENDING / 1-INTERRUPT_PENDING
 *   1        IMSK       0-UNMASK_INTERRUPT / 1-MASK_INTERRUPT
 *   0        EN         0-DISABLED / 1-ENABLED
 */
#define APSS_QTMR0_F3V1_QTMR_V1_CNTP_CTL_ADDR \
	(APSS_QTMR_FRAME_3_BASE + 0x0000002c)
#define disable_qtimer_fr3() mmio_write_32(APSS_QTMR0_F3V1_QTMR_V1_CNTP_CTL_ADDR, 0)
#define enable_qtimer_fr3() mmio_write_32(APSS_QTMR0_F3V1_QTMR_V1_CNTP_CTL_ADDR, 1)

/* Register: APSS_QTMR0_F3V1_QTMR_V1_CNTPCT_HI - Holds upper [23:0] bytes of QTimer current count */
#define APSS_QTMR0_F3V1_QTMR_V1_CNTPCT_HI_RMSK 0xffffff
#define APSS_QTMR0_F3V1_QTMR_V1_CNTPCT_HI_ADDR \
	(APSS_QTMR_FRAME_3_BASE + 0x00000004)
#define read_qtimer_fr3_hi()                                    \
	(mmio_read_32(APSS_QTMR0_F3V1_QTMR_V1_CNTPCT_HI_ADDR) & \
		APSS_QTMR0_F3V1_QTMR_V1_CNTPCT_HI_RMSK)

/* Register: APSS_QTMR0_F3V1_QTMR_V1_CNTPCT_LO - Holds lower [31:0] bytes of QTimer current count */
#define APSS_QTMR0_F3V1_QTMR_V1_CNTPCT_LO_RMSK 0xffffffff
#define APSS_QTMR0_F3V1_QTMR_V1_CNTPCT_LO_ADDR \
	(APSS_QTMR_FRAME_3_BASE + 0x00000000)
#define read_qtimer_fr3_lo()                                    \
	(mmio_read_32(APSS_QTMR0_F3V1_QTMR_V1_CNTPCT_LO_ADDR) & \
		APSS_QTMR0_F3V1_QTMR_V1_CNTPCT_LO_RMSK)

/* Register: APSS_QTMR0_F3V1_QTMR_V1_CNTP_CVAL_LO - Lower [31:0] bytes to set QTimer count */
#define APSS_QTMR0_F3V1_QTMR_V1_CNTP_CVAL_LO_ADDR \
	(APSS_QTMR_FRAME_3_BASE + 0x00000020)
#define write_qtimer_fr3_lo(ticks) \
	mmio_write_32(APSS_QTMR0_F3V1_QTMR_V1_CNTP_CVAL_LO_ADDR, LS32(ticks))

/* Register: APSS_QTMR0_F3V1_QTMR_V1_CNTP_CVAL_HI - Higher [23:0] bytes to set QTimer count */
#define APSS_QTMR0_F3V1_QTMR_V1_CNTP_CVAL_HI_ADDR \
	(APSS_QTMR_FRAME_3_BASE + 0x00000024)
#define write_qtimer_fr3_hi(ticks) \
	mmio_write_32(APSS_QTMR0_F3V1_QTMR_V1_CNTP_CVAL_HI_ADDR, MS32(ticks))

/* ------- Qtimer Frame 3 macro definitions end ------- */

/* Base address of frame 4 of primary/active Qtimer */
#define APSS_QTMR_FRAME_4_BASE \
	(APSS_PRIMARY_QTMR_BASE + PRIMARY_QTMR_FRAME4_OFFSET_FROM_BASE)

/*
 * Register: APSS_QTMR0_F4V1_QTMR_V1_CNTP_CTL
 *
 * [bits]  [Field Name]  [Field Values]
 *   2        ISTAT      0-INTERRUPT_NOT_PENDING / 1-INTERRUPT_PENDING
 *   1        IMSK       0-UNMASK_INTERRUPT / 1-MASK_INTERRUPT
 *   0        EN         0-DISABLED / 1-ENABLED
 */
#define APSS_QTMR0_F4V1_QTMR_V1_CNTP_CTL_ADDR \
	(APSS_QTMR_FRAME_4_BASE + 0x0000002c)
#define disable_qtimer_fr4() mmio_write_32(APSS_QTMR0_F4V1_QTMR_V1_CNTP_CTL_ADDR, 0)
#define enable_qtimer_fr4() mmio_write_32(APSS_QTMR0_F4V1_QTMR_V1_CNTP_CTL_ADDR, 1)

/* Register: APSS_QTMR0_F4V1_QTMR_V1_CNTPCT_HI - Holds upper [23:0] bytes of QTimer current count */
#define APSS_QTMR0_F4V1_QTMR_V1_CNTPCT_HI_RMSK 0xffffff
#define APSS_QTMR0_F4V1_QTMR_V1_CNTPCT_HI_ADDR \
	(APSS_QTMR_FRAME_4_BASE + 0x00000004)
#define read_qtimer_fr4_hi()                                    \
	(mmio_read_32(APSS_QTMR0_F4V1_QTMR_V1_CNTPCT_HI_ADDR) & \
		APSS_QTMR0_F4V1_QTMR_V1_CNTPCT_HI_RMSK)

/* Register: APSS_QTMR0_F4V1_QTMR_V1_CNTPCT_LO - Holds lower [31:0] bytes of QTimer current count */
#define APSS_QTMR0_F4V1_QTMR_V1_CNTPCT_LO_RMSK 0xffffffff
#define APSS_QTMR0_F4V1_QTMR_V1_CNTPCT_LO_ADDR \
	(APSS_QTMR_FRAME_4_BASE + 0x00000000)
#define read_qtimer_fr4_lo()                                    \
	(mmio_read_32(APSS_QTMR0_F4V1_QTMR_V1_CNTPCT_LO_ADDR) & \
		APSS_QTMR0_F4V1_QTMR_V1_CNTPCT_LO_RMSK)

/* Register: APSS_QTMR0_F4V1_QTMR_V1_CNTP_CVAL_LO - Lower [31:0] bytes to set QTimer count */
#define APSS_QTMR0_F4V1_QTMR_V1_CNTP_CVAL_LO_ADDR \
	(APSS_QTMR_FRAME_4_BASE + 0x00000020)
#define write_qtimer_fr4_lo(ticks) \
	mmio_write_32(APSS_QTMR0_F4V1_QTMR_V1_CNTP_CVAL_LO_ADDR, LS32(ticks))

/* Register: APSS_QTMR0_F4V1_QTMR_V1_CNTP_CVAL_HI - Higher [23:0] bytes to set QTimer count */
#define APSS_QTMR0_F4V1_QTMR_V1_CNTP_CVAL_HI_ADDR \
	(APSS_QTMR_FRAME_4_BASE + 0x00000024)
#define write_qtimer_fr4_hi(ticks) \
	mmio_write_32(APSS_QTMR0_F4V1_QTMR_V1_CNTP_CVAL_HI_ADDR, MS32(ticks))

/* ------- Qtimer Frame 4 macro definitions end ------- */

/* Base address of frame 5 of primary/active Qtimer */
#define APSS_QTMR_FRAME_5_BASE \
	(APSS_PRIMARY_QTMR_BASE + PRIMARY_QTMR_FRAME5_OFFSET_FROM_BASE)

/*
 * Register: APSS_QTMR0_F5V1_QTMR_V1_CNTP_CTL
 *
 * [bits]  [Field Name]  [Field Values]
 *   2        ISTAT      0-INTERRUPT_NOT_PENDING / 1-INTERRUPT_PENDING
 *   1        IMSK       0-UNMASK_INTERRUPT / 1-MASK_INTERRUPT
 *   0        EN         0-DISABLED / 1-ENABLED
 */
#define APSS_QTMR0_F5V1_QTMR_V1_CNTP_CTL_ADDR \
	(APSS_QTMR_FRAME_5_BASE + 0x0000002c)
#define disable_qtimer_fr5() mmio_write_32(APSS_QTMR0_F5V1_QTMR_V1_CNTP_CTL_ADDR, 0)
#define enable_qtimer_fr5() mmio_write_32(APSS_QTMR0_F5V1_QTMR_V1_CNTP_CTL_ADDR, 1)

/* Register: APSS_QTMR0_F5V1_QTMR_V1_CNTPCT_HI - Holds upper [23:0] bytes of QTimer current count */
#define APSS_QTMR0_F5V1_QTMR_V1_CNTPCT_HI_RMSK 0xffffff
#define APSS_QTMR0_F5V1_QTMR_V1_CNTPCT_HI_ADDR \
	(APSS_QTMR_FRAME_5_BASE + 0x00000004)
#define read_qtimer_fr5_hi()                                    \
	(mmio_read_32(APSS_QTMR0_F5V1_QTMR_V1_CNTPCT_HI_ADDR) & \
		APSS_QTMR0_F5V1_QTMR_V1_CNTPCT_HI_RMSK)

/* Register: APSS_QTMR0_F5V1_QTMR_V1_CNTPCT_LO - Holds lower [31:0] bytes of QTimer current count */
#define APSS_QTMR0_F5V1_QTMR_V1_CNTPCT_LO_RMSK 0xffffffff
#define APSS_QTMR0_F5V1_QTMR_V1_CNTPCT_LO_ADDR \
	(APSS_QTMR_FRAME_5_BASE + 0x00000000)
#define read_qtimer_fr5_lo()                                    \
	(mmio_read_32(APSS_QTMR0_F5V1_QTMR_V1_CNTPCT_LO_ADDR) & \
		APSS_QTMR0_F5V1_QTMR_V1_CNTPCT_LO_RMSK)

/* Register: APSS_QTMR0_F5V1_QTMR_V1_CNTP_CVAL_LO - Lower [31:0] bytes to set QTimer count */
#define APSS_QTMR0_F5V1_QTMR_V1_CNTP_CVAL_LO_ADDR \
	(APSS_QTMR_FRAME_5_BASE + 0x00000020)
#define write_qtimer_fr5_lo(ticks) \
	mmio_write_32(APSS_QTMR0_F5V1_QTMR_V1_CNTP_CVAL_LO_ADDR, LS32(ticks))

/* Register: APSS_QTMR0_F5V1_QTMR_V1_CNTP_CVAL_HI - Higher [23:0] bytes to set QTimer count */
#define APSS_QTMR0_F5V1_QTMR_V1_CNTP_CVAL_HI_ADDR \
	(APSS_QTMR_FRAME_5_BASE + 0x00000024)
#define write_qtimer_fr5_hi(ticks) \
	mmio_write_32(APSS_QTMR0_F5V1_QTMR_V1_CNTP_CVAL_HI_ADDR, MS32(ticks))

/* ------- Qtimer Frame 5 macro definitions end ------- */

/*
 * Register: APSS_QTMR0_QTMR_AC_CNTFRQ
 * Address offset to configure Qtimer counter frequency.
 */
#define APSS_QTMR_AC_CNTFRQ_RMSK 0xffffffff
#define APSS_QTMR_AC_CNTFRQ_ADDR_OFFSET 0x00000000

/*
 * Register: APSS_QTMR0_QTMR_AC_CNTNSAR_FG0
 * Address offset to configure Qtimer Counter Secure Reg to allow QTMR access.
 *
 * [bits]  [Field Name]    [Field Values]
 * [6:0]      NSN        0 - SECURE_ONLY / 1-SECURE_OR_NONSECURE
 */
#define APSS_QTMR_AC_CNTNSAR_FG0_ADDR_OFFSET 0x4

/*
 * Register: APSS_QTMR0_QTMR_AC_CNTACRn_FG0, n=[0..6]
 * Address offset for configuring Qtimer global access.
 *
 * [bits]  [Field Name]     [Field Values]
 *   5        RWPT        0-ACCESS_DENIED / 1-ACCESS_ALLOWED
 *   4        RWVT        0-ACCESS_DENIED / 1-ACCESS_ALLOWED
 *   3        RVOFF       0-ACCESS_DENIED / 1-ACCESS_ALLOWED
 *   2        RFRQ        0-ACCESS_DENIED / 1-ACCESS_ALLOWED
 *   1        RPVCT       0-ACCESS_DENIED / 1-ACCESS_ALLOWED
 *   0        RPCT        0-ACCESS_DENIED / 1-ACCESS_ALLOWED
 */
#define APSS_QTMR_AC_CNTACRn_FG0_ADDR_OFFSET(n) (0x40 + 0x4 * (n))

/*
 * Register: APSS_QTMR0_QTMR_AC_CNTVOFF_FG0_LO_n and
 *           APSS_QTMR0_QTMR_AC_CNTVOFF_FG0_HI_n
 * Address offsets for virtual offset registers.
 */
#define APSS_QTMR_AC_CNTVOFF_FG0_LO_n_ADDR_OFFSET(n) (0x80 + 0x8 * (n))
#define APSS_QTMR_AC_CNTVOFF_FG0_HI_n_ADDR_OFFSET(n) (0x84 + 0x8 * (n))

#endif /* TIMER_DEFS_H */
