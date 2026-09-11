/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <vmidmt.h>
#include <vmidmt_cfg.h>
#include <vmidmt_hal.h>
#include <vmidmt_internal.h>
#include <xpu_common.h>

#include <qti_interrupt_svc.h>

/* Maximum value for VMIDMT error status bit position.
 * Error status register is 32 bit so position ranges
 * from 0 to 31
 */
#define VMIDMT_ERR_BIT_POS_MAX 31

/* No access permissions to VMIDMT config registers to any non secure entity */
struct hal_vmidmt_access_config acc_ctl = { { 0xFFFFFFFF }, { 0 } };

/*
 * Interrupt context passed to the error handler. The register addresses and
 * interrupt numbers come from the configuration provider, which resolves them
 * either from cfg/${CHIPSET} (XPU3) or from the config image (XPU4).
 */
struct vmidmt_err_ctx {
	enum vmidmt_intr_id id;
	uint32_t reg;
};

static struct vmidmt_err_ctx vmidmt_err[VMIDMT_INTR_COUNT] = {
	[VMIDMT_INTR_CLT_SEC] = { .id = VMIDMT_INTR_CLT_SEC, .reg = 0 },
	[VMIDMT_INTR_CLT_NONSEC] = { .id = VMIDMT_INTR_CLT_NONSEC, .reg = 0 },
	[VMIDMT_INTR_CFG_SEC] = { .id = VMIDMT_INTR_CFG_SEC, .reg = 0 },
	[VMIDMT_INTR_CFG_NONSEC] = { .id = VMIDMT_INTR_CFG_NONSEC, .reg = 0 },
};

static struct hal_vmidmt_info *get_info_cfg(uint8_t port)
{
	struct hal_vmidmt_info *p = NULL;
	uint32_t count = 0U;

	if (vmidmt_cfg_get_info_array(&p, &count) != 0) {
		return NULL;
	}

	for (size_t i = 0; i < count; i++, p++) {
		if ((uint8_t)p->vmidmt != port)
			continue;

		return p;
	}

	return NULL;
}

/*
 * Build the context configuration shared by both mapping paths.
 */
static void map_vmid_ctx_init(struct hal_vmidmt_context_config *ctx_cfg,
			      struct hal_vmidmt_bus_attrib *bus_attribs,
			      uint32_t vmid, uint32_t mem_type, bool secure)
{
	if (mem_type != ACC_VMIDMT_MEMTYPE_DEFAULT) {
		bus_attribs->e_mtcfg = HAL_VMIDMT_MTCFG_MEMATTR;
		bus_attribs->mem_attr = mem_type;
	}

	if (secure)
		bus_attribs->e_nscfg = HAL_VMIDMT_NSCFG_SECURE;

	ctx_cfg->p_bus_attrib = bus_attribs;
	ctx_cfg->u_vmid = (uint8_t)vmid;
}

/*
 * Map a VMID for a single stream on the given instance.
 *
 * stream is the stream ID itself, not a table index: with one SID the stream
 * identifies the context, and SMRn.ID has to match what the master emits. The
 * mapping's index field is unused on this path.
 */
static int map_vmid_internal(enum hal_vmidmt_instance master, uint32_t stream,
			     uint32_t vmid, uint32_t mem_type, bool secure)
{
	struct hal_vmidmt_context_config ctx_cfg = { 0 };
	struct hal_vmidmt_bus_attrib bus_attribs = { 0 };
	struct hal_vmidmt_info *cfg;
	enum hal_vmidmt_status rc;

	map_vmid_ctx_init(&ctx_cfg, &bus_attribs, vmid, mem_type, secure);

	cfg = get_info_cfg(master);
	if (!cfg)
		return -1;

	rc = vmidmt_hal_config_ctx(cfg, stream, &ctx_cfg);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return -1;

	rc = vmidmt_hal_enable_client(cfg, true);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return -1;

	rc = vmidmt_hal_enable_client(cfg, false);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return -1;

	return 0;
}

/*
 * Map a VMID for a group of streams on the given instance.
 *
 * Here index selects the stream mapping register the group occupies, and the
 * SID list supplies the ID and mask that register matches on.
 */
