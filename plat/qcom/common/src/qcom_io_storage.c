/*
 * Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <tools_share/firmware_image_package.h>

#include <platform_def.h>
#include <qcom_plat.h>

static const io_dev_connector_t *qcom_fip_dev_con;
static uintptr_t qcom_fip_dev_handle;

static const io_dev_connector_t *qcom_backend_dev_con;
static uintptr_t qcom_backend_dev_handle;

static io_block_spec_t qcom_fip_spec = {
	.offset = PLAT_QCOM_FIP_IOBASE,
	.length = PLAT_QCOM_FIP_MAXSIZE,
};

static const io_uuid_spec_t qcom_bl31_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t qcom_bl32_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32,
};

static const io_uuid_spec_t qcom_bl33_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

struct qcom_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	uintptr_t init_params;
};

static const struct qcom_io_policy qcom_io_policies[] = {
	[FIP_IMAGE_ID] = {
		.dev_handle = &qcom_backend_dev_handle,
		.image_spec = (uintptr_t)&qcom_fip_spec,
	},
	[BL31_IMAGE_ID] = {
		.dev_handle = &qcom_fip_dev_handle,
		.image_spec = (uintptr_t)&qcom_bl31_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL32_IMAGE_ID] = {
		.dev_handle = &qcom_fip_dev_handle,
		.image_spec = (uintptr_t)&qcom_bl32_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL33_IMAGE_ID] = {
		.dev_handle = &qcom_fip_dev_handle,
		.image_spec = (uintptr_t)&qcom_bl33_spec,
		.init_params = FIP_IMAGE_ID,
	},
};

static int qcom_io_memmap_setup(void)
{
	int ret;

	ret = mmap_add_dynamic_region(qcom_fip_spec.offset, qcom_fip_spec.offset,
				      qcom_fip_spec.length, MT_RO_DATA | MT_SECURE);
	if (ret) {
		return ret;
	}

	ret = register_io_dev_memmap(&qcom_backend_dev_con);
	if (ret) {
		return ret;
	}

	return io_dev_open(qcom_backend_dev_con, 0, &qcom_backend_dev_handle);
}

static int qcom_io_fip_setup(void)
{
	int ret;

	ret = register_io_dev_fip(&qcom_fip_dev_con);
	if (ret) {
		return ret;
	}

	return io_dev_open(qcom_fip_dev_con, 0, &qcom_fip_dev_handle);
}

int qcom_io_setup(void)
{
	int ret;

	ret = qcom_io_memmap_setup();
	if (ret) {
		return ret;
	}

	ret = qcom_io_fip_setup();
	if (ret) {
		return ret;
	}

	return 0;
}

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	uintptr_t init_params;

	assert(image_id < ARRAY_SIZE(qcom_io_policies));

	*dev_handle = *qcom_io_policies[image_id].dev_handle;
	*image_spec = qcom_io_policies[image_id].image_spec;
	init_params = qcom_io_policies[image_id].init_params;

	return io_dev_init(*dev_handle, init_params);
}
