
#ifndef ESMA_ALLOC_H
#define ESMA_ALLOC_H

#include "common/numeric_types.h"

void *esma_malloc(size_t size);
void *esma_calloc(size_t items, size_t item_size);
void *esma_realloc(void *prev, size_t size);
void *esma_memalign(size_t alignment, size_t size);
void  esma_free(void *data);
#endif
