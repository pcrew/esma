
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "esma_alloc.h"
#include "esma_ring_buffer.h"

#include "common/compiler.h"
#include "common/macro_magic.h"

#define STRERR			strerror(errno)
#define RB_DEFAULT_SIZE		128

extern int errno;

int esma_ring_buffer_init(struct esma_ring_buffer *rb, u32 item_size, u32 cap)
{
	void *data;

	/* cap must be power of 2 */
	cap = (cap > 1 && (cap & (cap - 1)) == 0) ? cap : RB_DEFAULT_SIZE;

	item_size = ALIGN(item_size, 8);

	if (unlikely(NULL == rb))
		return 1;

	data = esma_calloc(item_size, cap);
	if (unlikely(NULL == rb)) {
		return 1;
	}

	rb->data = data;
	rb->item_size = item_size;

	rb->tail = 0;
	rb->head = 0xFFFFFFFF;
	rb->mask = cap - 1;
	rb->size = 0;
	rb->cap = cap;

	return 0;
}

void esma_ring_buffer_free(struct esma_ring_buffer *rb)
{
	if (unlikely(NULL == rb))
		return;

	if (rb->data) {
		free(rb->data);
	}
}

struct esma_ring_buffer *esma_ring_buffer_new(u32 item_size, u32 cap)
{
	struct esma_ring_buffer *rb = NULL;
	   int err;

	rb = esma_malloc(sizeof(struct esma_ring_buffer));
	if (unlikely(NULL == rb)) {
		return NULL;
	}

	err = esma_ring_buffer_init(rb, item_size, cap);
	if (unlikely(err)) {
		esma_free(rb);
		return NULL;
	}

	return rb;
}

void *esma_ring_buffer_get(struct esma_ring_buffer *rb)
{
	int i;

	if (unlikely(NULL == rb))
		return NULL;

	if (unlikely(0 == rb->size))
		return NULL;

	i = rb->tail;

	rb->size--;
	rb->tail++;
	rb->tail &= rb->mask;

	return rb->data + rb->item_size * i;
}

static int _esma_ring_buffer_expand(struct esma_ring_buffer *rb)
{
	void *data;
	 int  newcap = rb->cap << 1;

	data = esma_realloc(rb->data, rb->item_size * newcap);
	if (unlikely(NULL == data)) {
		return 1;
	}

	rb->data = data;
	rb->mask = newcap - 1;
	rb->cap  = newcap;
	return 0;
}

void *esma_ring_buffer_put(struct esma_ring_buffer *rb)
{
	int err;

	if (unlikely(NULL == rb))
		return NULL;

	if (rb->size == rb->cap) {
		err = _esma_ring_buffer_expand(rb);
		if (err) {
			return NULL;
		}
	}

	rb->size++;
	rb->head++;
	rb->head &= rb->mask;
	return rb->data + rb->item_size * rb->head;
}
