
#ifndef ESMA_CPU_H
#define ESMA_CPU_H

#include "common/numeric_types.h"

extern u32 cpu_vendor_info[5];
extern u32 cpu_info[4];

extern u32 cpu_base_frequency;
extern u32 cpu_max_frequency;
extern u32 cpu_bus_frequency;

extern u32 cpu_cacheline_size;

extern u32 cpu_supported_sse;
extern u32 cpu_supported_sse2;
extern u32 cpu_supported_sse3;
extern u32 cpu_supported_sse4_1;
extern u32 cpu_supported_sse4_2;
extern u32 cpu_supported_ssse3;

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
