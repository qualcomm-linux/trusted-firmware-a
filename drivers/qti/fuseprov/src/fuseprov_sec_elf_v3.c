/*
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <common/debug.h>
#include <drivers/qti/crypto/rng.h>
#include <drivers/qti/fuseprov/fuseprov_mrc_cfg.h>
#include <drivers/qti/fuseprov/fuseprov_port.h>
#include <drivers/qti/fuseprov/fuseprov_sec_elf_v3.h>

/* Validate SEC.DAT header and extract the flat fuse-entry array
 *
 * SEC.DAT v3 has no segments, footer or hash: a fixed header is immediately
 * followed by hdr->num_entries fuseprov_qfuse_entry_t records.
 */
static fuseprov_error_etype fuseprov_parse_secdat_hdr(
	const uint8_t *buffer, size_t buffer_len,
	fuseprov_secdat_hdr_t *hdr,
	const fuseprov_qfuse_entry_t **entries,
	uint32_t *entry_count)
{
	if (buffer == NULL || hdr == NULL || entries == NULL ||
	    entry_count == NULL) {
		return FUSEPROV_INVALID_ARG;
	}

	if (buffer_len < sizeof(fuseprov_secdat_hdr_t)) {
		return FUSEPROV_SECDAT_SIZE_LEN_MISMATCH;
	}

	memcpy(hdr, buffer, sizeof(fuseprov_secdat_hdr_t));

#if defined(QTI_FUSEPROV_TEST)
	INFO("Fuseprov: SEC.DAT raw bytes[0:4]=%02x %02x %02x %02x\n",
	     buffer[0], buffer[1], buffer[2], buffer[3]);

	INFO("Fuseprov: SEC.DAT magic1=0x%x magic2=0x%x\n",
	     hdr->magic1, hdr->magic2);
#endif /* QTI_FUSEPROV_TEST */

	if (hdr->magic1 != FUSEPROV_SECDAT_MAGIC1 ||
	    hdr->magic2 != FUSEPROV_SECDAT_MAGIC2) {
		ERROR("Fuseprov: SEC.DAT magic mismatch\n");
		return FUSEPROV_SECDAT_MAGIC_MISMATCH;
	}

	if (hdr->revision != FUSEPROV_SECDAT_V3_REV) {
		ERROR("Fuseprov: SEC.DAT revision %u not supported\n",
		      hdr->revision);
		return FUSEPROV_SECDAT_REV_NOT_SUPPORTED;
	}

	if ((buffer_len - sizeof(fuseprov_secdat_hdr_t)) /
	    sizeof(fuseprov_qfuse_entry_t) < hdr->num_entries) {
		ERROR("Fuseprov: Buffer too small for %u fuse entries\n",
		      hdr->num_entries);
		return FUSEPROV_SECDAT_SIZE_LEN_MISMATCH;
	}

	*entries = (const fuseprov_qfuse_entry_t *)(buffer +
						     sizeof(fuseprov_secdat_hdr_t));
	*entry_count = hdr->num_entries;

	return FUSEPROV_SUCCESS;
}

/* Blow fuses in a specific category via transport abstraction */
/* Map a SEC.DAT region type to its blow category
 *
 * Regions without an explicit category fall back to GENERAL so that region
 * types such as OEM_PK_HASH, ANTI_ROLLBACK, IMAGE_ENCR_KEY and MRC_2_0 are
 * still blown, as part of the GENERAL pass.
 */
fuseprov_category_t fuseprov_get_category_for_region(uint32_t region_type)
{
	switch (region_type) {
	case FUSEPROV_REGION_TYPE_OEM_SEC_BOOT:
		return FUSEPROV_CATEGORY_SECBOOT;
	case FUSEPROV_REGION_TYPE_SEC_HW_KEY:
		return FUSEPROV_CATEGORY_SHK;
	case FUSEPROV_REGION_TYPE_OEM_CONFIG:
		return FUSEPROV_CATEGORY_OEM_CONFIG;
	case FUSEPROV_REGION_TYPE_READ_PERM:
		return FUSEPROV_CATEGORY_READ_PERM;
	case FUSEPROV_REGION_TYPE_WRITE_PERM:
		return FUSEPROV_CATEGORY_WRITE_PERM;
	case FUSEPROV_REGION_TYPE_FEC_EN:
		return FUSEPROV_CATEGORY_FEC_EN;
	case FUSEPROV_REGION_TYPE_OEM_PRODUCT_SEED:
		return FUSEPROV_CATEGORY_OEM_PRODUCT_SEED;
	/*
	 * OEM_SPARE keeps its own category rather than falling under GENERAL so
	 * that random-value blowing is handled separately.
	 */
	case FUSEPROV_REGION_TYPE_OEM_SPARE:
		return FUSEPROV_CATEGORY_OEM_SPARE_RAND;
	default:
		return FUSEPROV_CATEGORY_GENERAL;
	}
}

bool fuseprov_is_region_in_category(fuseprov_category_t category,
				    uint32_t region_type)
{
	return category == fuseprov_get_category_for_region(region_type);
}

