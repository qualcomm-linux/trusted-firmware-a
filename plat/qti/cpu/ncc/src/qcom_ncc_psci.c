/*
 * Copyright (c) 2023-2025, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2023-2025, Qualcomm Technologies, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * NCC PSCI platform hooks.
 *
 * Implements the plat_qti_pwr_* interface consumed by
 * plat/qti/common/src/qti_pm.c.  All CPU and cluster power transitions use
 * the SCMI Power Domain Management Protocol via the CPUCP HostLib wrappers
 * (cpucp_cluster_core_power_on / cpucp_cluster_core_power_off).  NCC LPM
 * register programming is kept here as it is NCC-hardware-specific.
 *
 * Reference SCMI implementation:
 *   cpucp/hostlib/host_v2/init/common/scmi_power.c
 *
 * Integration with qti_pm.c:
 *   qti_pm.c owns the plat_psci_ops_t table and plat_setup_psci_ops().
 *   This file provides the platform-specific back-end that qti_pm.c calls
 *   through the plat_qti_pwr_* function pointers declared in qti_plat.h.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <platform_def.h>
#include <qti_cpu.h>
#include <qti_plat.h>

#include <drivers/arm/gicv3.h>
#include <hwm_lock.h>
#include <lib/spinlock.h>
#include <ncc_cpu_power.h>
#include <ncc_hwio.h>

/*
 * PSCI extended power-state encoding helpers.
 *
 * Bits [15:0]  = StateID  (NCC state packed as level-0 | level-1 << 4)
 * Bit  [16]    = StateType (0 = standby, 1 = powerdown)
 * Bits [27:24] = PowerLevel
 */
#define NCC_PSTATE_TYPE_SHIFT		16U
#define NCC_PSTATE_LVL_SHIFT		24U
#define NCC_PSTATE(state_id, lvl) \
	(((state_id) & 0xFFFFU) | (1U << NCC_PSTATE_TYPE_SHIFT) | \
	 ((lvl) << NCC_PSTATE_LVL_SHIFT))

/*
 * NCC state IDs packed for the QTI validate_power_state parser.
 * qti_pm.c extracts QTI_LOCAL_PSTATE_WIDTH (4) bits per power level:
 *   pwr_domain_state[0] = state_id & 0xF          (core state)
 *   pwr_domain_state[1] = (state_id >> 4) & 0xF   (cluster state)
 */
#define NCC_STATEID_C3		(STATE_C3)
#define NCC_STATEID_C4		(STATE_C4)
#define NCC_STATEID_CL4		(STATE_C4 | (STATE_CL4 << QTI_LOCAL_PSTATE_WIDTH))
#define NCC_STATEID_CL5		(STATE_C4 | (STATE_CL5 << QTI_LOCAL_PSTATE_WIDTH))

/* --------------------------------------------------------------------------
 * Per-cluster LPM vote counters
 *
 * cl4_count[i] / cl5_count[i] track how many cores in physical cluster i
 * have voted for CL4 / CL5.  A cluster may enter CL4/CL5 only when every
 * currently-ON core has voted.  Cores that are OFF are pre-counted so the
 * threshold is met as soon as all ON cores vote.
 * -------------------------------------------------------------------------- */

/*
 * Spin lock protecting the NCC LPM vote counters (cl4_count / cl5_count)
 * against concurrent access from multiple TF-A CPU contexts.
 * Static zero-initialisation is sufficient; spin_lock() / spin_unlock()
 * require no explicit init call.
 */
static spinlock_t ncc_pm_lock;

static int cl4_count[PLAT_CLUSTER_COUNT];
static int cl5_count[PLAT_CLUSTER_COUNT];

/* Stub function for NCC targets */
/* This is handled in cpucp */

void plat_qti_set_cpupwrctlr_val(void)
{

}

/* --------------------------------------------------------------------------
 * NCC LPM register helpers
 * -------------------------------------------------------------------------- */

