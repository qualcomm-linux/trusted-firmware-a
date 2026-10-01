#include "qti_cpu_id.h"


// Special Purpose System Register encodings
// (which may not be supported by compiler)
#define CPUPWRCTLR_EL1					S3_0_C15_C2_7
#define CLUSTERCFR_EL1					S3_0_C15_C3_0
#define CPUPARTSID_EL1					S3_0_C15_C2_6
#define CLUSTERIDR_EL1					S3_0_C15_C3_1
#define CLUSTEREVIDR_EL1				S3_0_C15_C3_2
#define CLUSTERACTLR_EL1				S3_0_C15_C3_3
#define CLUSTERECTLR_EL1				S3_0_C15_C3_4
#define GCR_EL1						S3_0_C1_C0_6

#define CPUPSELR_EL3					S3_6_c15_c8_0
#define CPUPCR_EL3					S3_6_c15_c8_1
#define CPUPOR_EL3					S3_6_c15_c8_2
#define CPUPMR_EL3					S3_6_c15_c8_3
//MPMM Registers
#define CPUPPMCR_EL3					S3_6_C15_C2_0
#define CPUMPMMCR_EL3					S3_6_C15_C2_1
#define CPUMPMMTUNE_EL3					S3_6_C15_C2_6

//CPTR and fields
#define CPTR_EL2					S3_4_c1_c1_2
#define CPTR_EL3					S3_6_c1_c1_2
#define CPTR_TAM_SHFT					30
#define CPTR_TAM_BITS					1
//CPTR_EL2.TZ and CPTR_EL3.EZ enable SVE for lower ELs
#define CPTR_EL3_EZ_SHFT				8
#define CPTR_EL3_EZ_BITS				1
#define CPTR_EL2_TZ_SHFT				8
#define CPTR_EL2_TZ_BITS				1
/*SME Traps - CPTR_EL3.ESM, CPTR_EL2.{SMEN, TSM} */
#define CPTR_EL3_ESM_SHFT				12
#define CPTR_EL3_ESM_BITS				1
#define CPTR_EL2_TSM_SHFT				12
#define CPTR_EL2_TSM_BITS				1

/* CPACR_EL1 and fields */
#define CPACR_EL1					S3_0_C1_C0_2
#define CPACR_EL1_SMEN_SHFT				24
#define CPACR_EL1_SMEN_BITS				2

//ERR<n>CTLR INT Enable Definitions used when accessed using ERXCTLR_EL1
#define ERRSELR_EL1					S3_0_C5_C3_1
#define ERRxCTLR_EL1					S3_0_C5_C4_1
#define ERXMISC0_EL1					S3_0_C5_C5_0
#define ERRxCTLR_CI_SHFT				13
#define ERRxCTLR_DUI_SHFT				10
#define ERRxCTLR_CFI_SHFT				8
#define ERRxCTLR_UE_SHFT				4
#define ERRxCTLR_FI_SHFT				3
#define ERRxCTLR_UI_SHFT				2
#define ERRxCTLR_ED_SHFT				0

// Field descriptions of MDCR_EL3
#define MDCR_MTPME_SHFT					28
#define MDCR_MTPME_BITS					1
#define MDCR_TDCC_SHFT					27
#define MDCR_TDCC_BITS					1
#define MDCR_NSTB_SHFT					24
#define MDCR_NSTB_BITS					2
#define MDCR_SCDD_SHFT					23
#define MDCR_SCDD_BITS					1
#define MDCR_ETAD_SHFT					22
#define MDCR_ETAD_BITS					1
#define MDCR_EPMAD_SHFT					21
#define MDCR_EPMAD_BITS					1
#define MDCR_EDAD_SHFT					20
#define MDCR_EDAD_BITS					1
#define MDCR_TTRF_SHFT					19
#define MDCR_TTRF_BITS					1
#define MDCR_STE_SHFT					18
#define MDCR_STE_BITS					1
#define MDCR_SPME_SHFT					17
#define MDCR_SPME_BITS					1
#define MDCR_SDD_SHFT					16
#define MDCR_SDD_BITS					1
#define MDCR_SPD32_SHFT					14
#define MDCR_SPD32_BITS					2
#define MDCR_TDOSA_SHFT					10
#define MDCR_TDOSA_BITS					1
#define MDCR_TDA_SHFT					9
#define MDCR_TDA_BITS					1
#define MDCR_TPM_SHFT					6
#define MDCR_TPM_BITS					1


