
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "core/esma_mempool.h"

int main()
{
	struct esma_mempool mp = {0};
	  void *addr = NULL;
	   int err;

	err = esma_mempool_init(&mp, 32, MEMPOOL_LARGE_BLOCK);
	if (err) {
		printf("esma_mempool_init() - failed.\n");
		return 0;
	}

	printf("Block large avail: %d\n", mp.block_large_avail);
	addr = esma_mempool_get_block_n(&mp, 128);
	if (NULL == addr) {
		printf("Failed to get n blocks.\n");
		return 0;
	}
	printf("Block large avail: %d\n", mp.block_large_avail);

	esma_mempool_free(&mp);
	return 0;
}
