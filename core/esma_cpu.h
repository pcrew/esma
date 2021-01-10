
#ifndef ESMA_CPU_H
#define ESMA_CPU_H

#include "common/numeric_types.h"

extern u32 cpu_vendor_info[5];
extern u32 cpu_info[4];

extern u32 cpu_cacheline_size;

extern u32 cpu_supported_sse;
extern u32 cpu_supported_sse2;
extern u32 cpu_supported_sse3;
extern u32 cpu_supported_sse4_1;
extern u32 cpu_supported_sse4_2;
extern u32 cpu_supported_ssse3;

void esma_cpuid(void);
#endif
