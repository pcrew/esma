
#include "esma_alloc.h"

#include <stdio.h> /* for debug */
#include <stdlib.h>
#include <string.h>

#include "common/compiler.h"

void *esma_malloc(u32 size)
{
	return malloc(size);
}

void *esma_calloc(u32 items, u32 item_size)
{
	void *data = NULL;

	data = esma_malloc(items * item_size);
	if (unlikely(NULL == data)) {
		return NULL;
	}

	memset(data, 0, items * item_size);
	return data;
}

void *esma_realloc(void *prev, u32 size)
{
	void *data = realloc(prev, size);

	if (unlikely(NULL == data))
		return NULL;

	return data;
}

void *esma_memalign(u32 alignment, u32 size)
{
	void *data = NULL;
	 int  err;

	err = posix_memalign(&data, alignment, size);
	if (err) {
		return NULL;
	}

	return data;
}

void esma_free(void *data)
{
	free(data);
}
