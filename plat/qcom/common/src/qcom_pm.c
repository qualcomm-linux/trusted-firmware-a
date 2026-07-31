/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018, 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qcom/sec_core/sec_core.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <platform.h>
#include <platform_def.h>
#include <qcom_cpu.h>
#include <qcom_plat.h>

#define QCOM_LOCAL_PSTATE_WIDTH		4
#define QCOM_LOCAL_PSTATE_MASK		((1 << QCOM_LOCAL_PSTATE_WIDTH) - 1)

#if PSCI_OS_INIT_MODE
#define QCOM_LAST_AT_PLVL_MASK		(QCOM_LOCAL_PSTATE_MASK <<	\
					 (QCOM_LOCAL_PSTATE_WIDTH *	\
					  (PLAT_MAX_PWR_LVL + 1)))
#endif

/* Make composite power state parameter till level 0 */
#define qcom_make_pwrstate_lvl0(lvl0_state, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | ((type) << PSTATE_TYPE_SHIFT))

/* Make composite power state parameter till level 1 */
#define qcom_make_pwrstate_lvl1(lvl1_state, lvl0_state, type) \
		(((lvl1_state) << QCOM_LOCAL_PSTATE_WIDTH) | \
		qcom_make_pwrstate_lvl0(lvl0_state, type))

/* Make composite power state parameter till level 2 */
#define qcom_make_pwrstate_lvl2(lvl2_state, lvl1_state, lvl0_state, type) \
		(((lvl2_state) << (QCOM_LOCAL_PSTATE_WIDTH * 2)) | \
		qcom_make_pwrstate_lvl1(lvl1_state, lvl0_state, type))

/* Make composite power state parameter till level 3 */
#define qcom_make_pwrstate_lvl3(lvl3_state, lvl2_state, lvl1_state, lvl0_state, type) \
		(((lvl3_state) << (QCOM_LOCAL_PSTATE_WIDTH * 3)) | \
		qcom_make_pwrstate_lvl2(lvl2_state, lvl1_state, lvl0_state, type))

/* QCOM_CORE_PWRDN_EN_MASK happens to be same across all CPUs */
#define QCOM_CORE_PWRDN_EN_MASK		1

/* cpu power control happens to be same across all CPUs */
DEFINE_RENAME_SYSREG_RW_FUNCS(cpu_pwrctrl_val, S3_0_C15_C2_7)

const unsigned int qcom_pm_idle_states[] = {
	qcom_make_pwrstate_lvl0(QCOM_LOCAL_STATE_OFF,
			       PSTATE_TYPE_POWERDOWN),
	qcom_make_pwrstate_lvl0(QCOM_LOCAL_STATE_DEEPOFF,
			       PSTATE_TYPE_POWERDOWN),
	qcom_make_pwrstate_lvl1(QCOM_LOCAL_STATE_DEEPOFF,
			       QCOM_LOCAL_STATE_DEEPOFF,
			       PSTATE_TYPE_POWERDOWN),
	qcom_make_pwrstate_lvl2(QCOM_LOCAL_STATE_OFF,
			       QCOM_LOCAL_STATE_DEEPOFF,
			       QCOM_LOCAL_STATE_DEEPOFF,
			       PSTATE_TYPE_POWERDOWN),
	qcom_make_pwrstate_lvl3(QCOM_LOCAL_STATE_OFF,
			       QCOM_LOCAL_STATE_DEEPOFF,
			       QCOM_LOCAL_STATE_DEEPOFF,
			       QCOM_LOCAL_STATE_DEEPOFF,
			       PSTATE_TYPE_POWERDOWN),
	0,
};

/*******************************************************************************
 * Qualcomm standard platform handler called to check the validity of the power
 * state parameter. The power state parameter has to be a composite power
 * state.
 ******************************************************************************/
