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
#include <string.h>
#include <errno.h>

#include "esma_alloc.h"
#include "esma_logger.h"
#include "esma_ring_buffer.h"

#include "common/compiler.h"
#include "common/macro_magic.h"

#define STRERR			strerror(errno)
#define RB_DEFAULT_SIZE		128

extern int errno;

static int _init(struct esma_ring_buffer *rb, u32 item_size, u32 cap)
{
	void *data;

	/* cap must be power of 2 */
	cap = (cap > 1 && (cap & (cap - 1)) == 0) ? cap : RB_DEFAULT_SIZE;

	item_size = ALIGN(item_size, 8);

	data = esma_calloc(item_size, cap);
	if (unlikely(NULL == data)) {
		return 1;
	}

	rb->data = data;
	rb->item_size = item_size;

	rb->tail = 0;
	rb->head = ~0;
	rb->mask = cap - 1;
	rb->size = 0;
	rb->cap = cap;

	return 0;
}

static ESMA_INLINE void *_get(struct esma_ring_buffer *rb);
static ESMA_INLINE void *_put(struct esma_ring_buffer *rb);

static void *_get_mutex(struct esma_ring_buffer *rb);
static void *_put_mutex(struct esma_ring_buffer *rb);

static void *_get_cmpxchg(struct esma_ring_buffer *rb);
static void *_put_cmpxchg(struct esma_ring_buffer *rb);

int esma_ring_buffer_init(struct esma_ring_buffer *rb, u32 item_size, u32 cap, u32 lock_type)
{
	int err = 0;

	if (unlikely(NULL == rb)) {
		return 1;
	}

	switch (lock_type) {
	case ESMA_RING_BUFFER_WITHOUT_LOCK:
		rb->get = _get;
		rb->put = _put;
		break;

	case ESMA_RING_BUFFER_CMPXCHG:
		rb->get = _get_cmpxchg;
		rb->put = _put_cmpxchg;
		rb->lock.cmpxchg = 0;
		break;

	case ESMA_RING_BUFFER_MUTEX:
		rb->get = _get_mutex;
		rb->put = _put_mutex;
		err = pthread_mutex_init(&rb->lock.mutex, NULL);
		break;

	default:
		return 1;
	}

	if (unlikely(err)) {
		return 1;
	}

	rb->lock_type = lock_type;
	return _init(rb, item_size, cap);
}

struct esma_ring_buffer *esma_ring_buffer_new(u32 item_size, u32 cap, u32 lock_type)
{
	struct esma_ring_buffer *rb = NULL;
	   int err;

	rb = esma_malloc(sizeof(struct esma_ring_buffer));
	if (unlikely(NULL == rb)) {
		esma_core_log_err("%s() - can't allocate memory for ring buffer\n", __func__);
		return NULL;
	}

	err = esma_ring_buffer_init(rb, item_size, cap, lock_type);
	if (unlikely(err)) {
		esma_free(rb);
		return NULL;
	}

	return rb;
}

void esma_ring_buffer_free(struct esma_ring_buffer *rb)
{
	if (unlikely(NULL == rb))
		return;

	if (rb->data) {
		free(rb->data);
	}

	switch (rb->lock_type) {
	case ESMA_RING_BUFFER_WITHOUT_LOCK:
		break;

	case ESMA_RING_BUFFER_CMPXCHG:
		rb->lock.cmpxchg = 0;
		break;

	case ESMA_RING_BUFFER_MUTEX:
		pthread_mutex_destroy(&rb->lock.mutex);
		break;

	default:
		break;
	}
}

static int _expand(struct esma_ring_buffer *rb)
{
	 int  newcap = rb->cap << 1;
	void *data = esma_realloc(rb->data, rb->item_size * newcap);

	if (unlikely(NULL == data)) {
		esma_core_log_err("%s() - can't reallocate memory for ring buffer's items\n", __func__);
		return 1;
	}

	/* shift unread messages */
	if (rb->head != rb->cap - 1) {
		void *unread_msg = rb->data + rb->item_size * rb->head;
		u32 delta = rb->cap - rb->head;
		void *memory = unread_msg + delta;

		memmove(unread_msg, memory, delta * rb->item_size);
		rb->head += delta;
	}

	rb->data = data;
	rb->mask = newcap - 1;
	rb->cap  = newcap;
	return 0;
}

static ESMA_INLINE void *_get(struct esma_ring_buffer *rb)
{
	int i;

	if (unlikely(NULL == rb || 0 == rb->size))
		return NULL;

	i = rb->tail;

	rb->size--;
	rb->tail++;
	rb->tail &= rb->mask;
	return rb->data + rb->item_size * i;
}


static ESMA_INLINE void *_put(struct esma_ring_buffer *rb)
{
	if (unlikely(NULL == rb))
		return NULL;

	if (unlikely(rb->size == rb->cap)) {
		if (_expand(rb)) {
			return NULL;
		}
	}

	rb->size++;
	rb->head++;
	rb->head &= rb->mask;
	return rb->data + rb->item_size * rb->head;
}

static void *_get_mutex(struct esma_ring_buffer *rb)
{
	void *ret;

	pthread_mutex_lock(&rb->lock.mutex);
	ret = _get(rb);
	pthread_mutex_unlock(&rb->lock.mutex);

	return ret;
}

static void *_put_mutex(struct esma_ring_buffer *rb)
{
	void *ret;

	pthread_mutex_lock(&rb->lock.mutex);
	ret = _put(rb);
	pthread_mutex_unlock(&rb->lock.mutex);

	return ret;
}

static ESMA_INLINE u32 is_unlock(void *addr)
{
	volatile u64 ret = 0;

	__asm__ __volatile__(
		"xor %%rax, %%rax;		"
		"mov $1, %%rbx;			"
		"lock cmpxchg %%ebx, (%1);	"
		"sete (%0);			"
		: /* void */
		: "r"(&ret), "r" (addr)
		: "%rax", "%rbx"
	);

	return ret ? 1 : 0;
}

static void *_get_cmpxchg(struct esma_ring_buffer *rb)
{
	while (1) {
		if (is_unlock(&rb->lock.cmpxchg)) {
			void *ret = _get(rb);
			rb->lock.cmpxchg = 0;
			return ret;
		}

		continue;
	}
}

static void *_put_cmpxchg(struct esma_ring_buffer *rb)
{
	while (1) {
		if (is_unlock(&rb->lock.cmpxchg)) {
			void *ret = _put(rb);
			rb->lock.cmpxchg = 0;
			return ret;
		}

		continue;
	}
}