//MPMM enable register value
#define MPMM_PINCTRL_EN					1
#define MPMM_EN						1
#define MPMM_GEAR_0						0	//medium & high bw vector and virus
#define MPMM_GEAR_1						1	//high bw vector & virus
#define MPMM_GEAR_2						2	//power virus
#define MPMM_GEAR_3						3	//do not throttle

//MPMM enable register bit position
#define MPMM_PINCTRL_SHFT				0
#define MPMM_EN_SHFT					0
#define MPMM_GEAR_SHFT					1

//MPMMTUNE register bit position
#define MPMM_G2_ATHR_SHFT				32
#define MPMM_G2_TP_SHFT					40
#define MPMM_G2_ATHR_BITS				8
#define MPMM_G2_TP_BITS					3


//MPMM enable register width
#define MPMM_PINCTRL_EN_BITS			1
#define MPMM_EN_BITS					1
#define MPMM_GEAR_BITS					2

// Field description of GCR_EL1
#define GCR_EL1_RRND					16

// SCR_EL3 and fields
#define SCR_EL3						S3_6_C1_C1_0
#define SCR_EL3_FGTEN_SHFT				27
#define SCR_EL3_FGTEN_BITS				1
#define SCR_EL3_ENTP2_SHFT				41
#define SCR_EL3_ENTP2_BITS				1

// HAFGRTR_EL2 and fields
#define HAFGRTR_EL2					S3_4_C3_C1_6
#define HAFGRTR_EL2_AMEVCNTR0_SHFT		1
#define HAFGRTR_EL2_AMEVCNTR0_BITS		4

// AMUSERENR_EL0 and fields
#define AMUSERENR_EL0					S3_3_C13_C2_3
#define AMUSERENR_EL0_EN_SHFT			0
#define AMUSERENR_EL0_EN_BITS			1

//SMC calls
#define SMC_CMD_SET_PF_CONFIG			69


/* ID_AA64PFR1_EL1 (Processor Feature Reg) definition and fields */
#define ID_AA64PFR1_EL1				S3_0_C0_C4_1
#define ID_AA64PFR1_EL1_SME_SHFT			24
#define ID_AA64PFR1_EL1_SME_BITS			4

// Helper macros for cpu core functions

#define REVIDR_EL1_BITS(X)		(1<<X)

	.macro SPR_OP spr, val, lsb, width
	MRS X0, \spr
	MOV X1, \val
	BFI X0, X1, \lsb, \width
	MSR \spr, X0
	.endm

	.macro AHB_REG_WRITE_W_READBACK addr, val, lsb, width
	// addr is supposed to be a 64bit gpr containing the address of the ahb register
	MOV X0, \addr
	LDR W1, [X0]
	MOV W4, \val
	BFI W1, W4, \lsb, \width
	STR W1, [X0]
	DMB SY
	LDR W1, [X0]
	.endm

	.macro AHB_REG_WRITE addr, val, lsb, width
	// addr is supposed to be a 64bit gpr containing the address of the ahb register
	MOV X0, \addr
	LDR W1, [X0]
	MOV W4, \val
	BFI W1, W4, \lsb, \width
	STR W1, [X0]
	.endm

	.macro BIC_REG_ARRAY dest_addr, src_addr, size
	LDR X0, =\dest_addr
	MOV W2, \size
	SUB W2, W2, #1
	ADR X4, \src_addr
	MOV W1, #0xFFFFFFFF
1:	LDR W3, [X4]
	BIC W3, W1, w3
	STR W3, [X0]
	ADD X0, X0, #4
	ADD X4, X4, #4
	SUBS W2, W2, #1
	B.PL 1b
	.endm

	.macro BIS_REG_ARRAY dest_addr, src_addr, size
	LDR X0, =\dest_addr
	MOV W2, \size
	SUB W2, W2, #1
	ADR X4, \src_addr
