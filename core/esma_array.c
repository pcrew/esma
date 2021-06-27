
#include <string.h>
#include <stdlib.h>
#include <stdio.h> /* for debug */

#include "esma_array.h"
#include "esma_alloc.h"
#include "esma_logger.h"

#include "common/compiler.h"

int esma_array_init(struct esma_array *arr, u32 capacity, u32 item_size)
{
	void *items;

	items = esma_calloc(capacity, item_size);
	if (NULL == items) {
		esma_core_log_err("%s() - esma_calloc(%ld, %ld): failed\n", __func__, capacity, item_size);
		return 1;
	}

	arr->items = items;
	arr->nitems = 0;
	arr->item_size = item_size;
	arr->capacity = capacity;

	return 0;
}

struct esma_array *esma_array_new(u32 capacity, u32 item_size)
{
	struct esma_array *arr = NULL;
	   int err;

	arr = esma_malloc(sizeof(struct esma_array));
	if (unlikely(NULL == arr)) {
		esma_core_log_err("%s() - esma_malloc(%d bytes): failed\n", __func__, sizeof(struct esma_array));
		return NULL;
	}

	err = esma_array_init(arr, capacity, item_size);
	if (unlikely(err)) {
		esma_core_log_err("%s() - esma_array_init(%ld, %ld): failed\n", __func__, capacity, item_size);
		esma_free(arr);
		return NULL;
	}

	return arr;
}

static int _esma_array_expand(struct esma_array *arr)
{
	void *items = NULL;
	 u32  new_cap;

	new_cap = arr->capacity << 1;

	items = esma_realloc(arr->items, new_cap * arr->item_size);
	if (unlikely(NULL == items)) {
		esma_core_log_err("%s() - esma_realloc(%ld): failed; prev size: %ld\n",
				__func__, new_cap * arr->item_size, arr->capacity* arr->item_size);
		return 1;
	}

	arr->items = items;
	arr->capacity = new_cap;
	return 0;
}

void *esma_array_push(struct esma_array *arr)
{
	void *item;
	 int  err;

	if (unlikely(NULL == arr))
		return NULL;

	if (likely(arr->nitems < arr->capacity))
		goto __push;

	err = _esma_array_expand(arr);
	if (err)
		return NULL;

	goto __push;

__push:
	item = arr->items + arr->nitems * arr->item_size;
	arr->nitems++;
	return item;
}

void *esma_array_pop(struct esma_array *arr)
{
	  if (unlikely(NULL == arr))
		  return NULL;

	  if (0 == arr->nitems)
		  return NULL;

	  return arr->items + (--arr->nitems) * arr->item_size;
}

void esma_array_free(struct esma_array *arr)
{
	if (unlikely(NULL == arr))
		return;

	if (arr->capacity && arr->items)
		esma_free(arr->items);

	return;
}
