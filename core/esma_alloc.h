
#ifndef ESMA_ALLOC_H
#define ESMA_ALLOC_H

#include "common/numeric_types.h"

void *esma_malloc(u32 size);
void *esma_calloc(u32 items, u32 item_size);
void *esma_realloc(void *prev, u32 size);
void *esma_memalign(u32 alignment, u32 size);
void *esma_alloca(u32 size);
void  esma_free(void *data);
#endif
