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

#define FUSEPROV_RANDOM_ROW_COUNT        5U
#define FUSEPROV_RANDOM_DATA_SIZE \
        (FUSEPROV_RANDOM_ROW_COUNT * sizeof(uint64_t))

/* QFPROM stores FEC for the lower 56 data bits in MSB bits [62:56]. */
static uint32_t fuseprov_calculate_fec(uint32_t lsb_data,
                                              uint32_t msb_data)
{
        uint8_t lfsr[7] = { 0 };
        uint32_t temp;
        uint32_t fec_val = 0;
        uint64_t data_loc;
        int i;

        data_loc = ((uint64_t)msb_data << 32) | lsb_data;

        for (i = 0; i < 56; i++) {
                temp = lfsr[0] ^ ((data_loc >> i) & 0x1U);
                lfsr[0] = lfsr[1] ^ temp;
                lfsr[1] = lfsr[2];
                lfsr[2] = lfsr[3];
                lfsr[3] = lfsr[4];
                lfsr[4] = lfsr[5] ^ temp;
                lfsr[5] = lfsr[6];
                lfsr[6] = temp;
        }

        for (i = 6; i >= 0; i--)
                fec_val |= (uint32_t)lfsr[i] << i;

        /* Preserve only data bits before inserting the newly computed FEC. */
        msb_data &= 0x80FFFFFFU;

        return (fec_val << 24) | msb_data;
}

/* Prevent sensitive fuse values from remaining in the SEC.DAT buffer. */
static void fuseprov_clear_entry_data(fuseprov_qfuse_entry_t *entry)
{
        volatile uint8_t *lsb = (volatile uint8_t *)&entry->lsb_val;
        volatile uint8_t *msb = (volatile uint8_t *)&entry->msb_val;
        size_t i;

        for (i = 0; i < sizeof(entry->lsb_val); i++)
                lsb[i] = 0U;

        for (i = 0; i < sizeof(entry->msb_val); i++)
                msb[i] = 0U;
}

/* Read the corrected row to avoid reprogramming a previously blown row. */
static fuseprov_error_etype fuseprov_row_is_programmed(
        const fuseprov_transport_t *t, uint32_t address, bool *programmed)
{
        uint32_t fuse_data[2];
        fuseprov_err_t ret;

        ret = fuseprov_row_read(t, address, FUSEPROV_ADDR_CORR, fuse_data);
        if (ret != FUSEPROV_OK)
                return FUSEPROV_QFPROM_READ_ERROR;

        *programmed = (fuse_data[0] != 0U || fuse_data[1] != 0U);
        return FUSEPROV_SUCCESS;
}

static uint32_t fuseprov_count_random_rows(
        const fuseprov_qfuse_entry_t *entries, uint32_t entry_count,
        fuseprov_region_type_t region_type)
{
        uint32_t i;
        uint32_t random_row_count = 0U;

        for (i = 0U; i < entry_count; i++) {
                if (entries[i].region_type == region_type &&
                    entries[i].operation == FUSEPROV_OPERATION_BLOW_RANDOM) {
                        random_row_count++;
                }
        }

        return random_row_count;
}

static fuseprov_error_etype fuseprov_validate_random_row_counts(
        const fuseprov_qfuse_entry_t *entries, uint32_t entry_count)
{
        uint32_t random_row_count;

        random_row_count = fuseprov_count_random_rows(
                entries, entry_count, FUSEPROV_REGION_TYPE_SEC_HW_KEY);
        if (random_row_count > FUSEPROV_RANDOM_ROW_COUNT) {
                ERROR("Fuseprov: too many SHK random rows (%u > %u)\n",
                      random_row_count, FUSEPROV_RANDOM_ROW_COUNT);
                return FUSEPROV_SHK_GENERATION_FAILED;
        }

        random_row_count = fuseprov_count_random_rows(
                entries, entry_count, FUSEPROV_REGION_TYPE_OEM_PRODUCT_SEED);
        if (random_row_count > FUSEPROV_RANDOM_ROW_COUNT) {
                ERROR("Fuseprov: too many OEM product seed random rows "
                      "(%u > %u)\n",
                      random_row_count, FUSEPROV_RANDOM_ROW_COUNT);
                return FUSEPROV_SHK_GENERATION_FAILED;
        }

        random_row_count = fuseprov_count_random_rows(
                entries, entry_count, FUSEPROV_REGION_TYPE_OEM_SPARE);
        if (random_row_count > FUSEPROV_RANDOM_ROW_COUNT) {
                ERROR("Fuseprov: too many OEM spare random rows (%u > %u)\n",
                      random_row_count, FUSEPROV_RANDOM_ROW_COUNT);
                return FUSEPROV_OEM_SPARE_RAND_GEN_FAILED;
        }

        return FUSEPROV_SUCCESS;
}

