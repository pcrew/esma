
#ifndef ESMA_OBJPOOL_H
#define ESMA_OBJPOOL_H

#include "common/numeric_types.h"

struct esma_objpool {
	u32 capacity;
	u32 capdelta;

	void **items;

	 u32  size;
	 u32  min_size;
};

struct esma_objpool *esma_objpool_new(u32 capacity, u32 min_size, u32 capdelta);
int esma_objpool_init(struct esma_objpool *pool, u32 capacity, u32 min_size, u32 capdelta);

void *esma_objpool_get(struct esma_objpool *pool);
 int  esma_objpool_put(struct esma_objpool *pool, void *item);

 int esma_objpool_shrink(struct esma_objpool *pool);
 int esma_objpool_expand(struct esma_objpool *pool);

#endif
