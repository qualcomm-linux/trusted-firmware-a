/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <drivers/qti/chipinfo/chipinfo.h>

#include <drivers/qti/icb/icbcfg.h>
#include "icbcfg_query.h"

static struct icbcfg_device_config	*icb_dev_config;
static bool				 dev_config_valid;

/*
 * If all Qultivate parts in prop->qtv_parts[] are disabled on this SKU the
 * write list is skipped.  chipinfo_is_part_disabled() returns false (assume
 * present) when the ChipInfo driver is not yet initialised.
 */
static void icb_configure_settings(const struct icbcfg_prop *prop)
{
	uint32_t i;

	if (prop == NULL) {
		return;
	}

	if (prop->num_qtv_parts > 0U) {
		bool all_disabled = true;

		for (i = 0U; i < prop->num_qtv_parts; i++) {
			if (!chipinfo_is_part_disabled(
				    prop->qtv_parts[i].part,
				    prop->qtv_parts[i].part_idx)) {
				all_disabled = false;
				break;
			}
		}

		if (all_disabled) {
			return;
		}
	}

	for (i = 0U; i < prop->len; i++) {
		if (prop->data[i].addr == 0U) {
			continue;
		}
		mmio_write_32((uintptr_t)prop->data[i].addr,
			      prop->data[i].val);
	}
}

/* Apply an ordered list of icbcfg_prop segments. */
static void icb_configure_settings_list(
	const struct icbcfg_prop_list *prop_list)
{
	uint32_t i;

	if (prop_list == NULL || prop_list->segs == NULL) {
		return;
	}

	for (i = 0U; i < prop_list->len; i++) {
		if (prop_list->segs[i] == NULL) {
			continue;
		}
		icb_configure_settings(prop_list->segs[i]);
	}
}

static bool get_device_configuration(struct icbcfg_device_config **dev_config)
{
	enum chipinfo_family	family;
	uint32_t		version;
	uint32_t		i;

	if (dev_config_valid) {
		*dev_config = icb_dev_config;
		return true;
	}

	family  = chipinfo_get_chip_family();
	version = chipinfo_get_chip_version();

	for (i = 0U; i < icbcfg_info.num_configs; i++) {
		struct icbcfg_device_config *cfg = icbcfg_info.configs[i];

		if (cfg == NULL) {
			continue;
		}

		if ((uint32_t)family != cfg->family) {
			continue;
		}

		/* Exact match, or non-exact and version >= cfg->version. */
		if (version != cfg->version &&
		    (cfg->match || version < cfg->version)) {
			continue;
		}

		/* No register check, or masked read equals reg_val. */
		if (cfg->reg_addr != NULL &&
		    cfg->reg_val != (mmio_read_32((uintptr_t)cfg->reg_addr) &
				     cfg->reg_mask)) {
			continue;
		}

		icb_dev_config   = cfg;
		dev_config_valid = true;
		break;
	}

	*dev_config = icb_dev_config;
	return dev_config_valid;
}

static void icb_config_init(void)
{
	struct icbcfg_device_config *dev_config;

	if (!get_device_configuration(&dev_config)) {
		return;
	}

	/* prop_data and prop_data_list are mutually exclusive. */
	if (dev_config->prop_data != NULL) {
		icb_configure_settings(dev_config->prop_data);
	} else {
		icb_configure_settings_list(dev_config->prop_data_list);
	}
}

static void icb_config_post_init(void)
{
	struct icbcfg_device_config *dev_config;

	if (!get_device_configuration(&dev_config)) {
		return;
	}

	/* post_prop_data and post_prop_data_list are mutually exclusive. */
	if (dev_config->post_prop_data != NULL) {
		icb_configure_settings(dev_config->post_prop_data);
	} else {
		icb_configure_settings_list(dev_config->post_prop_data_list);
	}
}

void qti_icbcfg_init(void)
{
	icb_config_init();
	INFO("ICB: configuration initialized\n");
}

/* Must be called after all remap/segment operations are complete. */
void qti_icbcfg_post_init(void)
{
	icb_config_post_init();
	INFO("ICB: post-init configuration done\n");
}