static fuseprov_error_etype fuseprov_blow_fuseregion(
	const fuseprov_transport_t *t,
	const fuseprov_qfuse_entry_t *entries,
	uint32_t entry_count,
	fuseprov_category_t category)
{
	uint32_t i;
	uint32_t fuse_data[2];
	uint32_t mask_fec_msb_bits;
	fuseprov_err_t ret;
	uint64_t data;

	for (i = 0; i < entry_count; i++) {
		if (!fuseprov_is_region_in_category(category,
						    entries[i].region_type)) {
			continue;
		}

		if (entries[i].operation != FUSEPROV_OPERATION_BLOW) {
			continue;
		}

		if (entries[i].lsb_val == 0 && entries[i].msb_val == 0) {
			continue;
		}

		/* Read current fuse value */
		ret = fuseprov_row_read(t, entries[i].raw_row_address,
				       FUSEPROV_ADDR_CORR, fuse_data);
		if (ret != FUSEPROV_OK) {
			ERROR("Fuseprov: Failed to read fuse at 0x%x\n",
			      entries[i].raw_row_address);
			return FUSEPROV_QFPROM_READ_ERROR;
		}

		/*
		 * FEC rows have hardware-computed correction bits in the top
		 * byte of the MSB word; exclude that byte when checking
		 * whether the target bits are already blown.
		 */
		mask_fec_msb_bits = (entries[i].region_type ==
				     FUSEPROV_REGION_TYPE_FEC_EN) ?
				    ~FUSEPROV_FEC_ROW_MSB_MASK :
				    0xFFFFFFFF;

		/* Blow only if we need to -- skip if already blown */
		if ((fuse_data[0] & entries[i].lsb_val) ==
		    entries[i].lsb_val &&
		    (fuse_data[1] & entries[i].msb_val & mask_fec_msb_bits) ==
		    (entries[i].msb_val & mask_fec_msb_bits)) {
			continue;
		}

		/* Write new fuse value */
		data = ((uint64_t)entries[i].msb_val << 32) |
		       entries[i].lsb_val;

		ret = fuseprov_rows_write(t, &entries[i].raw_row_address,
				&data, 1, NULL);
		if (ret != FUSEPROV_OK) {
			ERROR("Fuseprov: Failed to write fuse at 0x%x\n",
			      entries[i].raw_row_address);
			return FUSEPROV_QFPROM_WRITE_ERROR;
		}

		NOTICE("Fuseprov: Blew fuse at 0x%x (LSB=0x%x, MSB=0x%x)\n",
		       entries[i].raw_row_address, entries[i].lsb_val,
		       entries[i].msb_val);
	}

	return FUSEPROV_SUCCESS;
}

/* Provision SHK (Secondary Hardware Key) with random data */
static fuseprov_error_etype fuseprov_provision_shk(
	const fuseprov_transport_t *t,
	const fuseprov_qfuse_entry_t *entries,
	uint32_t entry_count)
{
	uint8_t random_data[40];
	uint32_t i;
	int ret;

	/* Generate random data for SHK (5 fuse rows * 8 bytes) */
	ret = qti_rng_get_data(random_data, sizeof(random_data));
	if (ret != 0) {
		ERROR("Fuseprov: Failed to generate random data for SHK\n");
		return FUSEPROV_SHK_GENERATION_FAILED;
	}

	/* Blow SHK fuses with random data */
	for (i = 0; i < entry_count; i++) {
		if (entries[i].region_type == FUSEPROV_REGION_TYPE_SEC_HW_KEY &&
		    entries[i].operation == FUSEPROV_OPERATION_BLOW_RANDOM) {
			uint64_t data;
			fuseprov_err_t fret;

			memcpy(&data, &random_data[i * 8], 8);

			fret = fuseprov_rows_write(t,
					&entries[i].raw_row_address,
					&data, 1, NULL);
			if (fret != FUSEPROV_OK) {
				ERROR("Fuseprov: Failed to write SHK fuse\n");
				return FUSEPROV_QFPROM_WRITE_ERROR;
			}
		}
	}

	return FUSEPROV_SUCCESS;
}

/* Provision OEM product seed with random data */
static fuseprov_error_etype fuseprov_provision_oem_product_seed(
	const fuseprov_transport_t *t,
	const fuseprov_qfuse_entry_t *entries,
	uint32_t entry_count)
{
	uint8_t random_data[40];
	uint32_t i;
	int ret;

	ret = qti_rng_get_data(random_data, sizeof(random_data));
	if (ret != 0) {
		ERROR("Fuseprov: Failed to generate random data for OEM product seed\n");
		return FUSEPROV_SHK_GENERATION_FAILED;
	}

	for (i = 0; i < entry_count; i++) {
		if (entries[i].region_type == FUSEPROV_REGION_TYPE_OEM_PRODUCT_SEED &&
		    entries[i].operation == FUSEPROV_OPERATION_BLOW_RANDOM) {
			uint64_t data;
			fuseprov_err_t fret;

			memcpy(&data, &random_data[i * 8], 8);

			fret = fuseprov_rows_write(t,
					&entries[i].raw_row_address,
					&data, 1, NULL);
			if (fret != FUSEPROV_OK) {
				ERROR("Fuseprov: Failed to write OEM product seed fuse\n");
				return FUSEPROV_QFPROM_WRITE_ERROR;
			}
		}
	}

	return FUSEPROV_SUCCESS;
}