1:	LDR W3, [X4]
	STR W3, [X0]
	ADD X0, X0, #4
	ADD X4, X4, #4
	SUBS W2, W2, #1
	B.PL 1b
	.endm

	.macro ENABLE_CPTR_AMU_REG
	SPR_OP CPTR_EL2, #0, #CPTR_TAM_SHFT, #CPTR_TAM_BITS
	SPR_OP CPTR_EL3, #0, #CPTR_TAM_SHFT, #CPTR_TAM_BITS
	.endm

	.macro ENABLE_HAFGRTR_AMU_REG
	// Read SCR, determine if fine Grain Traps (FGT) is enabled
	MRS X0, SCR_EL3
	ubfx X1, X0, #SCR_EL3_FGTEN_SHFT, #SCR_EL3_FGTEN_BITS
	cmp X1, #1
	B.EQ amevcntr_en  // HAFGRTR can be accessed if FGT == 1
	B amevcntr_skip
	amevcntr_en:
		// Set all 4 AMEVCNTR0<x> bits to 0
		SPR_OP HAFGRTR_EL2, #0, #HAFGRTR_EL2_AMEVCNTR0_SHFT, #HAFGRTR_EL2_AMEVCNTR0_BITS
	amevcntr_skip:
	.endm

	.macro DISABLE_AMUSERENR_AMU_REG
	// Trap EL0 reads to System AMU counters
	SPR_OP AMUSERENR_EL0, #0, AMUSERENR_EL0_EN_SHFT, AMUSERENR_EL0_EN_BITS
	.endm

	.macro ENABLE_CPUECTLR_EL1_WRITES_FROM_EL2
	SPR_OP ACTLR_EL3, #1, #1, #1
	.endm

	.macro SET_DEBUG_FEATURES
	MRS X0, MDCR_EL3 // Read MDCR_EL3 into X0
	// Left to reset value
	// SPME  - reset value 0
	// STE   - reset value 0
	// MTPME - reset value 1

	// Clear bits
	MOV X1, XZR
	ORR X1, X1, #(1<<MDCR_TPM_SHFT)
	ORR X1, X1, #(1<<MDCR_TDA_SHFT)
	ORR X1, X1, #(1<<MDCR_TDOSA_SHFT)
	ORR X1, X1, #(1<<MDCR_SPD32_SHFT)
	ORR X1, X1, #(1<<MDCR_TTRF_SHFT)
	ORR X1, X1, #(1<<MDCR_TDCC_SHFT)
	BIC X0, X0, X1

	// Set bits
	ORR X0, X0, #(1<<MDCR_SDD_SHFT)
	ORR X0, X0, #(1<<MDCR_SCDD_SHFT)
	ORR X0, X0, #(3<<MDCR_NSTB_SHFT)

	MSR MDCR_EL3, X0 // Write X0 to MDCR_EL3
	.endm

	.macro CLEAR_MDCR_EPMAD
	MRS X0, MDCR_EL3 // Read MDCR_EL3 into X0
	MOV X1, #1<<21
	BIC X0, X0, X1 // Clear bit 21
	MSR MDCR_EL3, X0 // Write X0 to MDCR_EL3
	.endm

	.macro CLEAR_DEBUG_OS_LOCK
	MOV X0, #0
	MSR OSLAR_EL1, X0
	.endm

	.macro EXPORT_PMU_EVENTS val
	SPR_OP PMCR_EL0, \val, #4, #1
	.endm

	.macro ENABLE_SVE
	SPR_OP CPTR_EL3, #1, #CPTR_EL3_EZ_SHFT, #CPTR_EL3_EZ_BITS
	SPR_OP CPTR_EL2, #0, #CPTR_EL2_TZ_SHFT, #CPTR_EL2_TZ_BITS
	ISB
	SPR_OP ZCR_EL3, #0, ZCR_LEN_SHFT, #ZCR_LEN_BITS	//select Max vector length of 128 bits
	SPR_OP ZCR_EL2, #0, ZCR_LEN_SHFT, #ZCR_LEN_BITS		//set it for cases where EL2 is not doing it
	.endm

	.macro ENABLE_SVE_IF_AVAILABLE
		MRS x0, ID_AA64PFR0_EL1
		UBFX x0, x0, ID_AA64PFR0_SVE_SHIFT, ID_AA64PFR0_SVE_WIDTH //Extracting the SVE field
		CMP x0, #1
		B.NE skip_en_sve //Skip SVE enablement if not available
	en_sve:
		ENABLE_SVE
	skip_en_sve:
	.endm

	.macro CONFIG_SME
		/* Disable SME Traps at EL1, EL2, and EL3 */
		SPR_OP CPTR_EL3, #1, #CPTR_EL3_ESM_SHFT, #CPTR_EL3_ESM_BITS
		SPR_OP CPTR_EL2, #0, #CPTR_EL2_TSM_SHFT, #CPTR_EL2_TSM_BITS
		SPR_OP CPACR_EL1, #3, #CPACR_EL1_SMEN_SHFT, #CPACR_EL1_SMEN_BITS

		/* Disable trap of TPIDR2_EL0 accesses */
		SPR_OP SCR_EL3, #1, #SCR_EL3_ENTP2_SHFT, #SCR_EL3_ENTP2_BITS
	.endm

	.macro CONFIG_SME_IF_ENABLED
		/* Check if FEAT_SME is Implemented */
		MRS X0, ID_AA64PFR1_EL1
		UBFX X0, X0, #ID_AA64PFR1_EL1_SME_SHFT, #ID_AA64PFR1_EL1_SME_BITS
		CMP X0, #1
		B.NE skip_sme_cfg
		cfg_sme:
			CONFIG_SME
		skip_sme_cfg:
	.endm

	.macro PANIC_IF_MMU_ENABLED
	MRS X0, SCTLR_EL3
	AND X0, X0, #1
	CBZ X0, 99f
	B . // while(1) if MMU is enabled
