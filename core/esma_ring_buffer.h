
#ifndef ESMA_RING_BUFFER_H
#define ESMA_RING_BUFFER_H

#include <pthread.h>

#include "common/numeric_types.h"
#include "common/compiler.h"

#define ESMA_RING_BUFFER_WITHOUT_LOCK	1
#define ESMA_RING_BUFFER_MUTEX		2
#define ESMA_RING_BUFFER_CMPXCHG	3

struct esma_ring_buffer {
	void *data;
	 u32  item_size;

	 u32 tail;
	 u32 head;
	 u32 mask;
	 u32 size;
	 u32 cap;

	void *(*get)(struct esma_ring_buffer *rb);
	void *(*put)(struct esma_ring_buffer *rb);

	u32 lock_type;
	union {
		pthread_mutex_t mutex;
		u32 cmpxchg;
	} lock;
};

struct esma_ring_buffer	*esma_ring_buffer_new(u32 item_size, u32 cap, u32 lock_type);
 int esma_ring_buffer_init(struct esma_ring_buffer *rb, u32 item_size, u32 cap, u32 lock_type);
void esma_ring_buffer_free(struct esma_ring_buffer *rb);

static ESMA_INLINE void *esma_ring_buffer_get(struct esma_ring_buffer *rb)
{
	return rb->get(rb);
}

static ESMA_INLINE void *esma_ring_buffer_put(struct esma_ring_buffer *rb)
{
	return rb->put(rb);
}

#endif