static int map_multisid_vmid_internal(enum hal_vmidmt_instance master,
				      uint32_t index, const uint32_t *sid_list,
				      uint8_t num_sids, uint32_t vmid,
				      uint32_t mem_type, bool secure)
{
	struct hal_vmidmt_context_config ctx_cfg = { 0 };
	struct hal_vmidmt_bus_attrib bus_attribs = { 0 };
	struct hal_vmidmt_info *cfg;
	enum hal_vmidmt_status rc;

	map_vmid_ctx_init(&ctx_cfg, &bus_attribs, vmid, mem_type, secure);

	cfg = get_info_cfg(master);
	if (!cfg)
		return -1;

	rc = vmidmt_hal_config_ctx_ext(cfg, index, sid_list, num_sids,
				       &ctx_cfg);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return -1;

	rc = vmidmt_hal_enable_client(cfg, true);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return -1;

	rc = vmidmt_hal_enable_client(cfg, false);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return -1;

	return 0;
}

/*
 * Apply the VMID mappings belonging to one instance.
 *
 * A mapping is only programmed when its flags say it is configured statically at
 * boot; the rest belong to masters that are power-collapsed now and get
 * configured when they come up. Bit 1 of the same field says whether the SID is
 * to be treated as secure, which decides S2VRn.NSCFG and the SSDRn bit.
 */
static int config_vmids_per_master(const struct vmidmt_cfg *entry)
{
	const struct vmidmt_map *map = entry->vmid_map;
	uint32_t i;
	int ret;

	if (map == NULL)
		return 0;

	for (i = 0U; i < entry->vmid_map_count; i++) {
		bool secure;

		if (!(map[i].flags & ACC_VMIDMT_FLAG_STATIC_CONFIG))
			continue;

		secure = (map[i].flags & ACC_VMIDMT_FLAG_SCFG_FOR_SID) != 0U;

		/*
		 * A single SID identifies its own context, so the SID is
		 * programmed and index is unused. A group of SIDs shares one
		 * stream mapping register, selected by index.
		 */
		if (map[i].num_sids == 1U) {
			ret = map_vmid_internal(
				(enum hal_vmidmt_instance)entry->table,
				map[i].sid_list[0], map[i].vmid,
				map[i].memattr, secure);
		} else {
			ret = map_multisid_vmid_internal(
				(enum hal_vmidmt_instance)entry->table,
				map[i].index, map[i].sid_list,
				map[i].num_sids, map[i].vmid,
				map[i].memattr, secure);
		}

		if (ret) {
			ERROR("VMIDMT: mapping %u failed on instance %u\n", i,
			      entry->table);
			return ret;
		}
	}

	return 0;
}

static int config_options_per_master(const struct vmidmt_cfg *entry)
{
	struct hal_vmidmt_default_vmid_config vmid_cfg = {
		.b_vmid_private_namespace_enable = false,
		.bypass_vmid = entry->secure_bypass_vmid,
		.p_access_control = &acc_ctl,
		.p_bypass_bus_attrib = NULL,
		.p_bypass_aux_config = NULL,
	};
	struct hal_vmidmt_default_secure_vmid_config secure_cfg = {
		.secure_extensions = HAL_VMIDMT_SECURE_EXT_DEFAULT,
		.b_glb_addr_space_restricted_acc_enable = 0,
		.p_default_secure_config = &vmid_cfg,
	};
	struct hal_vmidmt_bus_attrib bus_attribs;
	struct hal_vmidmt_info *cfg;
	enum hal_vmidmt_status rc;
	bool xbl_configured;

	cfg = get_info_cfg(entry->table);
	if (!cfg)
		return -1;

	memset(&bus_attribs, 0, sizeof(bus_attribs));

	if (entry->table == HAL_VMIDMT_DEHR) {
		bus_attribs.e_nscfg = HAL_VMIDMT_NSCFG_SECURE;
		vmid_cfg.p_bypass_bus_attrib = &bus_attribs;
		vmid_cfg.bypass_vmid = ACC_VMID_NOACCESS;
	}

	/*
	 * An instance XBL already configured keeps the VMID, SMR and SSD state
	 * XBL established; only the global options are (re)programmed. Writing
	 * the table defaults would discard that state.
	 */
	xbl_configured = entry->static_cfg == ACC_VMIDMT_STATIC_CONFIG_XBL;

	rc = vmidmt_hal_init(cfg, &secure_cfg, NULL, xbl_configured);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return -1;

	if (entry->ssd_table != NULL) {
		rc = vmidmt_hal_config_ssdt(cfg, entry->ssd_table,
					    entry->ssd_table_num_elements);
		if (rc != HAL_VMIDMT_NO_ERROR) {
			ERROR("VMIDMT: SSDT configuration failed on %u\n",
			      entry->table);
			return -1;
		}
	}

	rc = vmidmt_hal_cfg_err(cfg, false, entry->err_opt);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return -1;

	rc = vmidmt_hal_cfg_err(cfg, true, entry->err_opt);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return -1;

	cfg->is_initialised = true;

	return 0;
}

