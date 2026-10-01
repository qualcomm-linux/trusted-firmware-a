/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <spmi_arb.h>
#include <spmi_arb_regs.h>
#include <spmi_platform.h>

/*
 * addr is a 32-bit value combining (U)SID, PPID and register address:
 *   bits[19:16] = SID, bits[15:8] = PPID, bits[7:0] = register offset,
 * matching the HWIO_PMIC_ARB_REG_ADDRp_ADDR SID/ADDRESS field layout.
 */
#define ADDR_SID(addr)			(((addr) >> 16) & 0xfU)
#define ADDR_PPID(addr)			(((addr) >> 8) & 0xffU)
#define ADDR_REG_OFFSET(addr)		((addr) & 0xffU)

/*
 * Owner/EE-ID-aware channel resolution. Peripherals may appear on more than
 * one channel (e.g. shared across owners); a channel whose SID matches is a
 * provisional match, but a channel that *also* matches TZ_OWNER_ID wins and
 * ends the search immediately. If no owner match exists anywhere, the
 * earlier SID-only match still stands -- this is not a hard failure.
 */
static int addr_to_channel(uint32_t addr)
{
	uint8_t target_sid = ADDR_SID(addr);
	uint8_t target_ppid = ADDR_PPID(addr);
	int found = -1;
	unsigned int chan;

	for (chan = 0U; chan < SPMI_ARB_NUM_CHANNELS; chan++) {
		uint32_t reg_addr = HWIO_PMIC_ARB_REG_ADDRp_ADDR(SPMI_ARB_BASE, chan);
		uint32_t reg = mmio_read_32(reg_addr);
		uint8_t sid, ppid;

		if (reg == 0U) {
			continue;
		}

		sid = (reg & HWIO_PMIC_ARB_REG_ADDRp_SID_BMSK) >>
			HWIO_PMIC_ARB_REG_ADDRp_SID_SHFT;
		ppid = (reg & HWIO_PMIC_ARB_REG_ADDRp_ADDRESS_BMSK) >>
			HWIO_PMIC_ARB_REG_ADDRp_ADDRESS_SHFT;

		if (sid != target_sid || ppid != target_ppid) {
			continue;
		}

		found = (int)chan;

		INFO("SPMI_ARB addr_to_channel: chan=%u match, REG_ADDRp=0x%x\n",
			chan, reg_addr);

		uint32_t owner_addr = HWIO_SPMI_PERIPHm_2OWNER_TABLE_REG_ADDR(SPMI_ARB_BASE, chan);
		uint32_t owner_reg = mmio_read_32(owner_addr);
		uint8_t owner = (owner_reg &
				 HWIO_SPMI_PERIPHm_2OWNER_TABLE_REG_PERIPH2OWNER_BMSK) >>
				HWIO_SPMI_PERIPHm_2OWNER_TABLE_REG_PERIPH2OWNER_SHFT;

		INFO("SPMI_ARB addr_to_channel: chan=%u 2OWNER_TABLE=0x%x, owner=%u\n",
			chan, owner_addr, owner);

		if (owner == TZ_OWNER_ID) {
			INFO("SPMI_ARB addr_to_channel: chan=%u owner match, using chan=%u\n",
				chan, chan);
			break;
		}
	}

	if (found < 0) {
		INFO("SPMI_ARB addr_to_channel: no SID/PPID match for addr=0x%x across %u channels\n",
			addr, SPMI_ARB_NUM_CHANNELS);
	} else if (chan == SPMI_ARB_NUM_CHANNELS) {
		INFO("SPMI_ARB addr_to_channel: no owner match for addr=0x%x, falling back to SID/PPID-only chan=%d\n",
			addr, found);
	}

	return found;
}

static int wait_for_done(unsigned int chan)
{
	unsigned int timeout = SPMI_ARB_TIMEOUT_ITER;
	uint32_t status_addr = HWIO_PMIC_ARB_CHNLn_STATUS_ADDR(SPMI_ARB_BASE, chan);

	INFO("SPMI_ARB wait_for_done: chan=%u CHNLn_STATUS=0x%x\n", chan, status_addr);

	while (timeout-- != 0U) {
		uint32_t status = mmio_read_32(status_addr);

		if ((status & HWIO_PMIC_ARB_CHNLn_STATUS_DONE_BMSK) != 0U) {
			if ((status & (HWIO_PMIC_ARB_CHNLn_STATUS_FAILURE_BMSK |
				       HWIO_PMIC_ARB_CHNLn_STATUS_DENIED_BMSK |
				       HWIO_PMIC_ARB_CHNLn_STATUS_DROPPED_BMSK)) != 0U) {
				return -(int)status;
			}
			return 0;
		}
		udelay(1);
	}

	ERROR("SPMI_ARB timeout!\n");
	return -1;
}

