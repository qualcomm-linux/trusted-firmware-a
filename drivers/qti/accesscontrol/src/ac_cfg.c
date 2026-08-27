/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ac_cfg.h>
#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

/*
 * Base address of the access-control config image. The image is placed in DDR
 * by an earlier boot stage; BL31 is handed only this address and derives every
 * table from the contents.
 *
 * TODO: this is a provisional fixed address. Replace with the real handoff
 * mechanism (BL31 parameters or a reserved-memory node) once known.
 */
#define AC_CFG_IMAGE_BASE	UL(0xBC000000)

/* Validated image header, NULL until ac_cfg_init() succeeds. */
static const struct ac_global_data *ac_cfg;

/* Extent of the mapped image, used to bounds check every entry. */
static uintptr_t ac_cfg_base;
static size_t ac_cfg_size;

/*
 * Return true if [ptr, ptr + len) lies wholly within the config image.
 *
 * The image is produced by an earlier boot stage and its entries are absolute
 * pointers, so every one has to be checked before it is dereferenced: the
 * tables it describes are fed straight into XPU protection hardware. The
 * arithmetic is written to avoid wrapping on a hostile length.
 */
static bool ac_cfg_in_image(uintptr_t ptr, size_t len)
{
	uintptr_t end = ac_cfg_base + ac_cfg_size;

	if (ptr < ac_cfg_base || ptr >= end) {
		return false;
	}

	if (len > (size_t)(end - ptr)) {
		return false;
	}

	return true;
}

/*
 * Fetch one raw entry from the image. Entries are absolute pointers indexed by
 * tag; a zero entry means the producer did not populate it.
 */
static int ac_cfg_entry(enum ac_xpu_target_cfg tag, uintptr_t *value)
{
	uintptr_t entry;

	if (ac_cfg == NULL) {
		return -EPERM;
	}

	if (tag >= AC_XPU_CFG_MAX || (uint32_t)tag >= ac_cfg->num_entries) {
		return -ERANGE;
	}

	entry = ac_cfg->globaldata_ptr[tag];
	if (entry == 0U) {
		return -ENOENT;
	}

	*value = entry;

	return 0;
}

/* Read a 32-bit scalar that a tag points at. */
static int ac_cfg_scalar32(enum ac_xpu_target_cfg tag, uint32_t *out)
{
	uintptr_t entry;
	int rc;

	rc = ac_cfg_entry(tag, &entry);
	if (rc != 0) {
		return rc;
	}

	if (!ac_cfg_in_image(entry, sizeof(uint32_t))) {
		ERROR("ac_cfg: tag %u scalar out of image\n", (uint32_t)tag);
		return -EINVAL;
	}

	*out = *(const uint32_t *)entry;

	return 0;
}

/* Read a 64-bit scalar that a tag points at. */
static int ac_cfg_scalar64(enum ac_xpu_target_cfg tag, uint64_t *out)
{
	uintptr_t entry;
	int rc;

	rc = ac_cfg_entry(tag, &entry);
	if (rc != 0) {
		return rc;
	}

	if (!ac_cfg_in_image(entry, sizeof(uint64_t))) {
		ERROR("ac_cfg: tag %u scalar out of image\n", (uint32_t)tag);
		return -EINVAL;
	}

	*out = *(const uint64_t *)entry;

	return 0;
}

/* Read a boolean that a tag points at. */
static int ac_cfg_flag(enum ac_xpu_target_cfg tag, bool *out)
{
	uintptr_t entry;
	int rc;

	rc = ac_cfg_entry(tag, &entry);
	if (rc != 0) {
		return rc;
	}

	if (!ac_cfg_in_image(entry, sizeof(bool))) {
		ERROR("ac_cfg: tag %u flag out of image\n", (uint32_t)tag);
		return -EINVAL;
	}

	*out = *(const bool *)entry;

	return 0;
}

/*
 * Resolve a table described by a pointer tag and a separate count tag, and
 * verify that the whole array body lies inside the image.
 *
 * This is the single place table bounds are enforced, so every typed getter
 * below is a thin wrapper over it.
 */