/* Provision OEM spare data with random values */
static fuseprov_error_etype fuseprov_provision_oem_spare(
	const fuseprov_transport_t *t,
	const fuseprov_qfuse_entry_t *entries,
	uint32_t entry_count)
{
	uint8_t random_data[40];
	uint32_t i;
	int ret;

	ret = qti_rng_get_data(random_data, sizeof(random_data));
	if (ret != 0) {
		ERROR("Fuseprov: Failed to generate random data for OEM spare\n");
		return FUSEPROV_OEM_SPARE_RAND_GEN_FAILED;
	}

	for (i = 0; i < entry_count; i++) {
		if (entries[i].region_type == FUSEPROV_REGION_TYPE_OEM_SPARE &&
		    entries[i].operation == FUSEPROV_OPERATION_BLOW_RANDOM) {
			uint64_t data;
			fuseprov_err_t fret;

			memcpy(&data, &random_data[i * 8], 8);

			fret = fuseprov_rows_write(t,
					&entries[i].raw_row_address,
					&data, 1, NULL);
			if (fret != FUSEPROV_OK) {
				ERROR("Fuseprov: Failed to write OEM spare fuse\n");
				return FUSEPROV_QFPROM_WRITE_ERROR;
			}
		}
	}

	return FUSEPROV_SUCCESS;
}

/* Main entry point: parse and blow fuses from SEC.DAT v3
 * This is the portable layer entry point that takes a transport contract
 */
fuseprov_error_etype fuseprov_blow_fuses_sec_elf_v3(
	const fuseprov_transport_t *t,
	uint8_t *buf,
	uint32_t len)
{
	fuseprov_secdat_hdr_t hdr;
	const fuseprov_qfuse_entry_t *entries;
	uint32_t entry_count;
	fuseprov_error_etype ret;
	uint32_t i;

	if (buf == NULL || len == 0 || len > FUSEPROV_SECDAT_BUFFER_SIZE ||
	    t == NULL) {
		return FUSEPROV_INVALID_ARG;
	}

	/* Not an error: no sec partition, or an empty one, is valid */
	for (i = 0; i < len && buf[i] == 0; i++);

	if (i == len) {
		// NOTICE("Fuseprov: SEC.DAT buffer is empty, nothing to blow\n");
		return  FUSEPROV_SUCCESS ;
	}

	NOTICE("Fuseprov: Starting fuse provisioning\n");

	/* Parse SEC.DAT header and get the flat fuse-entry array */
	ret = fuseprov_parse_secdat_hdr(buf, len, &hdr, &entries, &entry_count);
	if (ret != FUSEPROV_SUCCESS) {
		ERROR("Fuseprov: Failed to parse SEC.DAT header\n");
		return ret;
	}

	/* Blow fuses in order: GENERAL -> SHK -> OEM_PRODUCT_SEED ->
	 * OEM_SPARE -> OEM_CONFIG -> SECBOOT -> FEC_EN -> READ_PERM ->
	 * WRITE_PERM (last, locks everything)
	 */
	ret = fuseprov_blow_fuseregion(t, entries, entry_count,
				      FUSEPROV_CATEGORY_GENERAL);
	if (ret != FUSEPROV_SUCCESS) {
		return ret;
	}

	ret = fuseprov_provision_shk(t, entries, entry_count);
	if (ret != FUSEPROV_SUCCESS) {
		return ret;
	}

	ret = fuseprov_provision_oem_product_seed(t, entries, entry_count);
	if (ret != FUSEPROV_SUCCESS) {
		return ret;
	}

	ret = fuseprov_provision_oem_spare(t, entries, entry_count);
	if (ret != FUSEPROV_SUCCESS) {
		return ret;
	}

	ret = fuseprov_blow_fuseregion(t, entries, entry_count,
				      FUSEPROV_CATEGORY_OEM_CONFIG);
	if (ret != FUSEPROV_SUCCESS) {
		return ret;
	}

	ret = fuseprov_blow_fuseregion(t, entries, entry_count,
				      FUSEPROV_CATEGORY_SECBOOT);
	if (ret != FUSEPROV_SUCCESS) {
		return ret;
	}

	ret = fuseprov_blow_fuseregion(t, entries, entry_count,
				      FUSEPROV_CATEGORY_FEC_EN);
	if (ret != FUSEPROV_SUCCESS) {
		return ret;
	}

	ret = fuseprov_blow_fuseregion(t, entries, entry_count,
				      FUSEPROV_CATEGORY_READ_PERM);
	if (ret != FUSEPROV_SUCCESS) {
		return ret;
	}

	ret = fuseprov_blow_fuseregion(t, entries, entry_count,
				      FUSEPROV_CATEGORY_WRITE_PERM);
	if (ret != FUSEPROV_SUCCESS) {
		return ret;
	}

	NOTICE("Fuseprov: Fuse provisioning complete\n");
	return FUSEPROV_SUCCESS;
}
