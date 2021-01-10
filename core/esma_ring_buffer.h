
#ifndef ESMA_RING_BUFFER_H
#define ESMA_RING_BUFFER_H

#include "common/numeric_types.h"
#include "common/compiler.h"

struct esma_ring_buffer {
	void *data;
	 u32  item_size;

	 u32  tail;
	 u32  head;
	 u32  mask;
	 u32  size;

	 u32  cap;
};

struct esma_ring_buffer	*esma_ring_buffer_new(u32 item_size, u32 cap);
 int  esma_ring_buffer_init(struct esma_ring_buffer *rb, u32 item_size, u32 cap);
void  esma_ring_buffer_free(struct esma_ring_buffer *rb);
void *esma_ring_buffer_get(struct esma_ring_buffer *rb);
void *esma_ring_buffer_put(struct esma_ring_buffer *rb);

#endif
