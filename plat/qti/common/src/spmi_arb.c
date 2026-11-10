/*
 * Copyright (c) 2020, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <platform_def.h>
#include <spmi_arb.h>

/*
 * APID table base and per-channel register base/stride differ across PMIC
 * arbiter hardware versions. Platforms override these in platform_def.h
 * when their arbiter instance doesn't match the defaults below.
 */
#ifndef SPMI_ARB_APID_MAP_BASE
#define SPMI_ARB_APID_MAP_BASE	0x0C440900U
#endif
#ifndef SPMI_ARB_CHNL_BASE
#define SPMI_ARB_CHNL_BASE	0x0C600000U
#endif
#ifndef SPMI_ARB_CHNL_STRIDE
#define SPMI_ARB_CHNL_STRIDE	0x10000U
#endif

#define REG_APID_MAP(apid)	((uint32_t)(SPMI_ARB_APID_MAP_BASE + sizeof(uint32_t) * apid))
#ifndef SPMI_ARB_NUM_APID
#define SPMI_ARB_NUM_APID	((0x1100U - 0x900U) / sizeof(uint32_t))
#endif
#define NUM_APID		SPMI_ARB_NUM_APID

#define PPID_MASK		(0xfffU << 8)

#define REG_ARB_CMD(apid)	((uint32_t)(SPMI_ARB_CHNL_BASE + SPMI_ARB_CHNL_STRIDE * apid))
/* These are opcodes specific to this SPMI arbitrator, *not* SPMI commands. */
#define OPC_EXT_WRITEL		0
#define OPC_EXT_READL		1

#define REG_ARB_STATUS(apid)	((uint32_t)(SPMI_ARB_CHNL_BASE + 0x8U + SPMI_ARB_CHNL_STRIDE * apid))
#define ARB_STATUS_DONE		BIT(0)
#define ARB_STATUS_FAILURE	BIT(1)
#define ARB_STATUS_DENIED	BIT(2)
#define ARB_STATUS_DROPPED	BIT(3)

/* Fake status to report driver errors. */
#define ARB_FAKE_STATUS_TIMEOUT	BIT(8)

#define REG_ARB_RDATA0(apid)	((uint32_t)(SPMI_ARB_CHNL_BASE + 0x18U + SPMI_ARB_CHNL_STRIDE * apid))
#define REG_ARB_WDATA0(apid)	((uint32_t)(SPMI_ARB_CHNL_BASE + 0x10U + SPMI_ARB_CHNL_STRIDE * apid))

#if defined(SPMI_ARB_OWNER_MAP_BASE) && defined(SPMI_ARB_OWNER_STRIDE) && defined(SPMI_ARB_OWNER_EE)
#define SPMI_ARB_OWNER_CHECK_ENABLED	1
#define REG_APID_OWNER(apid)	((uint32_t)(SPMI_ARB_OWNER_MAP_BASE + SPMI_ARB_OWNER_STRIDE * apid))
#define OWNER_FIELD_MASK	0xfU
#endif

static int addr_to_apid(uint32_t addr)
{
	unsigned int i;
#ifdef SPMI_ARB_OWNER_CHECK_ENABLED
	int fallback_apid = -1;
#endif

	for (i = 0U; i < NUM_APID; i++) {
		uint32_t reg = mmio_read_32(REG_APID_MAP(i));

		if ((reg != 0U) && ((addr & PPID_MASK) == (reg & PPID_MASK))) {
#ifdef SPMI_ARB_OWNER_CHECK_ENABLED
			uint32_t owner_reg = mmio_read_32(REG_APID_OWNER(i));
			uint32_t owner = owner_reg & OWNER_FIELD_MASK;

			if (owner == (uint32_t)SPMI_ARB_OWNER_EE) {
				return i;
			}
			if (fallback_apid < 0) {
				fallback_apid = (int)i;
			}
			continue;
#else
			return i;
#endif
		}
	}

#ifdef SPMI_ARB_OWNER_CHECK_ENABLED
	if (fallback_apid >= 0) {
		WARN("SPMI_ARB: no apid for addr=0x%x owned by this EE=%u; falling back to apid=%d owned by another EE -- HW will likely deny the access\n",
		     addr, (unsigned int)SPMI_ARB_OWNER_EE, fallback_apid);
		return fallback_apid;
	}
#endif

	return -1;
}

static int wait_for_done(uint16_t apid)
{
	unsigned int timeout = 100;

	while (timeout-- != 0U) {
		uint32_t status = mmio_read_32(REG_ARB_STATUS(apid));
		if ((status & ARB_STATUS_DONE) != 0U) {
			if ((status & ARB_STATUS_FAILURE) != 0U ||
			    (status & ARB_STATUS_DENIED) != 0U ||
			    (status & ARB_STATUS_DROPPED) != 0U) {
				return status & 0xff;
			}
			return 0;
		}
		mdelay(1);
	}
	ERROR("SPMI_ARB timeout!\n");
	return ARB_FAKE_STATUS_TIMEOUT;
}

static void arb_command(uint16_t apid, uint8_t opcode, uint32_t addr,
			uint8_t bytes)
{
	mmio_write_32(REG_ARB_CMD(apid), (uint32_t)opcode << 27 |
					 (addr & 0xff) << 4 | (bytes - 1));
}

int spmi_arb_read8(uint32_t addr)
{
	int apid = addr_to_apid(addr);

	if (apid < 0) {
		return apid;
	}

	arb_command(apid, OPC_EXT_READL, addr, 1);

	int ret = wait_for_done(apid);
	if (ret != 0) {
		ERROR("SPMI_ARB read error [0x%x]: 0x%x\n", addr, ret);
		return ret;
	}

	return mmio_read_32(REG_ARB_RDATA0(apid)) & 0xff;
}

int spmi_arb_write8(uint32_t addr, uint8_t data)
{
	int apid = addr_to_apid(addr);

	if (apid < 0) {
		return apid;
	}

	mmio_write_32(REG_ARB_WDATA0(apid), data);
	arb_command(apid, OPC_EXT_WRITEL, addr, 1);

	int ret = wait_for_done(apid);
	if (ret != 0) {
		ERROR("SPMI_ARB write error [0x%x] = 0x%x: 0x%x\n",
		      addr, data, ret);
	}

	return ret;
}