99:
	.endm

	.macro ENABLE_PWR_CTL_REG_WRITES_FROM_S_EL1
	SPR_OP ACTLR_EL3, #1, #7, #1
	.endm

	.macro ENABLE_CPUECTLR_EL1_WRITES_FROM_NS_EL1
	SPR_OP ACTLR_EL2, #1, #1, #1
	.endm

	.macro ENABLE_CLUSTERTHREADSID_EL1_WRITES_FROM_EL1
	SPR_OP ACTLR_EL3, #1, #10, #1
	SPR_OP ACTLR_EL2, #1, #10, #1
	.endm

	.macro ENABLE_ACTLR_PWREN_REG
	SPR_OP ACTLR_EL3, #1, #7, #1
	SPR_OP ACTLR_EL2, #1, #7, #1
	.endm

	.macro ENABLE_ACTLR_ACTLREN_REG
	SPR_OP ACTLR_EL3, #1, #0, #1
	SPR_OP ACTLR_EL2, #1, #0, #1
	.endm

	.macro ENABLE_CLUSTERPMXXX_WRITES_FROM_EL1
	SPR_OP ACTLR_EL3, #1, #12, #1
	SPR_OP ACTLR_EL2, #1, #12, #1
	.endm


	// If MIDR version matches, jump to the given location
	.macro CHECK_MIDR_AND_JUMP midr_version, jump_location
	mrs	x0, MIDR_EL1
	ldr	w5, =MIDR_VAR_REV_MASK
	and	w0, w0, w5

	ldr	w1, =\midr_version
	cmp	w0, w1
	b.eq	\jump_location
	.endm


    .macro CHECK_MIDR_PARTNUM_AND_JUMP midr_partnum, jump_location
    mrs x0, MIDR_EL1
    ldr w5, =MIDR_PARTNUM_MASK
    and w0, w0, w5

    ldr w1, =\midr_partnum
    cmp w0, w1
    b.eq    \jump_location
    .endm

	.macro CHECK_CLUSTERIDR_AND_JUMP clusteridr_version, jump_location
	mrs	x0, CLUSTERIDR_EL1
	ldr	w5, =CLUSTERIDR_VAR_REV_MASK
	and	w0, w0, w5

	ldr	w1, =\clusteridr_version
	cmp	w0, w1
	b.eq	\jump_location
	.endm


	// If REVIDR version doesn't match, call the given function
	.macro CHECK_REVIDR_AND_CALL REGISTER, REVIDR_BIT, function, dummy_label
	mrs	x0, \REGISTER
	and	w0, w0, #REVIDR_EL1_BITS(\REVIDR_BIT)
	cmp	w0, #REVIDR_EL1_BITS(\REVIDR_BIT)
	b.eq	\dummy_label
	bl	\function
