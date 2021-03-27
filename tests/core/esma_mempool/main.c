
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "core/esma_mempool.h"

struct test {
	int a;
	int b;
	int c;
};

int main()
{
	struct esma_mempool mp = {0};
	struct test *block = NULL;
	   int err;

	err = esma_mempool_init(&mp, sizeof(struct test));
	if (err) {
		printf("esma_mempool_init() - failed.\n");
		return 0;
	}

	printf("test start\n");

	srand(time(NULL));
	for (int i = 0; i < 10000000; i++) {
		block = esma_mempool_get_block(&mp);
		int a, b, c;
		if (NULL == block) {
			printf("esma_mempool_get_block() failed:\n");
			printf("\t iteration: %d\n", i);
			return 0;
		}

		a = rand() % 10;
		b = rand() % 10;
		c = rand() % 10;

		block->a = a;
		block->b = b;
		block->c = c;
	}

	printf("block->a = %d\n", block->a);
	printf("block->b = %d\n", block->b);
	printf("block->c = %d\n", block->c);
	printf("press any key to free memory\n");
	scanf("%*s");
	esma_mempool_free(&mp);

	printf("pres any key to return\n");
	scanf("%*s");
	return 0;
}