/* SEC.DAT v3 contains a fixed header followed by a flat entry array. */
static fuseprov_error_etype fuseprov_parse_secdat_hdr(
        uint8_t *buffer, size_t buffer_len,
        fuseprov_secdat_hdr_t *hdr,
        fuseprov_qfuse_entry_t **entries,
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

        *entries = (fuseprov_qfuse_entry_t *)(buffer +
                                                 sizeof(fuseprov_secdat_hdr_t));
        *entry_count = hdr->num_entries;

        return FUSEPROV_SUCCESS;
}

/* Map a SEC.DAT region type to its ordered provisioning category.
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
        fuseprov_qfuse_entry_t *entries,
        uint32_t entry_count,
        fuseprov_category_t category,
        bool *did_program)
{
        uint32_t i;
        uint32_t fuse_data[2];
        uint32_t mask_fec_msb_bits;
        uint32_t msb_data;
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
                bool fec_enabled = false;
                fec_enabled |= entries[i].region_type ==
                        FUSEPROV_REGION_TYPE_OEM_PK_HASH;
                NOTICE("Fuseprov: FEC enabled: %x\n", fec_enabled);

                mask_fec_msb_bits = fec_enabled ? FUSEPROV_FEC_ROW_MSB_MASK :
                        FUSEPROV_GEN_ROW_MSB_MASK;
                bool flag_skip_blow = true;
                flag_skip_blow &= (fuse_data[0] & entries[i].lsb_val) ==
                        entries[i].lsb_val;
                flag_skip_blow &=
                        (fuse_data[1] & entries[i].msb_val &
                         mask_fec_msb_bits) ==
                        (entries[i].msb_val & mask_fec_msb_bits);
                if (flag_skip_blow) {
                        continue;
                }

                msb_data = entries[i].msb_val;
                if (fec_enabled)
                        msb_data = fuseprov_calculate_fec(entries[i].lsb_val,
                                                          msb_data);

                data = ((uint64_t)msb_data << 32) |
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
                       msb_data);

                fuseprov_clear_entry_data(&entries[i]);
                *did_program = true;
        }

        return FUSEPROV_SUCCESS;
}

/* Generate and provision random Secondary Hardware Key rows. */
static fuseprov_error_etype fuseprov_provision_shk(
        const fuseprov_transport_t *t,
        fuseprov_qfuse_entry_t *entries,
        uint32_t entry_count,
        bool *did_program)
{
        uint8_t random_data[FUSEPROV_RANDOM_DATA_SIZE];
        uint32_t i;
        uint32_t random_index = 0;
        int ret;

        ret = qti_rng_get_data(random_data, sizeof(random_data));
        if (ret != 0) {
                ERROR("Fuseprov: Failed to generate random data for SHK\n");
                return FUSEPROV_SHK_GENERATION_FAILED;
        }

        for (i = 0; i < entry_count; i++) {
                if (entries[i].region_type == FUSEPROV_REGION_TYPE_SEC_HW_KEY &&
                    entries[i].operation == FUSEPROV_OPERATION_BLOW_RANDOM) {
                        uint64_t data;
                        bool programmed;
                        fuseprov_err_t fret;
                        fuseprov_error_etype status;

                        status = fuseprov_row_is_programmed(
                                t, entries[i].raw_row_address, &programmed);
                        if (status != FUSEPROV_SUCCESS) {
                                ERROR("Fuseprov: Failed to read SHK fuse\n");
                                return status;
                        }

                        if (programmed) {
                                random_index++;
                                continue;
                        }

                        memcpy(&data, &random_data[random_index++ * 8], 8);

                        fret = fuseprov_rows_write(t,
                                        &entries[i].raw_row_address,
                                        &data, 1, NULL);
                        if (fret != FUSEPROV_OK) {
                                ERROR("Fuseprov: Failed to write SHK fuse\n");
                                return FUSEPROV_QFPROM_WRITE_ERROR;
                        }

                        fuseprov_clear_entry_data(&entries[i]);
                        *did_program = true;
                }
        }

        return FUSEPROV_SUCCESS;
}

