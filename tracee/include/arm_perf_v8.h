/* SPDX-License-Identifier: MIT */
/* Copyright 2013, 2020 Alexander Zuepke */
/*
 * arm_perf_v8.h
 *
 * ARM performance monitoring on ARMv8.
 *
 * azuepke, 2013-09-22: initial
 * azuepke, 2020-12-08: accessors for 64-bit ARM (cloned from arm_perf_v7.h)
 */

#ifndef ARM_PERF_V8_H_
#define ARM_PERF_V8_H_

/* bits in (PMNC) control register */
#define ARM_PERF_PMCR_NUM(pmnc)	((pmnc >> 11) & 0x1f)	/* get # of counters */
#define ARM_PERF_PMCR_DP	0x20	/* disable counter in prohibited regions */
#define ARM_PERF_PMCR_X		0x10	/* export of events to ETM */
#define ARM_PERF_PMCR_D		0x08	/* cycle counter counts every 64th cycle */
#define ARM_PERF_PMCR_C		0x04	/* cycle counter reset */
#define ARM_PERF_PMCR_P		0x02	/* event counter reset */
#define ARM_PERF_PMCR_E		0x01	/* enable all counters */

/* CCNT bit in the mask registers */
#define ARM_PERF_MASK_CCNT	0x80000000
#define ARM_PERF_MASK_ALL	0xffffffff

/* user enable bit */
#define ARM_PERF_USERENR_EN	0x00000001
#define ARM_PERF_USERENR_SW	0x00000002
#define ARM_PERF_USERENR_CR	0x00000004
#define ARM_PERF_USERENR_ER	0x00000008

/* event type upper control bits */
#define ARM_PERF_TYPER_P	0x80000000	/* Do not count events in EL1 */
#define ARM_PERF_TYPER_U	0x40000000	/* Do not count events in EL0 */
#define ARM_PERF_TYPER_NSK	0x20000000	/* Non-secure EL1 filtering (complex) */
#define ARM_PERF_TYPER_NSU	0x10000000	/* Non-secure EL0 filtering (complex) */
#define ARM_PERF_TYPER_NSH	0x08000000	/* Count events in EL2 */
#define ARM_PERF_TYPER_M	0x04000000	/* Secure EL3 filtering (complex) */
#define ARM_PERF_TYPER_MT	0x02000000	/* Multithreading */
#define ARM_PERF_TYPER_SH	0x01000000	/* Secure EL2 filtering (complex) */

/* event types */
#define ARM_PERF_EVENT_CH	0x1e	/* chain (extend counter to 64-bit) */
#define ARM_PERF_EVENT_BC	0x1d	/* bus cycle */
#define ARM_PERF_EVENT_TW	0x1c	/* TTBR writes */
#define ARM_PERF_EVENT_IS	0x1b	/* instruction speculatively executed */
#define ARM_PERF_EVENT_ME	0x1a	/* memory error */
#define ARM_PERF_EVENT_BA	0x19	/* bus access */
#define ARM_PERF_EVENT_DC2W	0x18	/* L2D cache write-back */
#define ARM_PERF_EVENT_DC2R	0x17	/* L2D cache refill */
#define ARM_PERF_EVENT_DC2A	0x16	/* L2D cache access */
#define ARM_PERF_EVENT_DC1W	0x15	/* L1D cache write-back */
#define ARM_PERF_EVENT_ICIA	0x14	/* L1I cache access */
#define ARM_PERF_EVENT_MA	0x13	/* data memory access */
#define ARM_PERF_EVENT_BP	0x12	/* predicted branch speculatively e. */
#define ARM_PERF_EVENT_CC	0x11	/* cycle */
#define ARM_PERF_EVENT_BM	0x10	/* mispredicted branch */
#define ARM_PERF_EVENT_UL	0x0f	/* unaligned load or store */
#define ARM_PERF_EVENT_BR	0x0e	/* procedure return */
#define ARM_PERF_EVENT_BI	0x0d	/* immediate branch */
#define ARM_PERF_EVENT_PW	0x0c	/* change of PC */
#define ARM_PERF_EVENT_CW	0x0b	/* write to CONTEXTIDR */
#define ARM_PERF_EVENT_ER	0x0a	/* exception return */
#define ARM_PERF_EVENT_ET	0x09	/* exception taken */
#define ARM_PERF_EVENT_IA	0x08	/* instruction executed */
#define ARM_PERF_EVENT_ST	0x07	/* store */
#define ARM_PERF_EVENT_LD	0x06	/* load */
#define ARM_PERF_EVENT_DT1C	0x05	/* L1D TLB refill */
#define ARM_PERF_EVENT_DC1A	0x04	/* L1D cache access */
#define ARM_PERF_EVENT_DC1R	0x03	/* L1D cache refill */
#define ARM_PERF_EVENT_IT1R	0x02	/* L1I TLB refill */
#define ARM_PERF_EVENT_IC1R	0x01	/* L1I cache refill */
#define ARM_PERF_EVENT_SI	0x00	/* software increment */



