/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/sail_comm/sail_comm.h>

#ifdef QTI_SAIL_COMM

#include <arch_helpers.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <platform_def.h>

#define WAIT_FOR_SAIL_MS		U(1000)

#define QTI_TCSR_MAIN2SAIL_GP_NONSEC_STATUS7_REG	U(0x01F7A028)
#define QTI_TCSR_MAIN2SAIL_GP_NONSEC_STATUS8_REG	U(0x01F7A02C)
#define QTI_TCSR_SAIL2MAIN_GP_NONSEC_SHADOW_STATUS4_REG	U(0x01F7B014)

#define QTI_APSS_INTU_TZ_IPC_INTERRUPT			U(0x17824004)
#define QTI_APSS_INTU_TZ_IPC_INTERRUPT_SMSS_TZ_IPC_BMSK	U(0x0080000)

#define QTI_MD_SD_MSG_TYPE_SHIFT	U(16)
#define QTI_MD_SD_MSG_TYPE_MASK		(U(0xFF) << QTI_MD_SD_MSG_TYPE_SHIFT)
#define QTI_MD_SD_SUBSYS_TYPE_SHIFT	U(24)
#define QTI_MD_SD_SUBSYS_TYPE_MASK	(U(0xFF) << QTI_MD_SD_SUBSYS_TYPE_SHIFT)

#define QTI_SAIL_RESET_READY		U(0xA0300100)
#define QTI_MD_GRACEFUL_SHUTDOWN	U(0xB2200430)
#define QTI_MD_SOC_WR			U(0xB2200320)
#define QTI_MD_SOC_HR			U(0xB2200510)
#define QTI_MD_SD_TZ_SRC		U(0xB2)
#define QTI_MD_SD_NO_ACK		U(0x10)
#define QTI_MD_SD_WAIT_FOR_ACK		U(0x20)

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

static void qti_md_sd_psci_status_com(uint32_t status_to_sd)
{
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

void qti_sail_comm_system_reset(void)
{
	qti_md_sd_psci_status_com(QTI_MD_SOC_HR);
}

void qti_sail_comm_system_warm_reset(void)
{
	qti_md_sd_psci_status_com(QTI_MD_SOC_WR);
}

void qti_sail_comm_system_off(void)
{
	qti_md_sd_psci_status_com(QTI_MD_GRACEFUL_SHUTDOWN);
}

#endif /* QTI_SAIL_COMM */
