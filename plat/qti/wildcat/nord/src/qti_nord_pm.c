/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <platform_def.h>

#define WAIT_FOR_SAIL_MS		U(1000)

#define QTI_MD_SD_GET_MSG_TYPE(x)	\
	(((x) & QTI_MD_SD_MSG_TYPE_MASK) >> QTI_MD_SD_MSG_TYPE_SHIFT)
#define QTI_MD_SD_GET_SUBSYS_TYPE(x)	\
	(((x) & QTI_MD_SD_SUBSYS_TYPE_MASK) >> QTI_MD_SD_SUBSYS_TYPE_SHIFT)

static void qti_md_sd_write_status(uint32_t status)
{
	uint32_t msg_type = QTI_MD_SD_GET_MSG_TYPE(status);
	uint32_t subsys = QTI_MD_SD_GET_SUBSYS_TYPE(status);

	if (subsys != QTI_MD_SD_TZ_SRC) {
		return;
	}

	if (msg_type == QTI_MD_SD_NO_ACK) {
		mmio_write_32(QTI_TCSR_MAIN2SAIL_GP_NONSEC_STATUS7_REG, status);
	} else if (msg_type == QTI_MD_SD_WAIT_FOR_ACK) {
		mmio_write_32(QTI_TCSR_MAIN2SAIL_GP_NONSEC_STATUS8_REG, status);
	} else {
		return;
	}

	dsbsy();
	isb();
	mmio_write_32(QTI_APSS_INTU_TZ_IPC_INTERRUPT,
		      QTI_APSS_INTU_TZ_IPC_INTERRUPT_SMSS_TZ_IPC_BMSK);
}

static uint32_t qti_md_sd_read_sail_status(void)
{
	return mmio_read_32(QTI_TCSR_SAIL2MAIN_GP_NONSEC_SHADOW_STATUS4_REG);
}

static void qti_md_sd_psci_status_com(uint32_t status)
{
	uint32_t status_to_sd;

	switch (status) {
	case QTI_PSCI_GRACEFUL_SHUTDOWN_CMD:
		status_to_sd = QTI_MD_GRACEFUL_SHUTDOWN;
		break;
	case QTI_PSCI_HARD_RESET_CMD:
	default:
		status_to_sd = QTI_MD_SOC_HR;
		break;
	}

	qti_md_sd_write_status(status_to_sd);

	if (QTI_MD_SD_GET_MSG_TYPE(status_to_sd) == QTI_MD_SD_NO_ACK) {
		return;
	}

	for (uint32_t count = 0U; count < WAIT_FOR_SAIL_MS; count++) {
		mdelay(1);
		if (qti_md_sd_read_sail_status() == QTI_SAIL_RESET_READY) {
			break;
		}
	}
}

void qti_platform_psci_system_reset(void)
{
	qti_md_sd_psci_status_com(QTI_PSCI_HARD_RESET_CMD);
}

void qti_platform_psci_system_off(void)
{
	qti_md_sd_psci_status_com(QTI_PSCI_GRACEFUL_SHUTDOWN_CMD);
}
