
#ifndef ESMA_CPU_H
#define ESMA_CPU_H

#include "common/numeric_types.h"

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

	/* cache info */
	u32 l1_cache_line_size;
	u32 l2_cache_line_size;
	u32 l3_cache_line_size;
};

extern struct esma_cpuinfo cpuinfo;

extern u32 cpu_cacheline_size;

void esma_cpuid(void);

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
