
#ifndef ESMA_ARRAY_H
#define ESMA_ARRAY_H

#include <stdlib.h>
#include "common/numeric_types.h"

struct esma_array {
	void *items;
	 u32 nitems;

	 u32 item_size;
	 u32 capacity;
};

#if 1
#define esma_array_n(arr, n)	\
	((n) >= (arr)->capacity) ? NULL : (arr)->items + (arr)->item_size * (n)

#else
static inline void *esma_array_n(struct esma_array *a, int n)
{
	return n >= a->capacity ? NULL : a->items + a->item_size * n;
}
#endif

static inline void *esma_array_get_tail(struct esma_array *a)
{
	return a->nitems ? a->items + a->item_size * (a->nitems - 1) : NULL;
}

static inline void *esma_array_get_head(struct esma_array *a)
{
	return a->items;
}

struct esma_array *esma_array_new(u32 capacity, u32 item_size);
   int		   esma_array_init(struct esma_array *arr, u32 capacity, u32 item_size);
  void		  *esma_array_push(struct esma_array *a);
  void		   esma_array_free(struct esma_array *a);

#endif
