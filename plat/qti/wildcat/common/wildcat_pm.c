/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>

#include <cpucp.h>
#include <qti_plat.h>

/*
 * Native direct-register CPU power-on sequence for Nord (SA8797P / Oryon
 * "NCC"), the Wildcat-architecture analogue of qti_pm_hoya.c.
 *
 * Unlike hoya, the open Nord BL31 links no QTISECLIB, so the PSCI /
 * interrupt-dispatch hooks (see qti_plat.h) are implemented natively here.
 * plat_qti_pwr_domain_on uses the raw per-cluster NCC_ARCH register sequence (QC
 * normally runs the same physical sequence from CPUCP host firmware; the
 * registers are reachable from EL3). Nord has no CPUCP clock-domain handshake
 * in EL3, so plat_qti_pwr_domain_on_finish() does not call cpucp_clkdom_init()
 * (unlike hoya). The firmware power-collapse (LPM) sequence is not implemented:
 * Nord advertises CPU standby (WFI) only, so the power-off / suspend node hooks
 * are no-ops apart from restoring the GIC on resume.
 *
 * Nord topology: 3 clusters x 6 Oryon cores. The per-cluster NCC_ARCH block is
 * at NCC base + cluster*stride; a per-core field is selected by bit position
 * (core 0..5).
 */

extern void bl31_warm_entrypoint(void);

#define NORD_NCC_ARCH_BASE		0x19880000U
#define NORD_NCC_CLUSTER_STRIDE		0x1000000U

/* Per-cluster NCC_ARCH register offsets (all accesses are 64-bit). */
#define NCC_BOOT_CFG1			0x260U	/* CPUFETCHFWBLOCK[23:16],
						 * CORRVBARMASKWAKERCVD[40:33]
						 */
#define NCC_PWR_CTL1			0x278U	/* COREPWRUPREQ[7:0] */
#define NCC_PWR_CFG1			0x290U	/* OK2ENTERC3[7:0], OK2ENTERC4[15:8] */
#define NCC_PWR_STS1			0x2B0U	/* CORSPRINITDONE[15:8], CORDOMISON[7:0] */
#define NCC_RVBAR			0x328U	/* per-cluster warm-boot vector */
#define NCC_PWR_CTL3			0x478U	/* CORFRCCPURVBAR[7:0] */

#define NCC_POWERUP_POLL		2000000U

static inline void nord_clrbits_64(uintptr_t addr, uint64_t clr)
{
	mmio_write_64(addr, mmio_read_64(addr) & ~clr);
}

/*
 * One-time NCC LPM + GIC-700 sleep-sequencer init, plus a clean NCC_RVBAR
 * starting state. Enables the per-cluster CL5 trigger and the GIC sleep
 * sequencer (GIC_SLP) that powers the per-core redistributors as their cores
 * come up. Without GIC_SLP enabled the redistributor power-up (GICR_PWRR) is
 * slow/unreliable, so secondaries come up ~1s apart and occasionally one misses
 * its window. Mirrors hoya's gold_cluster_cold_boot() one-time pattern; runs
 * before the first secondary core is powered on.
 *   CLUSTER_LPM_CTRL 0x18878000: CL5_TRIG_CTRL +0x40+4*cl, DBG_PWRUP_WAKE_CFG +0x80+4*cl.
 *   APSS_CSR_XO GICSLP_CTRL 0x17750800: DISABLE_EXT_INTF(b2)=0.
 *   GIC_SLP SEQ_CFG0 0x188A0088: SEQ_EN(b0)=1.
 */