/* Generate and provision random OEM product-seed rows. */
static fuseprov_error_etype fuseprov_provision_oem_product_seed(
        const fuseprov_transport_t *t,
        fuseprov_qfuse_entry_t *entries,
        uint32_t entry_count,
        bool *did_program)
{
        uint8_t random_data[FUSEPROV_RANDOM_DATA_SIZE];
        uint32_t fuse_data[2];
        uint32_t i;
        uint32_t random_index = 0;
        int ret;

        ret = qti_rng_get_data(random_data, sizeof(random_data));
        if (ret != 0) {
                ERROR("Fuseprov: Failed to generate random data for "
                      "OEM product seed\n");
                return FUSEPROV_SHK_GENERATION_FAILED;
        }

        for (i = 0; i < entry_count; i++) {
                if (entries[i].region_type ==
                    FUSEPROV_REGION_TYPE_OEM_PRODUCT_SEED &&
                    entries[i].operation == FUSEPROV_OPERATION_BLOW_RANDOM) {
                        uint64_t data;
                        bool programmed;
                        fuseprov_err_t fret;
                        fuseprov_error_etype status;

                        status = fuseprov_row_is_programmed(
                                t, entries[i].raw_row_address, &programmed);
                        if (status != FUSEPROV_SUCCESS) {
                                ERROR("Fuseprov: Failed to read OEM product "
                                      "seed fuse\n");
                                return status;
                        }

                        if (programmed) {
                                random_index++;
                                continue;
                        }

                        memcpy(fuse_data, &random_data[random_index++ * 8],
                               sizeof(fuse_data));

                        /* OEM product-seed rows contain hardware-generated
                         * FEC bits.
                         */
                        fuse_data[1] = fuseprov_calculate_fec(fuse_data[0],
                                                             fuse_data[1]);
                        data = ((uint64_t)fuse_data[1] << 32) | fuse_data[0];

                        fret = fuseprov_rows_write(t,
                                        &entries[i].raw_row_address,
                                        &data, 1, NULL);
                        if (fret != FUSEPROV_OK) {
                                ERROR("Fuseprov: Failed to write OEM product "
                                      "seed fuse\n");
                                return FUSEPROV_QFPROM_WRITE_ERROR;
                        }

                        fuseprov_clear_entry_data(&entries[i]);
                        *did_program = true;
                }
        }

        return FUSEPROV_SUCCESS;
}

/* Generate and provision random OEM spare rows. */
static fuseprov_error_etype fuseprov_provision_oem_spare(
        const fuseprov_transport_t *t,
        fuseprov_qfuse_entry_t *entries,
        uint32_t entry_count,
        bool *did_program)
{
        uint8_t random_data[FUSEPROV_RANDOM_DATA_SIZE];
        uint32_t i;
        uint32_t random_index = 0;
        int ret;

        ret = qti_rng_get_data(random_data, sizeof(random_data));
        if (ret != 0) {
                ERROR("Fuseprov: Failed to generate random data for "
                      "OEM spare\n");
                return FUSEPROV_OEM_SPARE_RAND_GEN_FAILED;
        }

        for (i = 0; i < entry_count; i++) {
                if (entries[i].region_type == FUSEPROV_REGION_TYPE_OEM_SPARE &&
                    entries[i].operation == FUSEPROV_OPERATION_BLOW_RANDOM) {
                        uint64_t data;
                        bool programmed;
                        fuseprov_err_t fret;
                        fuseprov_error_etype status;

                        status = fuseprov_row_is_programmed(
                                t, entries[i].raw_row_address, &programmed);
                        if (status != FUSEPROV_SUCCESS) {
                                ERROR("Fuseprov: Failed to read OEM spare "
                                      "fuse\n");
                                return status;
                        }

                        if (programmed) {
                                random_index++;
                                continue;
                        }

                        memcpy(&data, &random_data[random_index++ * 8], 8);

                        fret = fuseprov_rows_write(t,
                                        &entries[i].raw_row_address,
                                        &data, 1, NULL);
                        if (fret != FUSEPROV_OK) {
                                ERROR("Fuseprov: Failed to write OEM spare "
                                      "fuse\n");
                                return FUSEPROV_QFPROM_WRITE_ERROR;
                        }

                        fuseprov_clear_entry_data(&entries[i]);
                        *did_program = true;
                }
        }

        return FUSEPROV_SUCCESS;
}

