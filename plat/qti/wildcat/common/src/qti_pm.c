/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018, 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Changes from Qualcomm Technologies, Inc. are provided under the following
 * license: Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <bl31qtilib_cb_interface.h>
#include <bl31qtilib_defs.h>
#include <bl31qtilib_interface.h>
#include <platform.h>
#include <platform_def.h>
#include <qti_plat.h>

static __dead2 void assert_ps_hold(void)
{
	/* Should be dead before reaching this. */
	panic();
}

/*
 * qti_get_sys_suspend_power_state
 *
 * This function determines the deepest allowed state on the subsystem and
 * properly assigns it into the psci_power_state_t type.
 *
 * param[out] req_state    Variable filling out the deepest state
 */
void qti_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	uint32_t idx = 0;
	uint64_t deepest_state = bl31qtilib_psci_get_suspend_state();

	while (deepest_state && (idx <= PLAT_MAX_PWR_LVL)) {
		req_state->pwr_domain_state[idx] =
			(deepest_state & BL31QTILIB_PSCI_FIELD_MASK);
		deepest_state >>= BL31QTILIB_PSCI_FIELD_WIDTH;
		idx++;
	}
}

/*
 * qti_pwr_domain_on
 *
 * Turns on the requested core (and any related required power domains).
 *
 * param[in]   mpidr   MPIDR of the requested domain
 * return  PSCI_E_SUCCESS on success, else error
 */
static int qti_pwr_domain_on(u_register_t mpidr)
{
	return bl31qtilib_psci_power_domain_on(mpidr);
};

/*
 * qti_pwr_domain_on_finish
 *
 * Function that runs on the core to finish setting up after being requested
 * to come online by qti_pwr_domain_on
 *
 * @param[in]  target_state    State to unconfigure from to turn back on
 */
static void qti_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr();
	psci_power_state_t suspend_state = { { PSCI_LOCAL_STATE_RUN } };
	uint32_t i;

	qti_get_sys_suspend_power_state(&suspend_state);
	for (i = PSCI_CPU_PWR_LVL; i <= PLAT_MAX_PWR_LVL; i++) {
		if (suspend_state.pwr_domain_state[i] >
		    target_state->pwr_domain_state[i]) {
			suspend_state.pwr_domain_state[i] =
				target_state->pwr_domain_state[i];
		}
	}

	plat_qti_gic_pcpu_init();
	plat_qti_gic_cpuif_enable();
	bl31qtilib_psci_power_domain_on_finish(
		mpidr, (const uint8_t *)suspend_state.pwr_domain_state);
}

/*
 * qti_pwr_domain_off
 *
 * Early function called as part of turning off a power domain. Unused on
 * current architecture
 *
 * @param[in] target_state Intended state to configure while going offline
 */
static void qti_pwr_domain_off(const psci_power_state_t *target_state)
{
}

/*
 * qti_pwr_domain_power_down_wfi
 *
 * The final call when turning off a power domain, this completes any register
 * setting and then lets the core power off gracefully. Unexpected to return.
 *
 * @param[in] target_state Intended state to configure while going offline
 */
static __dead2 void
qti_pwr_domain_power_down_wfi(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr();
	psci_power_state_t suspend_state = { { PSCI_LOCAL_STATE_RUN } };
	uint32_t i;

	qti_get_sys_suspend_power_state(&suspend_state);
	for (i = PSCI_CPU_PWR_LVL; i <= PLAT_MAX_PWR_LVL; i++) {
		if (suspend_state.pwr_domain_state[i] >
		    target_state->pwr_domain_state[i]) {
			suspend_state.pwr_domain_state[i] =
				target_state->pwr_domain_state[i];
		}
	}

	bl31qtilib_psci_suspend(
		mpidr, (const uint8_t *)suspend_state.pwr_domain_state);
	bl31qtilib_psci_cpu_pwr_down(mpidr);
	plat_qti_gic_cpuif_disable();

	while (true) {
		wfi();
	}
}

/*
 * qti_pwr_domain_suspend
 *
 * Sends the power domain down into suspend. Intended to be the final function
 * before wfi is initiated.
 *
 * @param[in] target_state Intended sleep mode(s) for this power domain
 */
static void qti_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	bl31qtilib_psci_suspend(
		read_mpidr(), (const uint8_t *)target_state->pwr_domain_state);
	plat_qti_gic_cpuif_disable();
}

