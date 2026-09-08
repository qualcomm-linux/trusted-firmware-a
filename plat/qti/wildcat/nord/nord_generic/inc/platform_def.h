/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <nord_def.h>

#define MAX_IO_HANDLES			U(2)
#define MAX_IO_DEVICES			U(2)
#define MAX_IO_BLOCK_DEVICES		U(1)

/*
 * Number of authenticated boot-image entries shared by XBL via the
 * boot_qsee_interface. Must match XBL/TZ (BOOT_IMAGES_NUM_ENTRY = 45).
 */
#define BOOT_IMAGES_NUM_ENTRIES		45

/*
 * Nord EL3 image memory map.
 *
 * BL2 is the reset entry image (RESET_TO_BL2=1) and runs before DDR is
 * trained, so it executes from on-chip SRAM/SYSTEM_IMEM. Nord XBL loads and
 * jumps to BL2 at the handoff address below. The 256KB window holds the BL2
 * image (~80KB text+data+bss).
 *
 * BL31 and BL32 run after DDR is up and sit in the secure DARE-TZ carveout
 * (per the Nord memory map: DDR_0 DARE-TZ, 0xBC000000..0xC6300000, 163MB).
 * Within that carveout the QTEE region (0xBC180000, 6.5MB) holds the secure
 * monitor and trusted OS: BL31 (TF-A EL3) occupies the first 1MB and BL32
 * (OP-TEE, SPD=opteed) the remaining 5.5MB. The 120MB "TA" region above
 * (0xBC800000) is OP-TEE's trusted-application memory pool. BL33 is
 * non-secure and is placed in free DRAM above the firmware reservations
 * (0xA0200000); BL2 maps each image dynamically while loading, so the BL3x
 * regions need not be contiguous.
 *
 * NOTE: BL2_BASE is the Nord XBL handoff address (inherited; reconcile the
 * SYSTEM_IMEM offset with the production XBL before flashing). The DDR
 * carveout addresses below are a self-consistent placeholder verified against
 * the Nord DDR memory map; reconcile sizes with the production layout.
 */
#define BL2_BASE			0x14694000
#define BL2_SIZE			0x40000
#define BL2_LIMIT			(BL2_BASE + BL2_SIZE)

/*
 * BL2 reads the FIP from memory (io_memmap). XBL loads the signed FIP
 * image into the non-secure DRAM region that the stock UEFI image used,
 * so the I/O base points there rather than the BL3x carveout above.
 */
#define PLAT_QTI_FIP_IOBASE		0xd8d00000
/*
 * The FIP holds BL31 + BL32 (OP-TEE) + BL33 (the EDK2 firmware device). The
 * MinPlatform-based BL33 FD is ~7.5MB (DEBUG), so the FIP is ~8.4MB; BL2 maps
 * only PLAT_QTI_FIP_MAXSIZE of it via io_memmap, hence the 16MB window.
 */
#define PLAT_QTI_FIP_MAXSIZE		0x01000000

#define BL31_BASE			0xBC180000
#define BL31_SIZE			0x00100000
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

#define BL32_BASE			0xBC280000
#define BL32_SIZE			0x00580000
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE)

/*
 * BL33 is the EDK2 firmware device. The MinPlatform multi-FV FD is ~7.5MB
 * (DEBUG), so the load window is 10MB; the UEFI CPU stack (PcdCPUCoresStackBase)
 * must sit above BL33_BASE + BL33_SIZE. There is free non-secure DRAM up to the
 * DARE-TZ carveout at 0xBC000000.
 */
#define BL33_BASE			0xA0200000
#define BL33_SIZE			0x00A00000

#endif /* PLATFORM_DEF_H */
