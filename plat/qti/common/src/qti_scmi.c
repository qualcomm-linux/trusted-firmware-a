#include <common/debug.h>
#include <drivers/arm/css/scmi.h>
#include <lib/spinlock.h>

#include "qti_plat.h"

/*
 * The handles for invoking the SCMI driver APIs after the driver
 * has been initialized.
 */

/*
 * The SCMI channel pointer
 */
static scmi_channel_t qti_scmi_channel_obj;

/* SCMI channel lock */
spinlock_t qti_scmi_lock;

/* SCMI cahnnel platform info */
scmi_channel_plat_info_t qti_scmi_plat_info;

/*
 * qti_scmi_init - Initialize SCMI channel for clock management.
 *
 * This function must be called once during BL31 initialization before
 * any SCMI clock operations are performed. Sets up the channel structure
 * and lock before calling scmi_init().
 */
void qti_scmi_init(void)
{
	qti_scmi_plat_info.scmi_mbx_mem = PLAT_SCMI_MBX_MEM_BASE;
	qti_scmi_plat_info.db_reg_addr = PLAT_SCMI_DB_REG_ADDR;
	qti_scmi_plat_info.db_preserve_mask = ~(1U << PLAT_SCMI_DB_OSM_IPC_SHIFT);
	qti_scmi_plat_info.db_modify_mask = (1U << PLAT_SCMI_DB_OSM_IPC_SHIFT);
	qti_scmi_plat_info.ring_doorbell = &scmi_ring_doorbell;
	qti_scmi_channel_obj.info = &qti_scmi_plat_info;
	qti_scmi_channel_obj.lock = &qti_scmi_lock;
	scmi_init(&qti_scmi_channel_obj, NULL);
}

scmi_channel_t *qti_scmi_get_channel(void)
{
	return &qti_scmi_channel_obj;
}