static void issue_command(unsigned int chan, uint8_t opcode, uint32_t addr)
{
	uint32_t cmd_addr = HWIO_PMIC_ARB_CHNLn_CMD_ADDR(SPMI_ARB_BASE, chan);
	uint32_t cmd = ((uint32_t)opcode << HWIO_PMIC_ARB_CHNLn_CMD_OPCODE_SHFT) |
			((ADDR_REG_OFFSET(addr) << HWIO_PMIC_ARB_CHNLn_CMD_ADDRESS_OFFSET_SHFT) &
			 HWIO_PMIC_ARB_CHNLn_CMD_ADDRESS_OFFSET_BMSK) |
			(0U & HWIO_PMIC_ARB_CHNLn_CMD_BYTE_CNT_BMSK); /* 1 byte */

	INFO("SPMI_ARB issue_command: chan=%u CHNLn_CMD=0x%x, cmd=0x%x\n",
		chan, cmd_addr, cmd);

	mmio_write_32(cmd_addr, cmd);
}

int spmi_arb_read8(uint32_t addr)
{
	int chan;
	int ret;
	uint8_t val;
	uint32_t rdata_addr;

	INFO("SPMI_ARB read: addr=0x%x, resolving channel\n", addr);

	chan = addr_to_channel(addr);
	if (chan < 0) {
		ERROR("SPMI_ARB read: no channel found for addr=0x%x\n", addr);
		return chan;
	}

	INFO("SPMI_ARB read: addr=0x%x resolved to chan=%d, issuing command\n",
		addr, chan);

	issue_command((unsigned int)chan, PMIC_ARB_CMD_EXTENDED_REG_READ_LONG, addr);

	INFO("SPMI_ARB read: chan=%d command issued, waiting for completion\n", chan);

	ret = wait_for_done((unsigned int)chan);
	if (ret != 0) {
		ERROR("SPMI_ARB read error [0x%x]: %d\n", addr, ret);
		return ret;
	}

	rdata_addr = HWIO_PMIC_ARB_CHNLn_RDATA0_ADDR(SPMI_ARB_BASE, chan);
	val = mmio_read_32(rdata_addr) & 0xffU;

	INFO("SPMI_ARB read: addr=0x%x chan=%d CHNLn_RDATA0=0x%x complete, data=0x%x\n",
		addr, chan, rdata_addr, val);

	return (int)val;
}

int spmi_arb_write8(uint32_t addr, uint8_t data)
{
	int chan;
	int ret;
	uint32_t wdata_addr;

	INFO("SPMI_ARB write: addr=0x%x, data=0x%x, resolving channel\n", addr, data);

	chan = addr_to_channel(addr);
	if (chan < 0) {
		ERROR("SPMI_ARB write: no channel found for addr=0x%x\n", addr);
		return chan;
	}

	wdata_addr = HWIO_PMIC_ARB_CHNLn_WDATA0_ADDR(SPMI_ARB_BASE, chan);

	INFO("SPMI_ARB write: addr=0x%x resolved to chan=%d CHNLn_WDATA0=0x%x, writing data=0x%x\n",
		addr, chan, wdata_addr, data);

	mmio_write_32(wdata_addr, data);

	INFO("SPMI_ARB write: chan=%d data written, issuing command\n", chan);

	issue_command((unsigned int)chan, PMIC_ARB_CMD_EXTENDED_REG_WRITE_LONG, addr);

	INFO("SPMI_ARB write: chan=%d command issued, waiting for completion\n", chan);

	ret = wait_for_done((unsigned int)chan);
	if (ret != 0) {
		ERROR("SPMI_ARB write error [0x%x] = 0x%x: %d\n", addr, data, ret);
	} else {
		INFO("SPMI_ARB write: addr=0x%x chan=%d complete\n", addr, chan);
	}

	return ret;
}