/*
 * ncc_config_core_lpm - enable/disable C3/C4 LPM permission for a core.
 *
 * @cpu_id      physical core index (will be masked to per-cluster index)
 * @state_id    STATE_C3 or STATE_C4
 * @addr_offset cluster register base offset (cluster_id * NCC_CLUSTER_STRIDE)
 * @enable      1 = allow LPM entry, 0 = disallow
 */
static void ncc_config_core_lpm(uint8_t cpu_id, plat_local_state_t state_id,
				uint64_t addr_offset, uint8_t enable)
{
	uint64_t val;

	cpu_id = cpu_id % PLAT_CORE_COUNT_PER_CLUSTER;

	switch (state_id) {
	case STATE_C4:
		val = mmio_read_64((unsigned long)(
			NCC0_NCC_NCC_ARCH_0_NCC_PWR_CFG1 + addr_offset));
		if (enable)
			val |= (((0x1 << cpu_id) & OK2ENTERC4_MASK)
				<< OK2ENTERC4_SHIFT);
		else
			val &= ~(((0x1 << cpu_id) & OK2ENTERC4_MASK)
				 << OK2ENTERC4_SHIFT);
		mmio_write_64(NCC0_NCC_NCC_ARCH_0_NCC_PWR_CFG1 + addr_offset,
			      val);
		[[fallthrough]];
	case STATE_C3:
		val = mmio_read_64((unsigned long)(
			NCC0_NCC_NCC_ARCH_0_NCC_PWR_CFG1 + addr_offset));
		if (enable)
			val |= (((0x1 << cpu_id) & OK2ENTERC3_MASK)
				<< OK2ENTERC3_SHIFT);
		else
			val &= ~(((0x1 << cpu_id) & OK2ENTERC3_MASK)
				 << OK2ENTERC3_SHIFT);
		mmio_write_64(NCC0_NCC_NCC_ARCH_0_NCC_PWR_CFG1 + addr_offset,
			      val);
		break;
	default:
		break;
	}
}

/*
 * ncc_config_cluster_lpm - enable/disable CL4/CL5 LPM permission for a
 * cluster.
 *
 * @state_id    STATE_CL4 or STATE_CL5
 * @addr_offset cluster register base offset
 * @enable      1 = allow LPM entry, 0 = disallow
 *
 * CL4 also requires CL3 to be enabled; both bits are managed together.
 * CL5 falls through to CL4 so both levels are configured atomically.
 */
static void ncc_config_cluster_lpm(plat_local_state_t state_id,
				   uint64_t addr_offset, uint8_t enable)
{
	uint64_t val;

	switch (state_id) {
	case STATE_CL5:
		val = mmio_read_64((unsigned long)(
			NCC0_NCC_NCC_ARCH_0_NCC_BOOT_CFG1 + addr_offset));
		if (enable)
			val |= ((0x1 & OK2ENTERCL5_MASK) << OK2ENTERCL5_SHIFT);
		else
			val &= ~((0x1 & OK2ENTERCL5_MASK) << OK2ENTERCL5_SHIFT);
		mmio_write_64(NCC0_NCC_NCC_ARCH_0_NCC_BOOT_CFG1 + addr_offset,
			      val);
		[[fallthrough]];
	case STATE_CL4:
		val = mmio_read_64((unsigned long)(
			NCC0_NCC_NCC_ARCH_0_NCC_PWR_CFG1 + addr_offset));
		if (enable) {
			val |= ((0x1 & OK2ENTERCL4_MASK) << OK2ENTERCL4_SHIFT);
			/* CL3 must be enabled for CL4 to work */
			val |= ((0x1 & OK2ENTERCL3_MASK) << OK2ENTERCL3_SHIFT);
		} else {
			val &= ~((0x1 & OK2ENTERCL4_MASK) << OK2ENTERCL4_SHIFT);
			val &= ~((0x1 & OK2ENTERCL3_MASK) << OK2ENTERCL3_SHIFT);
		}
		mmio_write_64(NCC0_NCC_NCC_ARCH_0_NCC_PWR_CFG1 + addr_offset,
			      val);
		break;
	default:
		break;
	}
}

