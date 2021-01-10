
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "core/esma_cpu.h"

int main()
{
	printf("cpu_cacheline_size:   %d\n", cpu_cacheline_size);
	printf("cpu_supported_sse:    %s\n", cpu_supported_sse     == 1 ? "yes" : "no");
	printf("cpu_supported_sse2:   %s\n", cpu_supported_sse2    == 1 ? "yes" : "no");
	printf("cpu_supported_sse3:   %s\n", cpu_supported_sse3    == 1 ? "yes" : "no");
	printf("cpu_supported_ssse3:  %s\n", cpu_supported_ssse3   == 1 ? "yes" : "no");
	printf("cpu_supported_sse4_1:  %s\n", cpu_supported_sse4_1 == 1 ? "yes" : "no");
	printf("cpu_supported_sse4_2:  %s\n", cpu_supported_sse4_2 == 1 ? "yes" : "no");
	return 0;
}
