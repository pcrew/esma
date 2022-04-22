/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_MEMPOOL_H
#define ESMA_MEMPOOL_H

#include "common/numeric_types.h"

/**
 * @brief Structure to represent memory pool.
 */
struct esma_mempool {
	void *blocks;		/**< Pointer to block chain. */
	void *basket;		/**< Pointer to used block chain. */
	 u32  block_size;	/**< Block size. */
	 u32  nblocks;		/**< Count of blocks. */

	 u8 **addr;		/**< Array of memory pages. */
	 u32 naddr;		/**< Count of memory pages. */
};

/*
 * @brief Creates a new memory pool.
 * @param [in] block_size 	Block size.
 * @return Pointer to the memory pool or NULL.
 */
struct esma_mempool *esma_mempool_new(u32 block_size);

/*
 * @brief Initialises the memory pool.
 * @param [out] mp		Pointer to memory pool.
 * @param [in] block_size	Block size in bytes.
 * @return 0 - if mempool initialized successfuly; 1 - otherwise.
 */
int esma_mempool_init(struct esma_mempool *mp, u32 block_size);

/*
 * @brief Release the memory pool.
 * @param [out] mp		Pointer to memory pool.
 */
void esma_mempool_free(struct esma_mempool *mp);

/*
 * @brief Gets block of memory.
 * @param [out] mp		Pointer to memory pool.
 * @return memory block or NULL.
 */
void *esma_mempool_get_block(struct esma_mempool *mp);

/*
 * @brief Gets several consecutive blocks of memory.
 * @param [out] mp		Pointer to memory pool.
 * @param [in] n		Count of blocks of memory.
 * @return n consecutive blocks of memory or NULL.
 */
void *esma_mempool_get_block_n(struct esma_mempool *mp, int n);

/*
 * @brief Puts block of memory in the memory pool.
 * @param [out] mp		Pointer to memory pool.
 * @param [in] n		Pointer to the memory block.
 */
int esma_mempool_put_block(struct esma_mempool *mp, void *block);

/*
 * @brief Puts several consecutive blocks of memory in the memory pool.
 * @param [out] mp		Pointer to memory pool.
 * @param [in] blocks		Pointer to the blocks of memory..
 * @param [in] n		Count of blocks of memory.
 */
int esma_mempool_put_block_n(struct esma_mempool *mp, void *block, int n);

#endif