/*
 * ncc_set_core_rvbar - force/clear RVBAR fetch for a core on next power-on.
 *
 * When set, the core fetches its reset vector from RVBAR on the next
 * power-on, which is required for a clean warm-boot after a full
 * power-collapse (CL5 / suspend).
 *
 * @cpu_id      physical core index within the cluster
 * @cluster_id  physical cluster index
 * @enable      1 = force RVBAR fetch, 0 = clear
 */
static void ncc_set_core_rvbar(uint8_t cpu_id, uint8_t cluster_id,
			       uint8_t enable)
{
	uint64_t addr_offset = (uint64_t)cluster_id * NCC_CLUSTER_STRIDE;
	uint64_t val;

	cpu_id = cpu_id % PLAT_CORE_COUNT_PER_CLUSTER;

	val = mmio_read_64((unsigned long)(
		NCC0_NCC_NCC_ARCH_0_NCC_PWR_CTL3 + addr_offset));
	if (enable)
		val |= (((0x1 << cpu_id) & CORFRCCPURVBAR_MASK)
			<< CORFRCCPURVBAR_SHIFT);
	else
		val &= ~(((0x1 << cpu_id) & CORFRCCPURVBAR_MASK)
			 << CORFRCCPURVBAR_SHIFT);
	mmio_write_64(NCC0_NCC_NCC_ARCH_0_NCC_PWR_CTL3 + addr_offset, val);
}

/* --------------------------------------------------------------------------
 * plat_qti_pwr_* hooks – called by qti_pm.c PSCI ops
 * -------------------------------------------------------------------------- */

/*
 * plat_qti_pwr_domain_on
 *
 * Power on the CPU identified by @mpidr via the SCMI Power Domain Management
 * Protocol.  cpucp_cluster_core_power_on() sends a synchronous
 * SCMI POWER_STATE_SET(domain_id, POWER_STATE_ON) command to the CPUCP
 * firmware, where domain_id = cluster_id << 8 | core_id.
 *
 * Returns PSCI_E_SUCCESS on success, PSCI_E_INTERN_FAIL on SCMI error.
 */
int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	uint32_t ret;

	(void)core_pos;

	VERBOSE("NCC: SCMI power-on mpidr=0x%lx\n", mpidr);

	ret = cpucp_cluster_core_power_on((uint64_t)mpidr);
	if (ret != 0U) {
		ERROR("NCC: SCMI power-on failed mpidr=0x%lx ret=%u\n",
		      mpidr, ret);
		return PSCI_E_INTERN_FAIL;
	}

	return PSCI_E_SUCCESS;
}

/*
 * plat_qti_pwr_domain_off
 *
 * Power off the calling CPU via SCMI POWER_STATE_SET(OFF).  Also increments
 * the per-cluster CL4/CL5 vote counters so that the cluster can collapse
 * when all remaining ON cores have voted.
 *
 * Note: GIC CPU-interface disable and CPUPWRCTLR programming are handled
 * by qti_pm.c (qti_node_power_off) before this call.
 */
