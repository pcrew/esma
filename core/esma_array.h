/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_ARRAY_H
#define ESMA_ARRAY_H

#include <stdlib.h>
#include "common/compiler.h"
#include "common/numeric_types.h"

/**
 * @brief Structure to represent dynamic array.
 */
struct esma_array {
	void *items;	/**< Pointer to memory. */
	 u32 nitems;	/**< Current count of the items. */

	 u32 item_size;	/**< Item size. */
	 u32 capacity;	/**< Array capacity. */
};

/**
 * @brief Returns a pointer to an item of the array by index.
 * @param [in] array	Pointer to the array.
 * @param [in] index	Index of element.	
 * @return Pointer to item if index <= capacity of array; else - NULL.
 */
static ESMA_INLINE void *esma_array_n(struct esma_array *array, int index)
{
	return index >= array->capacity ? NULL : array->items + array->item_size * index;
}

/**
 * @brief Returns a pointer to the tail of the array.
 * @param [in] array	Pointer to the array.
 * @return Pointer to tail item if current count of the items in array > 0; else - NULL.
 */
static inline void *esma_array_get_tail(struct esma_array *a)
{
	return a->nitems ? a->items + a->item_size * (a->nitems - 1) : NULL;
}

/**
 * @brief Returns a pointer to the head of the array.
 * @param [in] array	Pointer to the array.
 * @return Pointer to the array memory.
 */
static inline void *esma_array_get_head(struct esma_array *a)
{
	return a->items;
}

/**
 * @brief Creates a new array.
 * @param [in] capacity		Capacity of the array.
 * @param [in] item_size	Item size.
 * @return Pointer to new array if array successfuly created; NULL - otherwise.
 */
struct esma_array *esma_array_new(u32 capacity, u32 item_size);

/**
 * @brief Initialises the array.
 * @param [in] array		Pointer to the array.
 * @param [in] capacity		Capacity of the array.
 * @param [in] item_size	Item size.
 * @return 1 - if array initialized successfuly; 0 - otherwise.
 */
int esma_array_init(struct esma_array *arr, u32 capacity, u32 item_size);

/**
 * @brief Pushs item to the array.
 * @param [in] array	Pointer to the array.
 * @return Item addr or NULL.
 */
void *esma_array_push(struct esma_array *a);

/**
 * @brief Pops array.
 * @param [in] array	Pointer to the array.
 * @param [out] to	Pointer to the memory.
 */
void esma_array_pop(struct esma_array *a, void *to);

/**
 * @brief Releases array.
 * @param [in] array	Pointer to the array.
 */
void esma_array_free(struct esma_array *a);

/**
 * @brief Copies array.
 * @param [in] src	Pointer to the source array.
 * @param [out] dst	Pointer to the destination array.
 */
void esma_array_copy(struct esma_array *src, struct esma_array *dst);

#endif