/*
 * Bring up every statically configured instance.
 *
 * Options and VMID mappings are applied per instance rather than in two global
 * passes, so an instance is fully configured before the next is touched and a
 * mapping can never be programmed against an instance that was never
 * initialised.
 */
static int32_t configure_masters(void)
{
	const struct vmidmt_cfg *p = NULL;
	uint32_t count = 0U;
	uint32_t i;
	int ret;

	ret = vmidmt_cfg_get_cfg_array(&p, &count);
	if (ret) {
		ERROR("VMIDMT: no configuration array available\n");
		return ret;
	}

	for (i = 0U; i < count; i++) {
		if (p[i].static_cfg == ACC_VMIDMT_STATIC_CONFIG_NONE)
			continue;

		ret = config_options_per_master(&p[i]);
		if (ret) {
			ERROR("VMIDMT: options failed on instance %u\n",
			      p[i].table);
			return ret;
		}

		ret = config_vmids_per_master(&p[i]);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Report the latched error for one instance.
 *
 * secure selects which error bank is read and cleared: the secure registers
 * (SGFSR/SGFSRRESTORE) or the non-secure ones. It has to follow the interrupt
 * that fired, otherwise a non-secure fault is never decoded and, because its
 * status is never written back, the interrupt re-asserts forever.
 */
static void log_error(enum hal_vmidmt_instance vmidmt, bool secure)
{
	struct hal_vmidmt_info *cfg = NULL;
	struct hal_vmidmt_error error = { 0 };

	cfg = get_info_cfg(vmidmt);
	if (!cfg) {
		WARN("Unknown Port: 0x%X\n", vmidmt);
		return;
	}

	if (!vmidmt_hal_is_error(cfg, secure)) {
		WARN("VMIDMT ERROR: VMIDMT %d discarded error\n", vmidmt);
		return;
	}

	vmidmt_hal_get_error(cfg, secure, &error);

	ERROR("VMIDMT %d %s error\n", vmidmt, secure ? "secure" : "non-secure");
	ERROR("Error Flags: 0x%X\n", error.u_error_flags);
	ERROR("Bus Flags: 0x%X\n", error.u_bus_flags);
	ERROR("SSD Idx: 0x%X SID: 0x%X\n", error.u_ssd_index, error.u_sid);
	ERROR("MID: 0x%X AVMID: 0x%X\n", error.u_master_id, error.u_avmid);
	ERROR("ATID: 0x%X ABID: 0x%X APID: 0x%X\n", error.u_atid, error.u_abid,
	      error.u_apid);
	ERROR("Phys Addr: 0x%X 0x%X\n", error.u_physical_address_upper32,
	      error.u_physical_address_lower32);

	vmidmt_hal_clear_error(cfg, secure);
}

static void log_errors(uint32_t reg, uint32_t pos, bool secure)
{
	const struct vmidmt_err_pos_to_hal_map *map = NULL;
	const struct vmidmt_err_pos_to_hal_map *row;
	uint32_t reg_count = 0U;
	uint32_t per_reg = 0U;

	if (vmidmt_cfg_get_err_pos_map(&map, &reg_count, &per_reg) != 0)
		return;

	if (reg >= reg_count || per_reg == 0U)
		return;

	row = &map[reg * per_reg];

	for (size_t i = 0; i < per_reg; i++, row++) {
		if (row->bit_pos != pos)
			continue;

		if (row->vmidmt != HAL_VMIDMT_COUNT)
			log_error(row->vmidmt, secure);

		break;
	}
}

static void *error_handler(uint32_t int_num, void *ctx)
{
	struct vmidmt_err_ctx *err_ctx = (struct vmidmt_err_ctx *)ctx;
	const struct vmidmt_intr_reg *reg = NULL;
	bool secure;
	uint32_t err;

	if (vmidmt_cfg_get_intr_reg(err_ctx->id, &reg) != 0)
		return ctx;

	/* Only the two secure interrupts report through the secure bank. */
	secure = (err_ctx->id == VMIDMT_INTR_CLT_SEC) ||
		 (err_ctx->id == VMIDMT_INTR_CFG_SEC);

	err = mmio_read_32(reg->status_addr) & reg->status_mask;

	if (err == 0U)
		return ctx;

	ERROR("VMIDMT INT_REG%d: 0x%08X\n", err_ctx->reg, err);

	for (uint32_t bit = 0U; bit <= VMIDMT_ERR_BIT_POS_MAX; bit++) {
		if (err & BIT(bit))
			log_errors(err_ctx->reg, bit, secure);
	}

	return ctx;
}

/*
 * Interrupt handler for targets that route all four VMIDMT error interrupts to
 * one line. Each status register is polled and, for any that is asserted, the
 * per-interrupt handler runs with that interrupt's context so the secure and
 * non-secure banks are still distinguished correctly.
 */
static void *grouped_error_handler(uint32_t int_num, void *ctx)
{
	bool error_set = false;
	uint32_t i;

	for (i = 0U; i < VMIDMT_INTR_COUNT; i++) {
		const struct vmidmt_intr_reg *reg = NULL;

		if (vmidmt_cfg_get_intr_reg((enum vmidmt_intr_id)i, &reg) != 0)
			continue;

		if ((mmio_read_32(reg->status_addr) & reg->status_mask) == 0U)
			continue;

		error_set = true;
		error_handler(int_num, &vmidmt_err[i]);
	}

	if (!error_set)
		WARN("VMIDMT: interrupt with no error bit set\n");

	return ctx;
}

/* True when every VMIDMT error interrupt is delivered on the same line. */
static bool vmidmt_intr_is_grouped(const struct vmidmt_intr_reg **reg)
{
	uint32_t i;

	for (i = 1U; i < VMIDMT_INTR_COUNT; i++) {
		if (reg[i]->intr_num != reg[0]->intr_num)
			return false;
	}

	return true;
}

/*
 * Unmask every VMIDMT error source.
 *
 * On some targets the status and enable registers share a 4 KB page with ER XPU
 * information, where TZ has read-only access and another environment does the
 * programming. Writing anyway would be dropped, so each register is checked
 * against the XPU policy first.
 */
static void vmidmt_enable_interrupts(const struct vmidmt_intr_reg **reg)
{
	const uint32_t enable = 0xFFFFFFFF;
	uint32_t i;

	for (i = 0U; i < VMIDMT_INTR_COUNT; i++) {
		bool can_write = false;

		if (xpu_can_write(reg[i]->enable_addr, &can_write) != 0) {
			WARN("VMIDMT: cannot query write access for 0x%x\n",
			     reg[i]->enable_addr);
			continue;
		}

		if (!can_write) {
			INFO("VMIDMT: 0x%x not writable here, left to owner\n",
			     reg[i]->enable_addr);
			continue;
		}

		mmio_write_32(reg[i]->enable_addr, enable);
	}
}

static int32_t register_interrupts(void)
{
	const struct vmidmt_intr_reg *reg[VMIDMT_INTR_COUNT] = { NULL };
	uint32_t registered = 0U;
	int ret;
	uint32_t i;

	for (i = 0U; i < VMIDMT_INTR_COUNT; i++) {
		ret = vmidmt_cfg_get_intr_reg((enum vmidmt_intr_id)i, &reg[i]);
		if (ret) {
			ERROR("VMIDMT: interrupt %u unavailable\n", i);
			return -1;
		}
	}

	if (vmidmt_intr_is_grouped(reg)) {
		/*
		 * One line for all four sources: registering four handlers on
		 * the same number would collide, so install a single handler
		 * that decodes which source fired.
		 */
		ret = qti_interrupt_svc_register(reg[0]->intr_num,
						 grouped_error_handler,
						 &vmidmt_err[0]);
		if (ret)
			return -1;
	} else {
		for (i = 0U; i < VMIDMT_INTR_COUNT; i++) {
			ret = qti_interrupt_svc_register(reg[i]->intr_num,
							 error_handler,
							 &vmidmt_err[i]);
			if (ret)
				goto unwind;

			registered++;
		}
	}

	/* Unmask only once every handler is in place. */
	vmidmt_enable_interrupts(reg);

	return 0;

unwind:
	while (registered-- > 0U)
		qti_interrupt_svc_unregister(reg[registered]->intr_num);

	return -1;
}

int vmidmt_configure(void)
{
	int rc;

	rc = configure_masters();
	if (rc)
		return ACC_ERR_VMIDMT_CFG_FAIL;

	rc = register_interrupts();
	if (rc)
		return ACC_ERR_VMIDMT_CFG_FAIL;

	return 0;
}