void plat_qti_pwr_domain_off(const uint8_t *pwr_states)
{
	u_register_t mpidr = read_mpidr_el1();
	uint32_t cluster_id = find_cluster_id();
	uint32_t ret;

	(void)pwr_states;

	/*
	 * Account for this core going offline in the cluster vote counters.
	 * This allows the cluster to enter CL4/CL5 when all remaining ON
	 * cores vote for it.
	 */
	spin_lock(&ncc_pm_lock);
	++cl4_count[cluster_id];
	++cl5_count[cluster_id];
	spin_unlock(&ncc_pm_lock);

	/* Quiesce the GIC redistributor before power-off (GICv3) */
	gicv3_rdistif_off(plat_my_core_pos());

	VERBOSE("NCC: SCMI power-off mpidr=0x%lx\n", mpidr);

	/*
	 * SCMI POWER_STATE_SET(OFF) – informs CPUCP firmware that this core
	 * is powering down.  The CPUCP manages the actual rail/clock gating.
	 */
	ret = cpucp_cluster_core_power_off((uint64_t)mpidr);
	if (ret != 0U) {
		ERROR("NCC: SCMI power-off failed mpidr=0x%lx ret=%u\n",
		      mpidr, ret);
		assert(false);
	}
}

/*
 * plat_qti_pwr_domain_on_finish
 *
 * Called on the waking core after a CPU_ON power-on.  Decrements the
 * per-cluster CL4/CL5 vote counters to reflect that this core is now ON,
 * then re-initialises the GIC redistributor and CPU interface.
 *
 * Note: plat_qti_gic_cpuif_enable() is called by qti_pm.c after this
 * function returns.
 */
void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *pwr_states)
{
	uint8_t cluster_id = (uint8_t)find_cluster_id();

	(void)core_pos;
	(void)pwr_states;

	spin_lock(&ncc_pm_lock);
	--cl4_count[cluster_id];
	--cl5_count[cluster_id];
	spin_unlock(&ncc_pm_lock);

	/*
	 * Re-initialise the GIC redistributor for this core (GICv3).
	 * plat_qti_gic_cpuif_enable() is called by qti_pm.c after this
	 * function returns.
	 */
	plat_qti_gic_pcpu_init();
}

/*
 * plat_qti_pwr_domain_suspend
 *
 * Prepare the calling CPU for a power-collapse suspend.
 *
 * Steps:
 *  1. Program NCC LPM permission bits for the requested C-state.
 *  2. Increment per-cluster CL4/CL5 vote counters.
 *  3. Force RVBAR fetch on next wake so the core re-enters TF-A warm-boot.
 *  4. Enable cluster LPM when all cores in the cluster have voted.
 *  5. Send SCMI POWER_STATE_SET(OFF) to inform CPUCP of the impending
 *     power-collapse.
 *
 * Note: GIC CPU-interface disable and CPUPWRCTLR programming are handled
 * by qti_pm.c (qti_node_suspend) before this call when the core state is
 * QTI_LOCAL_STATE_OFF / DEEPOFF (i.e. STATE_C3 / STATE_C4).
 */
void plat_qti_pwr_domain_suspend(const uint8_t *pwr_states)
{
	u_register_t mpidr = read_mpidr_el1();
	uint8_t cpu_id = (uint8_t)plat_core_pos_by_mpidr(mpidr);
	uint8_t cluster_id = (uint8_t)find_cluster_id();
	uint64_t addr_offset = (uint64_t)cluster_id * NCC_CLUSTER_STRIDE;
	plat_local_state_t cpu_state =
		(plat_local_state_t)pwr_states[QTI_PWR_LVL0];
	uint32_t ret;

	VERBOSE("NCC: suspend cpu=%u cluster=%u cpu_state=%u\n",
		cpu_id, cluster_id, cpu_state);

	spin_lock(&ncc_pm_lock);

	/* Program core LPM permission bits (C3 or C4) */
	ncc_config_core_lpm(cpu_id, cpu_state, addr_offset, 1U);

	/* Track cluster-level votes */
	++cl5_count[cluster_id];
	++cl4_count[cluster_id];

	/*
	 * Force RVBAR fetch on next wake so the core re-enters TF-A
	 * warm-boot path after a full power-collapse.
	 */
	ncc_set_core_rvbar(cpu_id, cluster_id, 1U);

	/*
	 * Enable cluster LPM when all cores in the physical cluster have
	 * voted.  Prefer CL5 over CL4 when all cores have voted for CL5.
	 */
	if (cl5_count[cluster_id] == PLAT_CORE_COUNT_PER_CLUSTER)
		ncc_config_cluster_lpm(STATE_CL5, addr_offset, 1U);
	else if (cl4_count[cluster_id] == PLAT_CORE_COUNT_PER_CLUSTER)
		ncc_config_cluster_lpm(STATE_CL4, addr_offset, 1U);

	spin_unlock(&ncc_pm_lock);

	/*
	 * SCMI POWER_STATE_SET(OFF) – informs CPUCP that this core is about
	 * to enter a power-collapse.  The CPUCP manages cluster rail/clock
	 * gating based on the aggregated core votes.
	 */
	ret = cpucp_cluster_core_power_off((uint64_t)mpidr);
	if (ret != 0U)
		ERROR("NCC: SCMI suspend power-off failed mpidr=0x%lx ret=%u\n",
		      mpidr, ret);
}

