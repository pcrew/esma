
#ifndef ESMA_CPU_H
#define ESMA_CPU_H

#include "common/numeric_types.h"

/*
 * see: https://sandpile.org/x86/cpuid.htm#level_8000_0005h
 */

union esma_cacheinfo {
	u32 reg;
	struct {
		u8 line_size_in_bytes;
		u8 lines_per_tag;
		u8 associativity;
		u8 size_in_kbs;
	} l1;
	struct {
		u8 line_size_in_bytes;
		u8 lines_per_tag : 4;
		u8 associativity : 4;
		u16 size_in_kbs;
	} l2;

	struct {
		u8 line_size_in_bytes;
		u8 lines_per_tag : 4;
		u8 associativity : 4;
		u16 reserved : 2;
		u16 size_in_512kb_chunks : 14;
	} l3;
};

struct esma_cpuinfo {
	/* brand section */
	char brand_string[64];
	char vendor_string[16];

	/* frequency section */
	u32 base_frq;
	u32 max_frq;
	u32 bus_frq;

	/* sse section */
	u8 sse;
	u8 sse2;
	u8 sse3;
	u8 sse4_1;
	u8 sse4_2;
	u8 ssse3;

	/* cacheinfo */
	union esma_cacheinfo l1_data;
	union esma_cacheinfo l1_code;
	union esma_cacheinfo l2;
	union esma_cacheinfo l3;

	char mystery_string[16];
};

extern struct esma_cpuinfo cpuinfo;

extern u32 cpu_cacheline_size;

void esma_cpuid(void);

u32 cpu_current_core_clock_frequency(void);
u32 cpu_current_core_clock_voltage(void);
u32 cpu_current_preformance_level(void);
u32 cpu_current_gate_delay(void);

#if !__ARM__
static inline u64 esma_rdtsc(void)
{
	u64 ret;

	__asm__ __volatile__(
		"rdtsc;			"
		"shl $32, %%rdx;	"
		"or %%rdx, %0		"
		: "=a" (ret)
		: /* void */
		: "%rdx"
	);

	return ret;
}

static inline u64 esma_rdtscp(void)
{
	u64 ret;

	__asm__ __volatile__(
		"rdtscp;		"
		"shl $32, %%rdx;	"
		"or %%rdx, %0		"
		: "=a" (ret)
		: /* void */
		: "%rdx"
	);

	return ret;
}

#define esma_mfence() __asm__ __volatile__("mfence" ::: "memory");
#define esma_lfence() __asm__ __volatile__("lfence" ::: "memory");
#define esma_sfence() __asm__ __volatile__("sfence" ::: "memory");

#endif	// #if !__ARM__

#endif