/* Parse and provision the authenticated SEC.DAT v3 buffer. */
fuseprov_error_etype fuseprov_blow_fuses_sec_elf_v3(
        const fuseprov_transport_t *t,
        uint8_t *buf,
        uint32_t len,
        bool *did_program)
{
        fuseprov_secdat_hdr_t hdr;
        fuseprov_qfuse_entry_t *entries;
        uint32_t entry_count;
        fuseprov_error_etype ret;
        uint32_t magic;

        if (buf == NULL || len == 0 || len > FUSEPROV_SECDAT_BUFFER_SIZE ||
            t == NULL || did_program == NULL) {
                return FUSEPROV_INVALID_ARG;
        }

        *did_program = false;

        /* The buffer may contain arbitrary data when no secdata is
         * present.
         * Validate the header magic values instead of checking for zeroes.
        uint32_t i;
        for (i = 0; ((i < len) && (buf[i] == 0)); i++);

        if (i == len) {
                NOTICE("Fuseprov: SEC.DAT buffer is empty, nothing to blow\n");
                return  FUSEPROV_SECDAT_DEFAULT_NOFUSES ;
        }

        NOTICE("Fuseprov: Non-Zero Byte: %d.\n", i);
          */

        /* Validate the magic numbers as check for non-empty buffer */
        bool invalid_secdata = len < 2u*sizeof(uint32_t);
        if (!invalid_secdata) {
                memcpy(&magic, buf, sizeof(uint32_t));
                invalid_secdata |= magic != FUSEPROV_SECDAT_MAGIC1;
                memcpy(&magic, buf + sizeof(uint32_t), sizeof(uint32_t));
                invalid_secdata |= magic != FUSEPROV_SECDAT_MAGIC2;
        }
        if (invalid_secdata) {
                NOTICE("Fuseprov: secdata magic numbers are unavailable\n");
                NOTICE("Fuseprov: Clearing the secdata buffer\n");
                memset(buf, 0, len);
                return FUSEPROV_SECDAT_DEFAULT_NOFUSES;
        }

        NOTICE("Fuseprov: Starting fuse provisioning\n");

        ret = fuseprov_parse_secdat_hdr(buf, len, &hdr, &entries, &entry_count);
        if (ret != FUSEPROV_SUCCESS) {
                ERROR("Fuseprov: Failed to parse SEC.DAT header\n");
                return ret;
        }

        ret = fuseprov_validate_random_row_counts(entries, entry_count);
        if (ret != FUSEPROV_SUCCESS)
                return ret;

        /* Write-permission rows are last because they lock further writes. */
        ret = fuseprov_blow_fuseregion(t, entries, entry_count,
                                      FUSEPROV_CATEGORY_GENERAL, did_program);
        if (ret != FUSEPROV_SUCCESS) {
                return ret;
        }
        ret = fuseprov_provision_shk(t, entries, entry_count, did_program);
        if (ret != FUSEPROV_SUCCESS) {
                return ret;
        }
        ret = fuseprov_provision_oem_product_seed(t, entries, entry_count,
                                                  did_program);
        if (ret != FUSEPROV_SUCCESS) {
                return ret;
        }
        ret = fuseprov_provision_oem_spare(t, entries, entry_count,
                                           did_program);
        if (ret != FUSEPROV_SUCCESS) {
                return ret;
        }
        ret = fuseprov_blow_fuseregion(
                t, entries, entry_count, FUSEPROV_CATEGORY_OEM_CONFIG,
                did_program);
        if (ret != FUSEPROV_SUCCESS) {
                return ret;
        }
        ret = fuseprov_blow_fuseregion(t, entries, entry_count,
                                      FUSEPROV_CATEGORY_SECBOOT, did_program);
        if (ret != FUSEPROV_SUCCESS) {
                return ret;
        }
        ret = fuseprov_blow_fuseregion(t, entries, entry_count,
                                      FUSEPROV_CATEGORY_FEC_EN, did_program);
        if (ret != FUSEPROV_SUCCESS) {
                return ret;
        }
        ret = fuseprov_blow_fuseregion(t, entries, entry_count,
                                      FUSEPROV_CATEGORY_READ_PERM, did_program);
        if (ret != FUSEPROV_SUCCESS) {
                return ret;
        }
        ret = fuseprov_blow_fuseregion(
                t, entries, entry_count, FUSEPROV_CATEGORY_WRITE_PERM,
                did_program);
        if (ret != FUSEPROV_SUCCESS) {
                return ret;
        }
        NOTICE("Fuseprov: Fuse provisioning complete\n");
        return FUSEPROV_SUCCESS;
}