/*
 * plat_qti_pwr_domain_suspend_finish
 *
 * Called on the waking core after a suspend.
 *
 * Steps:
 *  1. Clear the RVBAR-force bit so normal execution resumes.
 *  2. Tear down cluster LPM if this is the first core waking in the cluster.
 *  3. Decrement per-cluster CL4/CL5 vote counters.
 *  4. Deconfigure core LPM permission bits.
 *
 * Note: plat_qti_gic_cpuif_enable() is called by qti_pm.c after this
 * function returns when the core state was QTI_LOCAL_STATE_OFF / DEEPOFF.
 */
void plat_qti_pwr_domain_suspend_finish(const uint8_t *pwr_states)
{
	u_register_t mpidr = read_mpidr_el1();
	uint8_t cluster_id = (uint8_t)find_cluster_id();
	uint64_t addr_offset = (uint64_t)cluster_id * NCC_CLUSTER_STRIDE;
	uint8_t cpu_id = (uint8_t)(plat_core_pos_by_mpidr(mpidr) %
				   PLAT_CORE_COUNT_PER_CLUSTER);
	plat_local_state_t cpu_state =
		(plat_local_state_t)pwr_states[QTI_PWR_LVL0];

	spin_lock(&ncc_pm_lock);

	/* Clear RVBAR-force so normal execution resumes after warm-boot */
	ncc_set_core_rvbar(cpu_id, cluster_id, 0U);

	/*
	 * Tear down cluster LPM if we are the first core waking.
	 * Check before decrementing so the threshold comparison is correct.
	 */
	if (cl5_count[cluster_id] == PLAT_CORE_COUNT_PER_CLUSTER)
		ncc_config_cluster_lpm(STATE_CL5, addr_offset, 0U);
	else if (cl4_count[cluster_id] == PLAT_CORE_COUNT_PER_CLUSTER)
		ncc_config_cluster_lpm(STATE_CL4, addr_offset, 0U);

	/* Decrement vote counters now that this core is back online */
	--cl5_count[cluster_id];
	--cl4_count[cluster_id];

	/* Deconfigure core LPM permission bits */
	ncc_config_core_lpm(cpu_id, cpu_state, addr_offset, 0U);

	spin_unlock(&ncc_pm_lock);
}

/* --------------------------------------------------------------------------
 * Idle state table – consumed by qti_pm.c qti_validate_power_state()
 *
 * All NCC LPM states are exposed as PSTATE_TYPE_POWERDOWN so they are
 * routed through pwr_domain_suspend (which programs NCC LPM registers and
 * sends SCMI) rather than the simple cpu_standby WFI path.
 *
 * State ID encoding (QTI_LOCAL_PSTATE_WIDTH = 4 bits per level):
 *   bits [3:0]  = level-0 (core) state
 *   bits [7:4]  = level-1 (cluster) state
 * -------------------------------------------------------------------------- */

