
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "core/esma_objpool.h"

struct test {
	int a;
	int b;
	int c;
};

int main()
{
	struct esma_objpool pool = {0};
	   int err;

	err = esma_objpool_init(&pool, 10, 0, 0);
	if (err) {
		printf("esma_objpool_init() - failed\n");
		return 0;
	}

	for (int i = 0; i < 10; i++) {
		struct test *t = esma_objpool_put(&pool);

		t->a = i;
		t->b = i + 1;
		t->c = i + 2;
	}

	struct test *p;
	struct test *t;

	t = esma_objpool_get(&pool);
	printf("t->a = %d\n", t->a);
	printf("t->b = %d\n", t->b);
	printf("t->c = %d\n\n", t->c);

	p = esma_objpool_put(&pool);
	p = t;

	t = esma_objpool_get(&pool);
	printf("t->a = %d\n", t->a);
	printf("t->b = %d\n", t->b);
	printf("t->c = %d\n\n", t->c);

	printf("The end.\n");
	return 0;
}
