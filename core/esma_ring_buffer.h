/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_RING_BUFFER_H
#define ESMA_RING_BUFFER_H

#include <pthread.h>

#include "common/numeric_types.h"
#include "common/compiler.h"

#define ESMA_RING_BUFFER_WITHOUT_LOCK	1
#define ESMA_RING_BUFFER_MUTEX		2
#define ESMA_RING_BUFFER_CMPXCHG	3

/**
 * @brief Structure to represent ring buffer.
 */
struct esma_ring_buffer {
	void *data;					/**< Memory. */
	 u32  item_size;				/**< Item size in bytes. */

	 u32 tail;					/**< Index of tail. */
	 u32 head;					/**< Index of head. */
	 u32 mask;					/**< Index mask. */
	 u32 size;					/**< Ring buffers size. */
	 u32 cap;					/**< Ring buffers capacity. */

	void *(*get)(struct esma_ring_buffer *rb);	/**< Pointer to the get function. */
	void *(*put)(struct esma_ring_buffer *rb);	/**< Pointer to the put function. */

	u32 lock_type;					/**< Lock type. */
	
	/**
	 * @brief Union to represent lock info.
	 */
	union {
		pthread_mutex_t mutex;			/**< Pthread mutex. */
		u32 cmpxchg;				/**< Spinlock. */
	} lock;
};

/**
 * @brief Creates a new ring buffer.
 * @param [in] item_size	Item size.
 * @param [in] cap		Capacity.
 * @param [in] lock_type	Lock type.
 * @return new ring buffer or NULL.
 */
struct esma_ring_buffer	*esma_ring_buffer_new(u32 item_size, u32 cap, u32 lock_type);

/**
 * @brief Initialises ring buffer.
 * @param [out] rb		Pointer to the ring buffer.
 * @param [in] item_size	Item size.
 * @param [in] cap		Capacity.
 * @param [in] lock_type	Lock type.
 * @return 0 if ring buffer initilized successfuly; otherwise - 1.
 */
int esma_ring_buffer_init(struct esma_ring_buffer *rb, u32 item_size, u32 cap, u32 lock_type);

/**
 * @brief Releases ring buffer.
 * @param [in] rb		Pointer to the ring buffer.
 */
void esma_ring_buffer_free(struct esma_ring_buffer *rb);

/**
 * @brief Gets item from ring buffer.
 * @param [in] rb		Pointer to the ring buffer.
 * @return memory area for read.
 */
static ESMA_INLINE void *esma_ring_buffer_get(struct esma_ring_buffer *rb)
{
	return rb->get(rb);
}

/**
 * @brief Puts item from ring buffer.
 * @param [in] rb		Pointer to the ring buffer.
 * @return memory area for write.
 */
static ESMA_INLINE void *esma_ring_buffer_put(struct esma_ring_buffer *rb)
{
	return rb->put(rb);
}

#endif