static const unsigned int ncc_idle_states[] = {
	/* C3 – core clock-gated, context retained */
	NCC_PSTATE(NCC_STATEID_C3,  QTI_PWR_LVL0),
	/* C4 – core power-collapsed */
	NCC_PSTATE(NCC_STATEID_C4,  QTI_PWR_LVL0),
	/* CL4 – cluster L2 power-collapsed (core C4 + cluster CL4) */
	NCC_PSTATE(NCC_STATEID_CL4, QTI_PWR_LVL1),
	/* CL5 – cluster fully off (core C4 + cluster CL5) */
	NCC_PSTATE(NCC_STATEID_CL5, QTI_PWR_LVL1),
	0U /* terminator */
};

const unsigned int *plat_qti_pm_idle_states(void)
{
	return ncc_idle_states;
}

/*
 * plat_get_target_pwr_state
 *
 * PSCI platform hook called by psci_do_state_coordination() to determine
 * the target power state for a power domain at level @lvl.
 *
 * Returns the minimum (shallowest) power state across all @ncpu entries in
 * @states.  This ensures the cluster/system only enters a deeper state when
 * every CPU in the domain has voted for it.
 */
plat_local_state_t plat_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *states,
					     unsigned int ncpu)
{
	plat_local_state_t target = PLAT_MAX_OFF_STATE;
	unsigned int i;

	(void)lvl;

	for (i = 0U; i < ncpu; i++) {
		if (states[i] < target)
			target = states[i];
	}

	return target;
}

/* --------------------------------------------------------------------------
 * PSCI initialisation
 * -------------------------------------------------------------------------- */

/*
 * plat_qti_pwr_psci_init
 *
 * Called from qti_pm.c plat_setup_psci_ops().  Initialises:
 *  - The NCC spin lock (zero-initialised by static storage; no explicit init).
 *  - The CPUCP HostLib, which sets up the SCMI transport channel to the
 *    CPUCP firmware (equivalent to scmi_power_init() in the reference).
 *  - Per-cluster CL4/CL5 vote counters.
 *
 * Vote counter pre-loading:
 *   Each counter is initialised to PLAT_CORE_COUNT_PER_CLUSTER so
 *   that a cluster can enter CL4/CL5 as soon as all currently-ON cores vote,
 *   even if some cores have never been turned on (those OFF cores are
 *   pre-counted).  Cluster 0 is the boot cluster: one core is already
 *   running, so its counter is decremented by one.
 *
 * @warm_entrypoint  BL31 warm-boot entry point address (unused here; the
 *                   CPUCP learns the entry point via the SCMI channel).
 *
 * Returns PSCI_E_SUCCESS on success.
 */
int plat_qti_pwr_psci_init(uintptr_t warm_entrypoint)
{
	uint32_t i;

	(void)warm_entrypoint;

	/*
	 * ncc_pm_lock is a static spinlock_t; C zero-initialisation sets
	 * lock = 0 (unlocked).  No explicit spin_lock_init() call is needed.
	 */

	/*
	 * Initialise the CPUCP HostLib.  This sets up the shared-memory IPC
	 * channel used by cpucp_cluster_core_power_on/off() to send SCMI
	 * POWER_STATE_SET commands to the CPUCP firmware.
	 */
	cpuss_pwron_lib_init();

	/*
	 * Pre-load vote counters.  Cores that are OFF are pre-counted so the
	 * cluster-collapse threshold is met as soon as all ON cores vote.
	 */
	for (i = 0U; i < PLAT_CLUSTER_COUNT; i++) {
		cl4_count[i] = PLAT_CORE_COUNT_PER_CLUSTER;
		cl5_count[i] = PLAT_CORE_COUNT_PER_CLUSTER;
	}

	/* Boot cluster (cluster 0) already has one core running */
	--cl4_count[0];
	--cl5_count[0];

	INFO("NCC: PSCI init complete – SCMI transport ready\n");

	return PSCI_E_SUCCESS;
}