#ifndef __ASSEMBLER__

/* access to performance monitor registers (ARMv8) */

/** Get Performance Monitor Control Register */
static inline unsigned int arm_perf_get_ctrl(void)
{
	unsigned int val;
	__asm__ volatile ("mrs %0, PMCR_EL0" : "=r"(val));
	return val;
}

/** Set Performance Monitor Control Register */
static inline void arm_perf_set_ctrl(unsigned int val)
{
	__asm__ volatile ("msr PMCR_EL0, %0" : : "r"(val) : "memory");
}


/** Performance Monitors Count Enable Set register */
static inline void arm_perf_enable_counter(unsigned int mask)
{
	__asm__ volatile ("msr PMCNTENSET_EL0, %0" : : "r"(mask) : "memory");
}

/** Performance Monitors Count Enable Clear register */
static inline void arm_perf_disable_counter(unsigned int mask)
{
	__asm__ volatile ("msr PMCNTENCLR_EL0, %0" : : "r"(mask) : "memory");
}


/** Performance Monitor Overflow Flag Status Clear register */
static inline void arm_perf_int_ack(unsigned int mask)
{
	__asm__ volatile ("msr PMOVSCLR_EL0, %0" : : "r"(mask) : "memory");
}

/** Set USEREN (User Enable) Register */
static inline void arm_perf_set_useren(unsigned int val)
{
	__asm__ volatile ("msr PMUSERENR_EL0, %0" : : "r"(val) : "memory");
}



/** Performance Monitors Interrupt Enable Set register */
static inline void arm_perf_int_unmask(unsigned int mask)
{
	__asm__ volatile ("msr PMINTENSET_EL1, %0" : : "r"(mask) : "memory");
}

/** Performance Monitors Interrupt Enable Clear register */
static inline void arm_perf_int_mask(unsigned int mask)
{
	__asm__ volatile ("msr PMINTENCLR_EL1, %0" : : "r"(mask) : "memory");
}

////////////////////////////////////////////////////////////////////////////////

/** Performance Monitors Event Count Register 0 */
static inline unsigned int arm_perf_counter0(void)
{
	unsigned int val;
	__asm__ volatile ("mrs %0, PMEVCNTR0_EL0" : "=r"(val));
	return val;
}

/** Performance Monitors Event Count Register 1 */
static inline unsigned int arm_perf_counter1(void)
{
	unsigned int val;
	__asm__ volatile ("mrs %0, PMEVCNTR1_EL0" : "=r"(val));
	return val;
}

/** Performance Monitors Event Count Register 2 */
static inline unsigned int arm_perf_counter2(void)
{
	unsigned int val;
	__asm__ volatile ("mrs %0, PMEVCNTR2_EL0" : "=r"(val));
	return val;
}

/** Performance Monitors Event Count Register 3 */
static inline unsigned int arm_perf_counter3(void)
{
	unsigned int val;
	__asm__ volatile ("mrs %0, PMEVCNTR3_EL0" : "=r"(val));
	return val;
}

/** Performance Monitors Event Count Register 4 */
static inline unsigned int arm_perf_counter4(void)
{
	unsigned int val;
	__asm__ volatile ("mrs %0, PMEVCNTR4_EL0" : "=r"(val));
	return val;
}

/** Performance Monitors Event Count Register 5 */
static inline unsigned int arm_perf_counter5(void)
{
	unsigned int val;
	__asm__ volatile ("mrs %0, PMEVCNTR5_EL0" : "=r"(val));
	return val;
}

/** Set Performance Monitors Event Type Register 0 */
static inline void arm_perf_type0(unsigned int val)
{
	__asm__ volatile ("msr PMEVTYPER0_EL0, %0" : : "r"(val) : "memory");
}

/** Set Performance Monitors Event Type Register 1 */
static inline void arm_perf_type1(unsigned int val)
{
	__asm__ volatile ("msr PMEVTYPER1_EL0, %0" : : "r"(val) : "memory");
}

/** Set Performance Monitors Event Type Register 2 */
static inline void arm_perf_type2(unsigned int val)
{
	__asm__ volatile ("msr PMEVTYPER2_EL0, %0" : : "r"(val) : "memory");
}

/** Set Performance Monitors Event Type Register 3 */
static inline void arm_perf_type3(unsigned int val)
{
	__asm__ volatile ("msr PMEVTYPER3_EL0, %0" : : "r"(val) : "memory");
}

/** Set Performance Monitors Event Type Register 4 */
static inline void arm_perf_type4(unsigned int val)
{
	__asm__ volatile ("msr PMEVTYPER4_EL0, %0" : : "r"(val) : "memory");
}

/** Set Performance Monitors Event Type Register 5 */
static inline void arm_perf_type5(unsigned int val)
{
	__asm__ volatile ("msr PMEVTYPER5_EL0, %0" : : "r"(val) : "memory");
}

#endif

#endif