static int ac_cfg_table(enum ac_xpu_target_cfg ptr_tag,
			enum ac_xpu_target_cfg count_tag, size_t elem_size,
			const void **data, uint32_t *count)
{
	uintptr_t entry;
	uint32_t entries;
	int rc;

	if (data == NULL || count == NULL || elem_size == 0U) {
		return -EINVAL;
	}

	rc = ac_cfg_entry(ptr_tag, &entry);
	if (rc != 0) {
		return rc;
	}

	rc = ac_cfg_scalar32(count_tag, &entries);
	if (rc != 0) {
		return rc;
	}

	/* Reject a count whose byte length cannot be represented. */
	if (entries > (uint32_t)(SIZE_MAX / elem_size)) {
		ERROR("ac_cfg: tag %u count %u overflows\n",
		      (uint32_t)ptr_tag, entries);
		return -EINVAL;
	}

	if (!ac_cfg_in_image(entry, (size_t)entries * elem_size)) {
		ERROR("ac_cfg: tag %u table [%lx +%u] out of image\n",
		      (uint32_t)ptr_tag, (unsigned long)entry, entries);
		return -EINVAL;
	}

	*data = (const void *)entry;
	*count = entries;

	return 0;
}

int plat_qti_ac_get_config_image(uintptr_t *base, size_t *size)
{
	if (base == NULL || size == NULL) {
		return -EINVAL;
	}

	*base = AC_CFG_IMAGE_BASE;
	*size = AC_TZ_AC_CONFIG_IMAGE_SIZE;

	return 0;
}

int ac_cfg_init(void)
{
	const struct ac_global_data *hdr;
	uintptr_t base;
	size_t size;
	int rc;

	if (ac_cfg != NULL) {
		return 0;
	}

	rc = plat_qti_ac_get_config_image(&base, &size);
	if (rc != 0) {
		ERROR("ac_cfg: config image not available (%d)\n", rc);
		return rc;
	}

	if (size < sizeof(struct ac_global_data)) {
		ERROR("ac_cfg: image too small (%zu)\n", size);
		return -EINVAL;
	}

	/*
	 * Map read-write.
	 *
	 * The image is not purely policy data: it also holds the runtime caches
	 * the drivers write back into. xpu4_hal_init_xpu() stores the probed REV
	 * and IDR values into struct ac_xpu4_priv_info, the shadow permission
	 * arrays reached through ac_xpu_dynamic_rgs are updated on every dynamic
	 * assignment, and VMIDMT caches its probed device parameters into the
	 * port map. Those all live in image memory, so a read-only mapping
	 * faults on the first instance that is brought up.
	 *
	 * The getters mirror the split: the tables that are pure policy are
	 * returned const, and only the probed-state tables are not.
	 */
	rc = mmap_add_dynamic_region(base, base, size,
				     MT_MEMORY | MT_RW | MT_SECURE);
	if (rc != 0) {
		ERROR("ac_cfg: cannot map image at 0x%lx (%d)\n",
		      (unsigned long)base, rc);
		return rc;
	}

	ac_cfg_base = base;
	ac_cfg_size = size;

	hdr = (const struct ac_global_data *)base;

	if (hdr->magic_cookie != AC_TZ_MAGIC_COOKIE) {
		ERROR("ac_cfg: bad magic 0x%x\n", hdr->magic_cookie);
		goto fail;
	}

	if (hdr->num_entries == 0U) {
		ERROR("ac_cfg: bad entry count %u\n", hdr->num_entries);
		goto fail;
	}

	/* The entry array itself has to be inside the image. */
	if (!ac_cfg_in_image((uintptr_t)hdr->globaldata_ptr,
			     (size_t)hdr->num_entries * sizeof(uintptr_t))) {
		ERROR("ac_cfg: entry array out of image\n");
		goto fail;
	}

	ac_cfg = hdr;

	return 0;

fail:
	ac_cfg_base = 0U;
	ac_cfg_size = 0U;

	return -EINVAL;
}

int ac_cfg_get_xpu_enabled(bool *enabled)
{
	return ac_cfg_flag(AC_IS_XPU_ENABLED, enabled);
}

int ac_cfg_get_xpu_wa_supported(bool *supported)
{
	return ac_cfg_flag(AC_XPU_WA_SUPPORTED, supported);
}

int ac_cfg_get_silent_logging_enabled(bool *enabled)
{
	return ac_cfg_flag(AC_IS_SILENTLOGGING_ENABLED, enabled);
}

