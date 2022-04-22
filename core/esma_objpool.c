/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "esma_alloc.h"
#include "esma_logger.h"
#include "esma_objpool.h"

#include "common/compiler.h"

int esma_objpool_init(struct esma_objpool *pool, u32 capacity, u32 min_size, u32 capdelta)
{
	if (unlikely(NULL == pool))
		return 1;

	pool->items = esma_malloc(capacity * sizeof(void *));
	if (unlikely(NULL == pool->items)) {
		esma_core_log_err("%s() - can't allocate memory\n", __func__);
		return 1;
	}

	pool->size = 0;
	pool->capacity = capacity;
	pool->capdelta = capdelta;
	pool->min_size = min_size;
	return 0;
}

struct esma_objpool *esma_objpool_new(u32 capacity, u32 min_size, u32 capdelta)
{
	struct esma_objpool *pool;
	   int err;

	pool = esma_malloc(sizeof(struct esma_objpool));
	if (unlikely(NULL == pool)) {
		esma_core_log_err("%s() - can't allocate memory for object poll\n", __func__);
		return NULL;
	}

	err = esma_objpool_init(pool, capacity, min_size, capdelta);
	if (unlikely(err)) {
		esma_free(pool);
		return NULL;
	}

	return pool;
}

int esma_objpool_shrink(struct esma_objpool *pool)
{
	u32 new_cap;
	void *items;

	if (unlikely(NULL == pool))
		return 1;

	if (0 == pool->capdelta)
		return 0;

	new_cap = pool->capacity > pool->capdelta ? pool->capacity - pool->capdelta : pool->capdelta;

	items = esma_realloc(pool->items, new_cap * sizeof(void *));
	if (unlikely(NULL == items)) {
		esma_core_log_err("%s() - can't reallocate memory\n", __func__);
		return 1;
	}

	pool->items = items;
	pool->capacity = new_cap;
	pool->size = pool->size > pool->capacity ? pool->capacity : pool->size;
	return 0;
}

int esma_objpool_expand(struct esma_objpool *pool)
{
	u32 new_cap;
	void *items;

	if (unlikely(NULL == pool))
		return 1;

	new_cap = pool->capdelta ? pool->capacity + pool->capdelta : pool->capacity << 1;

	items = esma_realloc(pool->items, new_cap * sizeof(void *));
	if (NULL == items) {
		esma_core_log_err("%s() - can't reallocate memory\n", __func__);
		return 1;
	}

	pool->items = items;
	pool->capacity = new_cap;
	return 0;
}

int esma_objpool_put(struct esma_objpool *pool, void *item)
{
	int err = 0;

	if (unlikely(pool->size == pool->capacity)) {
		err = esma_objpool_expand(pool);
	}

	if (unlikely(err)) {
		esma_core_log_err("%s() - can't put item\n", __func__);
		return 1;
	}

	pool->items[pool->size++] = item;
	return 0;
}

void *esma_objpool_get(struct esma_objpool *pool)
{
	void *item;

	if (0 == pool->size)
		return NULL;

	item = pool->items[--pool->size];;
	return item;
}
