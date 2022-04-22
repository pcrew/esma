/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "esma_alloc.h"
#include "esma_logger.h"
#include "common/compiler.h"

extern int errno;

void *esma_malloc(size_t size)
{
	void *ret = malloc(size);

	if (unlikely(NULL == ret)) {
		esma_core_log_sys("malloc('%lld' bytes) failed: %s\n", size, strerror(errno));
		return 0;
	}

	return ret;
}

void *esma_calloc(size_t items, size_t item_size)
{
	void *data = NULL;

	data = esma_malloc(items * item_size);
	if (unlikely(NULL == data)) {
		return NULL;
	}

	memset(data, 0, items * item_size);
	return data;
}

void *esma_realloc(void *prev, size_t size)
{
	void *data = realloc(prev, size);

	if (unlikely(NULL == data)) {
		esma_core_log_sys("realloc(%p,'%lld' bytes) failed: %s\n", prev, size, strerror(errno));
		return NULL;
	}

	return data;
}

void *esma_memalign(size_t alignment, size_t size)
{
	void *data = NULL;
	 int  err;

	err = posix_memalign(&data, alignment, size);
	if (unlikely(err)) {
		esma_core_log_sys("posix_memalign(%p, aligment '%lld', '%lld' bytes) failed: %s\n",
				data, alignment, size, strerror(errno));
		return NULL;
	}

	return data;
}

void esma_free(void *data)
{
	free(data);
}