int ac_cfg_get_sle_addr(uint32_t *addr)
{
	return ac_cfg_scalar32(AC_XPUTCSRSLE_ADDR, addr);
}

int ac_cfg_get_sle_enable_value(uint32_t *value)
{
	return ac_cfg_scalar32(AC_XPU_SLE_ENABLE_VALUE, value);
}

int ac_cfg_get_sle_disable_value(uint32_t *value)
{
	return ac_cfg_scalar32(AC_XPU_SLE_DISABLE_VALUE, value);
}

int ac_cfg_get_global_xpu_cfg(const struct ac_xpu_cfg **cfg, uint32_t *count)
{
	return ac_cfg_table(AC_XPUCFG_ARRAY, AC_XPUCFG_ARRAY_SIZE,
			    sizeof(struct ac_xpu_cfg), (const void **)cfg,
			    count);
}

int ac_cfg_get_xpu_infos(struct ac_xpu4_priv_info **infos, uint32_t *count)
{
	return ac_cfg_table(AC_XPUINFOS_ARRAY, AC_XPUINFOS_ARRAY_SIZE,
			    sizeof(struct ac_xpu4_priv_info),
			    (const void **)infos, count);
}

int ac_cfg_get_addr_offset_mask_info(const struct ac_xpu_hwaddr_mask **mask,
				     uint32_t *count)
{
	return ac_cfg_table(AC_XPUHWADDR_MASK_ARRAY,
			    AC_XPUHWADDR_MASK_ARRAY_SIZE,
			    sizeof(struct ac_xpu_hwaddr_mask),
			    (const void **)mask, count);
}

int ac_cfg_get_dynamic_protect_ranges(
			const struct ac_xpu_protected_range **range,
			uint32_t *count)
{
	return ac_cfg_table(AC_XPUPROTECTEDRANGES_ARRAY,
			    AC_XPUPROTECTEDRANGES_ARRAY_SIZE,
			    sizeof(struct ac_xpu_protected_range),
			    (const void **)range, count);
}

int ac_cfg_get_apu_rpu_partitions_info(uintptr_t **group, uint32_t *count)
{
	return ac_cfg_table(AC_APURPU_PARTITIONS_ARRAY,
			    AC_APURPU_PARTITIONS_ARRAY_SIZE,
			    sizeof(uintptr_t), (const void **)group, count);
}

int ac_cfg_get_lock_delegate_masks(const struct ac_xpu_id_to_qad_mask **mask,
				   uint32_t *count)
{
	return ac_cfg_table(AC_LOCK_DELEGATE_REDUCED_MASKS,
			    AC_LOCK_DELEGATE_REDUCED_MASKS_COUNT,
			    sizeof(struct ac_xpu_id_to_qad_mask),
			    (const void **)mask, count);
}

int ac_cfg_get_ra_xpu_info(const struct ac_ra_xpu_info **info, uint32_t *count)
{
	return ac_cfg_table(AC_RAXPU_INFO_ARRAY, AC_RAXPU_INFO_ARRAY_SIZE,
			    sizeof(struct ac_ra_xpu_info),
			    (const void **)info, count);
}

int ac_cfg_get_ra_xpu_interrupt_details(
			const struct ac_xpu_intr_reg **status_reg,
			const struct ac_xpu_intr_reg **enable_reg,
			const enum xpu4_id (**pos_to_xpu_map)[32],
			uint32_t *count)
{
	uintptr_t entry;
	uint32_t regs;
	int rc;

	if (status_reg == NULL || enable_reg == NULL ||
	    pos_to_xpu_map == NULL || count == NULL) {
		return -EINVAL;
	}

	/*
	 * All three tables are sized by the same count entry, so resolve the
	 * count once and then validate each table against it.
	 */
	rc = ac_cfg_scalar32(AC_XPUINTR_TCSRREG_ARRAY_SIZE, &regs);
	if (rc != 0) {
		return rc;
	}

	rc = ac_cfg_entry(AC_XPUINTRSTATUS_REG_ARRAY, &entry);
	if (rc != 0) {
		return rc;
	}

	if (!ac_cfg_in_image(entry,
			     (size_t)regs * sizeof(struct ac_xpu_intr_reg))) {
		return -EINVAL;
	}

	*status_reg = (const struct ac_xpu_intr_reg *)entry;

	rc = ac_cfg_entry(AC_XPUINTREN_REG_ARRAY, &entry);
	if (rc != 0) {
		return rc;
	}

	if (!ac_cfg_in_image(entry,
			     (size_t)regs * sizeof(struct ac_xpu_intr_reg))) {
		return -EINVAL;
	}

	*enable_reg = (const struct ac_xpu_intr_reg *)entry;

	rc = ac_cfg_entry(AC_XPUINTR_POS_XPUMAP_ARRAY, &entry);
	if (rc != 0) {
		return rc;
	}

	if (!ac_cfg_in_image(entry, (size_t)regs * 32U *
				    sizeof(enum xpu4_id))) {
		return -EINVAL;
	}

	*pos_to_xpu_map = (const enum xpu4_id (*)[32])entry;
	*count = regs;

	return 0;
}

