/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_ALLOC_H
#define ESMA_ALLOC_H

#include "common/numeric_types.h"

/**
 * @brief Allocates memory.
 * @param [in] size	Memory size in bytes.
 * @return Pointer to the valid memory if memory allocated successfuly or NULL.
 */
void *esma_malloc(size_t size);

/**
 * @brief Allocates memory for an array.
 * @param [in] nitems		Count of items.
 * @param [in] item_size	Item size.
 * @return Pointer to the valid memory if memory allocated successfuly or NULL.
 */
void *esma_calloc(size_t nitems, size_t item_size);

/**
 * @brief Reallocates memory.
 * @param [in] prev		Pointer to area must be reallocated.
 * @param [in] new_size		New size of area.
 * @return Pointer to the valid memory if memory reallocated successfuly or NULL.
 */
void *esma_realloc(void *prev, size_t new_size);

/**
 * @brief Allocates aligned memory.
 * @param [in] alignment	Aligment in bytes.
 * @param [in] size		Memory size in bytes.
 * @return Pointer to the valid memory if memory allocated successfuly or NULL.
 */
void *esma_memalign(size_t alignment, size_t size);

/**
 * @brief Frees memory.
 * @param [in] data		Pointer to the memory.
 */
void esma_free(void *data);
#endif