\dummy_label:
	.endm

//------------------------------------------------------------------------------
// AMU
//------------------------------------------------------------------------------
#define AMCR_EL0			S3_3_C13_C2_0
#define AMCR_HDBG_SHFT			10
#define AMCR_CG1RZ_SHFT			17
#define AMCR_CG1RZ_BITS			1

#define AMCNTENCLR0_EL0			S3_3_C13_C2_4
#define AMCNTENCLR1_EL0			S3_3_C13_C3_0
#define AMCNTENSET0_EL0			S3_3_C13_C2_5
#define AMCNTENSET1_EL0			S3_3_C13_C3_1

#define AMEVCNTR0n_EL0(n)		S3_3_C13_C4_##n  // correct with n<=7
#define AMEVCNTR1n_EL0(n)		S3_3_C13_C12_##n // correct with n<=7
#define AMEVCNTR0n_EL0_A78(n)		S3_3_C15_C4_##n  // correct with n<=3
#define AMEVCNTR1n_EL0_A78(n)		S3_3_C15_C12_##n // correct with n<=2

//Use m=7 or m=15 when accessing more than 8 counters
#define AMEVTYPER0n_EL0(m, n)		S3_3_C13_C##m##_##n //Correct with m=6 0r 7, n<=7
#define AMEVTYPER1n_EL0(m, n)		S3_3_C13_C##m##_##n //Correct with m=14 or 15, n<=7

#define AMU_BYTES_PER_CNTR		8
#define AMU_CNTRS_PER_CPU		7
#define AMU_BYTES_PER_CPU		(AMU_BYTES_PER_CNTR * AMU_CNTRS_PER_CPU)

#define AMU_BYTES_PER_CNTR_CACHE	4
#define AMU_CNTRS_PER_CPU_CACHE		4
#define AMU_BYTES_PER_CPU_CACHE		(AMU_BYTES_PER_CNTR_CACHE * AMU_CNTRS_PER_CPU_CACHE)


	.macro DISABLE_AMCR_AUX_AMU_REG
	// Disable EL0/EL1/EL2 reads to Auxilliary AMU counters
	SPR_OP AMCR_EL0, #1, #AMCR_CG1RZ_SHFT, #AMCR_CG1RZ_BITS
	.endm


/*--------------------------------------------------------
 * AMU EVENT IDs
 *--------------------------------------------------------
 */
#define L1_I_CACHE_REFILL        0x1
#define L1_D_CACHE_REFILL_EVT    0x3
#define BR_MIS_PRED              0x10
#define L2_D_CACHE_REFILL_EVT    0x17
#define BUS_ACCESS_EVT           0x19
#define INST_SPEC_EVT            0x1B
#define NCC_ENERGY_CPU_EVT       0x2C00
#define NCC_ENERGY_CPU_DYN_EVT   0x2C01

// Helper macros for cpu core functions

	.macro ENABLE_EX_LD_STR_TO_NC_MEM
	SPR_OP ACTLR_EL3, #3, #0, #2
	.endm

	.macro ENABLE_PIPED_DEV
	SPR_OP ACTLR_EL3, #1, #3, #1
	.endm

	.macro CONFIG_AUX_AMU_CNTR cnt_reg, evt_n
	SPR_OP \cnt_reg, \evt_n, #0, #16
	.endm
