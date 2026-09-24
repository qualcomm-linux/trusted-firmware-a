#include <stdint.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <drivers/arm/css/scmi.h>

void scmi_ring_doorbell(struct scmi_channel_plat_info *plat_info)
{
	uint32_t db = mmio_read_32(plat_info->db_reg_addr);

	mmio_write_32(PLAT_SCMI_DB_REG_ADDR,
		      db | (1U << PLAT_SCMI_DB_OSM_IPC_SHIFT));
}