static void nord_lpm_seq_init(void)
{
	unsigned int cl;

	/*
	 * Start each cluster's NCC with CORFRCCPURVBAR (PWR_CTL3) clear so the
	 * per-core warm-boot vectoring in plat_qti_pwr_domain_on starts from a known
	 * state.
	 */
	for (cl = 0U; cl < 3U; cl++) {
		mmio_write_64((uintptr_t)NORD_NCC_ARCH_BASE +
			      (uintptr_t)cl * NORD_NCC_CLUSTER_STRIDE +
			      NCC_PWR_CTL3, 0U);
	}
	dsbsy();

	for (cl = 0U; cl < 3U; cl++) {
		mmio_write_32((uintptr_t)0x18878000U + 0x40U + (4U * cl), 1U);
		mmio_write_32((uintptr_t)0x18878000U + 0x80U + (4U * cl), 0U);
	}
	mmio_write_32((uintptr_t)0x17750800U,
		      mmio_read_32((uintptr_t)0x17750800U) & ~0x4U);
	mmio_write_32((uintptr_t)0x188A0088U,
		      mmio_read_32((uintptr_t)0x188A0088U) | 0x1U);
	dsbsy();
	isb();
}

/*
 * Direct NCC power-up of one Oryon core. The per-cluster warm-boot vector
 * NCC_RVBAR is programmed to bl31_warm_entrypoint and CORFRCCPURVBAR[core] is
 * set BEFORE the power-up so the released core fetches from our warm entry.
 * XBL pre-powers cluster0 cores 0-4 (CORFRCCPURVBAR already SET); cores never
 * touched by XBL (cluster0 core5, all of clusters 1/2) have it CLEAR, so
 * without this they power up (CORDOMISON asserts) but fetch from the wrong
 * vector and never reach the kernel. Leave the bit set: every PSCI CPU_ON
 * warm-boots from RVBAR (cpuidle is off).
 */
static bool nord_ncc_power_on_core(uintptr_t base, unsigned int core)
{
	uint64_t m = (uint64_t)1U << core;
	unsigned int to;
	bool spr_init_done = false;
	bool dom_is_on = false;

	mmio_write_64(base + NCC_RVBAR, (uint64_t)(uintptr_t)bl31_warm_entrypoint);
	mmio_setbits_64(base + NCC_PWR_CTL3, m);	/* CORFRCCPURVBAR[core] */
	/*
	 * parked-state replicate: SET CORRVBARMASKWAKERCVD so the powerup clear
	 * below creates the 1->0 transition that vectors a cold core to RVBAR.
	 */
	mmio_setbits_64(base + NCC_BOOT_CFG1, m << 33);

	nord_clrbits_64(base + NCC_PWR_CFG1, (m << 0) | (m << 8)); /* dis C3/C4 LPM */
	mmio_setbits_64(base + NCC_BOOT_CFG1, m << 16);		/* CPUFETCHFWBLOCK */
	nord_clrbits_64(base + NCC_BOOT_CFG1, m << 33);		/* clr WAKERCVD */
	mmio_setbits_64(base + NCC_PWR_CTL1, m << 0);		/* COREPWRUPREQ */

	for (to = 0U; to < NCC_POWERUP_POLL; to++) {
		if (((mmio_read_64(base + NCC_PWR_STS1) >> 8) & m) == m) {
			spr_init_done = true;		/* CORSPRINITDONE */
			break;
		}
	}
	for (to = 0U; to < NCC_POWERUP_POLL; to++) {
		if ((mmio_read_64(base + NCC_PWR_STS1) & m) == m) {
			dom_is_on = true;		/* CORDOMISON */
			break;
		}
	}

	/*
	 * Only unblock the core's fetch once it has actually powered up. Doing so
	 * after a timed-out poll would release a partially powered core to fetch
	 * from an unexpected state; fail the CPU_ON instead so PSCI surfaces it.
	 */
	if (!spr_init_done || !dom_is_on) {
		return false;
	}

	nord_clrbits_64(base + NCC_BOOT_CFG1, m << 16);	/* unblock fetch -> RVBAR */
	return true;
}

/*
 * plat_qti_pwr_domain_on - power on a secondary Oryon core using the raw NCC_ARCH
 * reset sequence. Called from qti_cpu_power_on() (pwr_domain_on) on a kernel
 * PSCI CPU_ON. core_pos is the linear position; mpidr carries the affinity.
 */