/*
 * qti_pwr_domain_suspend_finish
 *
 * Unconfigures everything after waking up from suspend, according to the LPMs
 * specified.
 *
 * @param[in] target_state State to restore from
 */
static void
qti_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	plat_qti_gic_cpuif_enable();
	bl31qtilib_psci_resume(read_mpidr(),
			       (const uint8_t *)target_state->pwr_domain_state);
}

/*
 * qti_cpu_standby
 *
 * A quicker implementation of suspend, this function runs when certain
 * conditions are met and there is no aggregation to be done before sleep is
 * initiated.
 *
 * @param[in] cpu_state Local LPM to send the core into
 */
static void qti_cpu_standby(plat_local_state_t cpu_state)
{
	psci_power_state_t target_state = { { PSCI_LOCAL_STATE_RUN } };

	target_state.pwr_domain_state[PSCI_CPU_PWR_LVL] = cpu_state;
	qti_pwr_domain_suspend(&target_state);
	wfi();
	qti_pwr_domain_suspend_finish(&target_state);
}

/*
 * qti_system_off
 *
 * Turns the entire system off, without any intention of turning back on.
 */
__dead2 void qti_system_off(void)
{
	bl31qtilib_psci_system_off();

	assert_ps_hold();
}


/*
 * qti_system_reset
 *
 * Restarts the system. Not intended to return from this function.
 */
__dead2 void qti_system_reset(void)
{
	bl31qtilib_psci_system_reset();

	assert_ps_hold();
}

/*
 * qti_system_reset2
 *
 * Restarts the system, with settings determined by the passed in arguments.
 * Not intended to return from this function.
 *
 * @param[in]  is_vendor   Determines if it's a vendor-specific setting
 * @param[in]  reset_type  Warm or cold reset
 * @param[in]  cookie      Cookie containing any extra flags
 */
static int qti_system_reset2(int is_vendor, int reset_type, u_register_t cookie)
{
	bl31qtilib_psci_system_reset2(reset_type, cookie);

	/* Stay in WFI until the system resets / reboots according to spec */
	while (1) {
		wfi();
	}

	/* Major failure upon reaching here */
	return 0;
}


int qti_validate_power_state(unsigned int power_state,
			     psci_power_state_t *req_state)
{
	bl31qtilib_psci_validate_power_state(power_state,
					     req_state->pwr_domain_state);

	return PSCI_E_SUCCESS;
}

/*
 * Structure containing platform specific PSCI operations. Common
 * PSCI layer will use this.
 */
const plat_psci_ops_t plat_qti_psci_pm_ops = {
	.pwr_domain_on = qti_pwr_domain_on,
	.pwr_domain_on_finish = qti_pwr_domain_on_finish,
	.cpu_standby = qti_cpu_standby,
	.pwr_domain_off = qti_pwr_domain_off,
	.pwr_domain_suspend = qti_pwr_domain_suspend,
	.pwr_domain_suspend_finish = qti_pwr_domain_suspend_finish,
	.pwr_domain_pwr_down = qti_pwr_domain_power_down_wfi,
	.system_off = qti_system_off,
	.system_reset = qti_system_reset,
	.system_reset2 = qti_system_reset2,
	.get_node_hw_state = NULL,
	.translate_power_state_by_mpidr = NULL,
	.get_sys_suspend_power_state = qti_get_sys_suspend_power_state,
	.validate_power_state = qti_validate_power_state,
};

/*
 * The QTI Standard platform definition of platform porting API
 * `plat_setup_psci_ops`.
 *
 */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	int err = PSCI_E_SUCCESS;
#ifndef DISABLE_PSCI_INIT
	uint32_t mpidr = read_mpidr();

	err = bl31qtilib_psci_init((uintptr_t)bl31_warm_entrypoint, mpidr);
#endif
	if (err == PSCI_E_SUCCESS) {
		*psci_ops = &plat_qti_psci_pm_ops;
	}

	return err;
}

/*******************************************************************************
 * Platform handler to calculate the proper target power level at the
 * specified affinity level
 ******************************************************************************/
plat_local_state_t plat_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *states,
					     unsigned int ncpu)
{
	uint32_t mpidr = read_mpidr();

	return (plat_local_state_t)bl31qtilib_psci_get_target_pwr_state(
		lvl, (uint8_t *)states, ncpu, mpidr);
}