int qcom_validate_power_state(unsigned int power_state,
			     psci_power_state_t *req_state)
{
	unsigned int state_id;
	int i;

	assert(req_state);

	/*
	 *  Currently we are using a linear search for finding the matching
	 *  entry in the idle power state array. This can be made a binary
	 *  search if the number of entries justify the additional complexity.
	 */
	for (i = 0; !!qcom_pm_idle_states[i]; i++) {
#if PSCI_OS_INIT_MODE
		if ((power_state & ~QCOM_LAST_AT_PLVL_MASK) ==
		    qcom_pm_idle_states[i])
#else
		if (power_state == qcom_pm_idle_states[i])
#endif
			break;
	}

	/* Return error if entry not found in the idle state array */
	if (!qcom_pm_idle_states[i])
		return PSCI_E_INVALID_PARAMS;

	i = 0;
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	for (i = QCOM_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = state_id &
		    QCOM_LOCAL_PSTATE_MASK;
		state_id >>= QCOM_LOCAL_PSTATE_WIDTH;
	}
#if PSCI_OS_INIT_MODE
	req_state->last_at_pwrlvl = state_id & QCOM_LOCAL_PSTATE_MASK;
#endif

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * PLATFORM FUNCTIONS
 ******************************************************************************/

static void qcom_set_cpupwrctlr_val(void)
{
	unsigned long val;

	val = read_cpu_pwrctrl_val();
	val |= QCOM_CORE_PWRDN_EN_MASK;
	write_cpu_pwrctrl_val(val);

	isb();
}

/**
 * CPU power on function - ideally we want a wrapper since this function is
 * target specific. But to unblock teams.
 */
static int qcom_cpu_power_on(u_register_t mpidr)
{
	int core_pos = plat_core_pos_by_mpidr(mpidr);

	/* If not valid mpidr, return error */
	if (core_pos < 0 || core_pos >= PLATFORM_CORE_COUNT) {
		return PSCI_E_INVALID_PARAMS;
	}

	plat_qcom_pwr_domain_on(mpidr, core_pos);
	return PSCI_E_SUCCESS;
}

static bool is_cpu_off(const psci_power_state_t *target_state)
{
	if ((target_state->pwr_domain_state[QCOM_PWR_LVL0] ==
	     QCOM_LOCAL_STATE_OFF) ||
	    (target_state->pwr_domain_state[QCOM_PWR_LVL0] ==
	     QCOM_LOCAL_STATE_DEEPOFF)) {
		return true;
	} else {
		return false;
	}
}

static void qcom_cpu_power_on_finish(const psci_power_state_t *target_state)
{
	const uint8_t *pwr_states =
	    (const uint8_t *)target_state->pwr_domain_state;

	plat_qcom_pwr_domain_on_finish(plat_my_core_pos(), pwr_states);

	if (is_cpu_off(target_state)) {
		plat_qcom_gic_cpuif_enable();
	}
}

static void qcom_cpu_standby(plat_local_state_t cpu_state)
{
}

static void qcom_node_power_off(const psci_power_state_t *target_state)
{
	plat_qcom_pwr_domain_off((const uint8_t *)target_state->pwr_domain_state);
	if (is_cpu_off(target_state)) {
		plat_qcom_gic_cpuif_disable();
		qcom_set_cpupwrctlr_val();
	}
}

static void qcom_node_suspend(const psci_power_state_t *target_state)
{
	plat_qcom_pwr_domain_suspend((const uint8_t *)target_state->pwr_domain_state);
	if (is_cpu_off(target_state)) {
		plat_qcom_gic_cpuif_disable();
		qcom_set_cpupwrctlr_val();
	}
}

static void qcom_node_suspend_finish(const psci_power_state_t *target_state)
{
	const uint8_t *pwr_states =
	    (const uint8_t *)target_state->pwr_domain_state;
	plat_qcom_pwr_domain_suspend_finish(pwr_states);
	if (is_cpu_off(target_state)) {
		plat_qcom_gic_cpuif_enable();
	}
}

static __dead2 void assert_ps_hold(void)
{
#ifdef QCOM_PS_HOLD_REG
	mmio_write_32(QCOM_PS_HOLD_REG, 0);
	mdelay(1000);
#endif

	/* Should be dead before reaching this. */
	panic();
}

__dead2 void qcom_system_off(void)
{
	qcom_pmic_prepare_shutdown();
	assert_ps_hold();
}

__dead2 void qcom_system_reset(void)
{
	qcom_pmic_prepare_reset();
	assert_ps_hold();
}

void qcom_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	int i = 0;
	unsigned int state_id, power_state;
	int size = ARRAY_SIZE(qcom_pm_idle_states);

	/*
	 * Find deepest state.
	 * The arm_pm_idle_states[] array has last element by default 0,
	 * so the real deepest state is second last element of that array.
	 */
	power_state = qcom_pm_idle_states[size - 2];
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	while (state_id) {
		req_state->pwr_domain_state[i++] =
		    state_id & QCOM_LOCAL_PSTATE_MASK;
		state_id >>= QCOM_LOCAL_PSTATE_WIDTH;
	}

#if PSCI_OS_INIT_MODE
	req_state->last_at_pwrlvl = PLAT_MAX_PWR_LVL;
#endif
}

/*
 * Structure containing platform specific PSCI operations. Common
 * PSCI layer will use this.
 */
const plat_psci_ops_t plat_qcom_psci_pm_ops = {
	.pwr_domain_on = qcom_cpu_power_on,
	.pwr_domain_on_finish = qcom_cpu_power_on_finish,
	.cpu_standby = qcom_cpu_standby,
	.pwr_domain_off = qcom_node_power_off,
	.pwr_domain_suspend = qcom_node_suspend,
	.pwr_domain_suspend_finish = qcom_node_suspend_finish,
	.system_off = qcom_system_off,
	.system_reset = qcom_system_reset,
	.get_node_hw_state = NULL,
	.translate_power_state_by_mpidr = NULL,
	.get_sys_suspend_power_state = qcom_get_sys_suspend_power_state,
	.validate_power_state = qcom_validate_power_state,
};

/**
 * The Qualcomm Standard platform definition of platform porting API
 * `plat_setup_psci_ops`.
 */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	int err;

	qcom_sec_core_remap((uintptr_t)bl31_warm_entrypoint);

	err = plat_qcom_pwr_psci_init((uintptr_t)bl31_warm_entrypoint);
	if (err == PSCI_E_SUCCESS) {
		*psci_ops = &plat_qcom_psci_pm_ops;
	}

	return err;
}