int ac_cfg_get_xpu_vmidmt_intr_info(struct ac_interrupt_desc **desc,
				    uint32_t *count)
{
	return ac_cfg_table(AC_INTR_DESC_ARRAY, AC_INTR_DESC_ARRAY_SIZE,
			    sizeof(struct ac_interrupt_desc),
			    (const void **)desc, count);
}

int ac_cfg_get_device_memory_map_info(struct ac_device_memory_info **map,
				      uint32_t *count)
{
	return ac_cfg_table(AC_TZ_DEVICE_MEM_MAP_ARRAY,
			    AC_TZ_DEVICE_MEM_MAP_ARRAY_SIZE,
			    sizeof(struct ac_device_memory_info),
			    (const void **)map, count);
}

int ac_cfg_get_shadow_vm_list_info(struct ac_shadow_vm **list, uint32_t *count)
{
	return ac_cfg_table(AC_TZ_SHADOW_VM_ARRAY, AC_TZ_SHADOW_VM_ARRAY_SIZE,
			    sizeof(struct ac_shadow_vm),
			    (const void **)list, count);
}

int ac_cfg_get_ddr_allocation_offset(uint64_t *offset)
{
	return ac_cfg_scalar64(ACDB_ALLOC_FROM_DDR_OFFSET, offset);
}

int ac_cfg_get_fixed_acdb_base_addr(uint64_t *addr)
{
	return ac_cfg_scalar64(ACDB_FIXED_DDR_BASE_ADDR, addr);
}

int ac_cfg_get_xpu_dbgar_info(const uint32_t **dbgars, uint32_t *count)
{
	return ac_cfg_table(AC_XPUDBGAR_ARRAY, AC_XPUDBGAR_ARRAY_SIZE,
			    sizeof(uint32_t), (const void **)dbgars, count);
}

int ac_cfg_get_vmidmt_intr_regs(const struct vmidmt_intr_addr **regs,
				uint32_t *count)
{
	return ac_cfg_table(AC_VMIDMT_INTR_REGS_ARRAY,
			    AC_VMIDMT_INTR_REGS_ARRAY_SIZE,
			    sizeof(struct vmidmt_intr_addr),
			    (const void **)regs, count);
}

int ac_cfg_get_vmidmt_table(enum ac_vmidmt_table which, size_t elem_size,
			    const void **data, uint32_t *count)
{
	enum ac_xpu_target_cfg ptr_tag;
	enum ac_xpu_target_cfg count_tag;

	switch (which) {
	case AC_VMIDMT_TABLE_CFG:
		ptr_tag = AC_VMIDMT_CFG_ARRAY_V2;
		count_tag = AC_VMIDMT_CFG_ARRAY_SIZE_V2;
		break;

	case AC_VMIDMT_TABLE_INFO:
		ptr_tag = AC_VMIDMT_INFO_ARRAY;
		count_tag = AC_VMIDMT_INFO_ARRAY_SIZE;
		break;

	case AC_VMIDMT_TABLE_MAP:
		ptr_tag = AC_VMIDMT_MAP_ARRAY;
		count_tag = AC_VMIDMT_MAP_ARRAY_SIZE;
		break;

	case AC_VMIDMT_TABLE_INTR_POS_MAP:
		ptr_tag = AC_VMIDMT_INTR_POS_MAP_ARRAY;
		count_tag = AC_VMIDMT_INTR_POS_MAP_ARRAY_SIZE;
		break;

	default:
		return -EINVAL;
	}

	return ac_cfg_table(ptr_tag, count_tag, elem_size, data, count);
}
