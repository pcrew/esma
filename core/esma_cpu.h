/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_CPU_H
#define ESMA_CPU_H

#include "common/compiler.h"
#include "common/numeric_types.h"

/**
 * @brief Union to represent cache info.
 * @note see https://sandpile.org/x86/cpuid.htm#level_8000_0005h
 */
union esma_cacheinfo {
	u32 reg;				/**< CPU register. */
	/**< @brief struct to represent l1 cache */
	struct l1 {
		u8 line_size_in_bytes;		/**< Line size in bytes. */
		u8 lines_per_tag;		/**< Lines per tag. */
		u8 associativity;		/**< Associativity. */
		u8 size_in_kbs;			/**< Size in kbs. */
	} l1;
	/**< @brief struct to represent l2 cache */
	struct l2 {
		u8 line_size_in_bytes;		/**< Line size in bytes. */
		u8 lines_per_tag : 4;		/**< Lines per tag. */
		u8 associativity : 4;		/**< Associativity. */
		u16 size_in_kbs;		/**< Size in kbs. */
	} l2;
	/**< @brief struct to represent l1 cache */
	struct l3 {
		u8 line_size_in_bytes;		/**< Line size in bytes. */
		u8 lines_per_tag : 4;		/**< Lines per tag. */
		u8 associativity : 4;		/**< Associativity. */
		u16 reserved : 2;		/**< Reserved (not used). */
		u16 size_in_512kb_chunks : 14;	/**< Size in 512 kb chunck. */
	} l3;
};

/**< @brief struct to represent cpu features. */
struct esma_cpuinfo {
	int status;			/**< Status: initilized or not. */

	/* brand section */
	char brand_string[64];		/**< Brand string. */
	char vendor_string[16];		/**< Vendor string. */

	/* frequency section */
	u32 base_frq;			/**< Base frequency. */
	u32 max_frq;			/**< Max frequency. */
	u32 bus_frq;			/**< Bus frequency. */

	/* sse section */
	u8 sse;				/**< Supporting sse. */
	u8 sse2;			/**< Supporting sse2. */
	u8 sse3;			/**< Suppotting sse3. */
	u8 sse4_1;			/**< Supporting sse4_1. */
	u8 sse4_2;			/**< Supporting sse4_2. */
	u8 ssse3;			/**< Supporting ssse3. */

	/* cacheinfo */
	union esma_cacheinfo l1_data;	/**< l1 cache: data. */
	union esma_cacheinfo l1_code;	/**< l1 cache: code. */
	union esma_cacheinfo l2;	/**< l2 cache. */
	union esma_cacheinfo l3;	/**< l3 cache. */

	char mystery_string[16];	/**< Mystery string. */
};

/**
 * @brief Value of CPU info.
 */
extern struct esma_cpuinfo cpuinfo;

/**
 * @brief Value of CPU cache line size.
 */
extern u32 cpu_cacheline_size;

/**
 * @brief Get CPU info.
 */
void esma_cpuid(void);

/**
 * @brief Return current core clock frequency.
 */
u32 cpu_current_core_clock_frequency(void);

/**
 * @brief Return current core clock voltage.
 */
u32 cpu_current_core_clock_voltage(void);

/**
 * @brief Return current performance level.
 */
u32 cpu_current_preformance_level(void);

/**
 * @brief Return current gate delay.
 */
u32 cpu_current_gate_delay(void);

#if !__ARM__

/**
 * @brief Returns the number of clock cycles since the last reset.
 */
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

/**
 * @brief Returns the number of clock cycles since the last reset.
 */
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

/**
 * @brief mfence memory barrier.
 */
static ESMA_INLINE void esma_mfence()
{
	__asm__ __volatile__("mfence" ::: "memory");
}

/**
 * @brief lfence memory barrier.
 */
static ESMA_INLINE void esma_lfence()
{
	__asm__ __volatile__("lfence" ::: "memory");
}

/**
 * @brief sfence memory barrier.
 */
static ESMA_INLINE void esma_sfence()
{
	__asm__ __volatile__("sfence" ::: "memory");
}

#endif	// #if !__ARM__

#endif
