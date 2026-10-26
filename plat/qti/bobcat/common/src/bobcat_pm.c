/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <platform_def.h>
#include <qti_plat.h>

/*
 * Native bobcat backend for the QTI PSCI power-domain hooks declared in
 * qti_plat.h. Secondary-core bringup and the firmware low-power-mode node
 * handling are not ported yet, so the power-on/off and suspend hooks are
 * stubs. Only CPU-level standby (WFI retention) is advertised.
 */

/*
 * The secondary-core reset vector (RVBAR) is programmed by
 * qti_sec_core_remap(), invoked from plat_setup_psci_ops() ahead of this hook,
 * so there is nothing left to do here.
 */
int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	(void)warmboot_entry;

	return PSCI_E_SUCCESS;
}

/*
 * Secondary-core power-on sequence is not ported. Fail the request instead of
 * reporting success for a core that will never start executing.
 */
int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	(void)mpidr;
	(void)core_pos;

	return PSCI_E_INTERN_FAIL;
}

void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)core_pos;
	(void)states;
}

void plat_qti_pwr_domain_off(const uint8_t *states)
{
	(void)states;
}

void plat_qti_pwr_domain_suspend(const uint8_t *states)
{
	(void)states;
}

void plat_qti_pwr_domain_suspend_finish(const uint8_t *states)
{
	(void)states;
}

__dead2 void qti_system_off(void)
{
	/* De-assert PS_HOLD to trigger system reset */
	mmio_write_32(QTI_PS_HOLD_REG, 0);
	while (1) {
	}
}

__dead2 void qti_system_reset(void)
{
	/* De-assert PS_HOLD to trigger system reset */
	mmio_write_32(QTI_PS_HOLD_REG, 0);
	while (1) {
	}
}

/*
 * No native EL3 handler exists for QTI-specific interrupts; an unhandled
 * interrupt reaching EL3 is not expected, so log and drop it.
 */
void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle)
{
	(void)handle;

	ERROR("Unhandled EL3 interrupt %u\n", id);
}

/*
 * Power collapse is not supported natively, so advertise only CPU-level
 * standby (retention), entered with a WFI and handled by qti_cpu_standby().
 */
const unsigned int *plat_qti_pm_idle_states(void)
{
	static const unsigned int idle_states[] = {
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_STB,
				       PSTATE_TYPE_STANDBY),
		0,
	};

	return idle_states;
}