int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	static bool lpm_seq_done;
	static spinlock_t lpm_seq_lock;
	unsigned int core = (unsigned int)((mpidr >> 8) & 0xFFU);
	unsigned int cluster = (unsigned int)((mpidr >> 16) & 0xFFU);
	uintptr_t base = (uintptr_t)NORD_NCC_ARCH_BASE +
			 (uintptr_t)cluster * (uintptr_t)NORD_NCC_CLUSTER_STRIDE;

	if ((cluster >= 3U) || (core >= 6U)) {
		return PSCI_E_INVALID_PARAMS;
	}

	/*
	 * One-time NCC LPM + GIC sleep-sequencer init before the first core.
	 * CPU_ON can run concurrently on multiple online cores, so guard the
	 * check-and-set with a lock to avoid a racing double-init or skip.
	 */
	spin_lock(&lpm_seq_lock);
	if (!lpm_seq_done) {
		nord_lpm_seq_init();
		lpm_seq_done = true;
	}
	spin_unlock(&lpm_seq_lock);

	/*
	 * Power up this core's GIC-700 redistributor before it warm-boots.
	 * gicv3_cpuif_enable() polls GICR_WAKER.ChildrenAsleep, which never
	 * clears while the redistributor is power-gated, and TF-A's GIC-600
	 * support does not drive this on the warm secondary-boot path.
	 * gicv3_rdistif_on() is the standard driver entry point for this: it
	 * looks up the redistributor base from the driver's own
	 * rdistif_base_addrs[] table (populated for every core, including
	 * not-yet-booted secondaries, at GIC driver-init time) and drives the
	 * full GIC-600/700 PWRR sequence (in-transit wait + RDPD poll), unlike
	 * the previous hand-rolled loop here which polled RDGPO instead of
	 * RDPD and skipped the in-transit check.
	 */
	gicv3_rdistif_on((unsigned int)core_pos);

	/* Power up the Oryon core and warm-boot it to bl31_warm_entrypoint. */
	if (!nord_ncc_power_on_core(base, core)) {
		ERROR("nord: cluster %u core %u power-up timed out\n",
		      cluster, core);
		return PSCI_E_INTERN_FAIL;
	}
	dsbsy();
	isb();

	/*
	 * Notify CPUCP that this core has just powered on. The native NCC_ARCH
	 * bring-up above does not otherwise tell CPUCP anything about core power
	 * state, so without this CPUCP's view of which cores are running never
	 * updates. Best-effort: the core is already up regardless of whether
	 * CPUCP acknowledges the notification.
	 */
	cpucp_notify_core_power_on(core, cluster);

	return PSCI_E_SUCCESS;
}

/*
 * plat_qti_pwr_domain_on_finish - per-core setup once it has come online, run on the
 * SECONDARY core in EL3 during its warm boot before it enters the NS kernel.
 * plat_qti_gic_pcpu_init() -> gicv3_rdistif_init() ->
 * gicv3_ppi_sgi_config_defaults() now writes IGRPMODR=0 alongside IGROUPR=~0,
 * so the arch-timer PPIs are guaranteed G1NS on any GIC reset state.
 */
void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)core_pos;
	(void)states;

	plat_qti_gic_pcpu_init();
}

/*
 * plat_qti_pm_idle_states - advertise the supported CPU idle states.
 *
 * The native Nord port implements no firmware power-collapse (LPM) sequence, so
 * a CPU cannot be physically powered down by EL3 alone. Advertise only
 * CPU-level standby (retention), entered with a simple WFI and handled by
 * qti_cpu_standby(). This matches the hoya native backend and is safe alongside
 * the OP-TEE SPD, which cannot tolerate the PSCI power-down abandon path.
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

/*
 * Nord advertises CPU standby (WFI) only and implements no firmware
 * power-collapse, so the PSCI framework never drives the power-off / suspend
 * node paths. Provide the hooks for completeness.
 */
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

	/*
	 * After a CPU/cluster power-collapse the redistributor returns at secure
	 * reset defaults. gicv3_rdistif_init() -> gicv3_ppi_sgi_config_defaults()
	 * writes IGRPMODR=0 alongside IGROUPR=~0, restoring G1NS for all PPIs.
	 */
	plat_qti_gic_pcpu_init();
}

int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	(void)warmboot_entry;
	return PSCI_E_SUCCESS;
}

void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle)
{
	(void)id;
	(void)handle;
}
