
#ifndef COMPILER_H
#define COMPILER_H

#define ESMA_INLINE	inline __attribute__((always_inline))

#define   likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#define esma_membar() __asm__ __volatile__("" ::: "memory");

#endif
