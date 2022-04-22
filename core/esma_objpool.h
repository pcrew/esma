/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_OBJPOOL_H
#define ESMA_OBJPOOL_H

#include "common/numeric_types.h"

/**
 * @brief Structure to represent object pool.
 */
struct esma_objpool {
	u32 capacity;	/**< Capacity. */
	u32 capdelta;	/**< Shrink delta. */

	void **items;	/**< Array of objects. */

	 u32  size;	/**< Current size. */
	 u32  min_size;	/**< Minimum size. */
};

/*
 * @brief Checks if the pool is empty.
 * @param [in] pool	Pointer to the object pool.
 * @return 1 if object pool is empty or 0.
 */
static inline int esma_objpool_is_empty(struct esma_objpool *pool)
{
	return pool->size ? 1 : 0;
}

/* 
 * @brief Creates a new object pool.
 * @param [in] capacity		Capacity.
 * @param [in] min_size		Minimum size.
 * @param [in] capdelta		Shrink delta.
 * @return new object pool or NULL.
 */
struct esma_objpool *esma_objpool_new(u32 capacity, u32 min_size, u32 capdelta);

/* 
 * @brief Initilises the object pool.
 * @param [out] pool		Pointer to the object pool.
 * @param [in] capacity		Capacity.
 * @param [in] min_size		Minimum size.
 * @param [in] capdelta		Shrink delta.
 * @return 0 if object pool initilises successuly; 1 - otherwise.
 */
int esma_objpool_init(struct esma_objpool *pool, u32 capacity, u32 min_size, u32 capdelta);

/* 
 * @brief Returns pointer to the object.
 * @param [in] pool		Pointer to the object pool.
 * @return Pointer to the object or NULL.
 */
void *esma_objpool_get(struct esma_objpool *pool);

/* 
 * @brief Puts object to the object_pool.
 * @param [out] pool		Pointer to the object pool.
 * @return 0 if object puted succesfuly; 1 - otherwise.
 */
int esma_objpool_put(struct esma_objpool *pool, void *item);

/* 
 * @brief Shrinks object pool..
 * @param [out] pool		Pointer to the object pool.
 * @return 0 if pool shrinked successfuly; 1 - otherwise.
 */
int esma_objpool_shrink(struct esma_objpool *pool);

/* 
 * @brief Expands object pool..
 * @param [out] pool		Pointer to the object pool.
 * @return 0 if pool expanded successfuly; 1 - otherwise.
 */
int esma_objpool_expand(struct esma_objpool *pool);

#endif
