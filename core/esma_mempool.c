/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "esma_alloc.h"
#include "esma_logger.h"
#include "esma_mempool.h"

#include "common/compiler.h"
#include "common/macro_magic.h"

#define NBLOCKS	1024

#define LOG_MSG_INVALID_ARGS()	esma_core_log_err("%s() - invalid arguments\n", __func__)

struct block {
	struct block *next;
};

static void _esma_mempool_init_blocks(u8 *addr, u32 blocks_qty, u32 block_size)
{
	struct block *b;
	u8 *a = addr;

	while (--blocks_qty) {
		b = (struct block *) a;
		b->next = (struct block *) (a + block_size);
		a += block_size;
	}

	b = (struct block *) a;
	b->next = NULL;
}

static int _esma_mempool_new_addr(struct esma_mempool *mp)
{
	u8 **addr = esma_realloc(mp->addr, (mp->naddr + 1) * sizeof(u8 *));
	u8 *blocks;

	if (unlikely(NULL == addr)) {
		esma_core_log_err("%s() - esma_realloc() failed\n", __func__);
		return 1;
	}

	mp->addr = addr;

	blocks = esma_malloc(NBLOCKS * mp->block_size);
	if (unlikely(NULL == blocks)) {
		esma_core_log_err("%s() - esma_malloc(%ld bytes) failed\n",
				__func__, NBLOCKS * mp->block_size);
		esma_free(addr);
		return 1;
	}

	mp->addr[mp->naddr] = blocks;
	mp->naddr++;

	mp->blocks = (struct block *) blocks;
	mp->nblocks = NBLOCKS;

	_esma_mempool_init_blocks(blocks, NBLOCKS, mp->block_size);
	return 0;
}

int esma_mempool_init(struct esma_mempool *mp, u32 block_size)
{
	int err;

	if (unlikely(NULL == mp)) {
		LOG_MSG_INVALID_ARGS();
		return 1;
	}

	block_size = max(block_size, sizeof(struct block));
	block_size = ALIGN(block_size, 8);

	mp->block_size = block_size;
	mp->addr = NULL;
	mp->naddr = 0;

	err = _esma_mempool_new_addr(mp);
	if (unlikely(err)) {
		esma_user_log_err("%s() - can't allocate memory for new page\n", __func__);
		goto __fail;
	}

	return 0;

__fail:
	esma_core_log_err("%s() - failed\n", __func__);
	return 1;
}

struct esma_mempool *esma_mempool_new(u32 block_size)
{
	struct esma_mempool *mp;
	   int err;

	mp = esma_malloc(sizeof(struct esma_mempool));
	if (unlikely(NULL == mp)) {
		esma_core_log_err("%s() - esma_malloc(%ld bytes): failed\n",
				__func__, sizeof(struct esma_mempool));
		return NULL;
	}

	err = esma_mempool_init(mp, block_size);
	if (unlikely(err)) {
		esma_core_log_err("%s() - esma_mempool_init(%ld): failed\n",
				__func__, block_size);
		esma_mempool_free(mp);
		return NULL;
	}

	return mp;
}

void *esma_mempool_get_block(struct esma_mempool *mp)
{
	struct block *ret;

	if (unlikely(NULL == mp)) {
		LOG_MSG_INVALID_ARGS();
		return NULL;
	}

	if (mp->basket) {
		goto __ret_basket;		
	}

	if (unlikely(NULL == mp->blocks)) {
		int err = _esma_mempool_new_addr(mp);
		if (err) {
			esma_user_log_err("%s() - can't allocate memory for new page\n", __func__);
			goto __fail;
		}
	}

	mp->nblocks--;
	ret = mp->blocks;
	mp->blocks = ret->next;
	return ret;

__ret_basket:
	ret = mp->basket;
	mp->basket = ret->next;
	return ret;

__fail:
	return NULL;
}

void *esma_mempool_get_block_n(struct esma_mempool *mp, int n)
{
	void *block;
	 int  err;

	if (unlikely(NULL == mp || n < 0)) {
		LOG_MSG_INVALID_ARGS();
		return NULL;
	}

	if (likely(n <= mp->nblocks)) {
		goto __return_n_blocks;
	}

	((struct block *) mp->blocks + mp->block_size * mp->nblocks)->next = mp->basket;
	mp->basket = mp->blocks;

	err = _esma_mempool_new_addr(mp);
	if (unlikely(err)) {
		esma_user_log_err("%s() - can't allocate memory for new page\n", __func__);
		return NULL;
	}

	goto __return_n_blocks;

__return_n_blocks:

	block = mp->blocks;
	mp->blocks = block + mp->block_size * n;
	mp->nblocks -= n;

	return block;
}

int esma_mempool_put_block(struct esma_mempool *mp, void *ptr)
{
	struct block *basket;
	struct block *tmp;

	if (unlikely(NULL == mp || NULL == ptr)) {
		LOG_MSG_INVALID_ARGS();
		return 1;
	}

	tmp = mp->basket;
	basket = ptr;

	mp->basket = basket;
	basket->next = tmp;

	return 0;
}

int esma_mempool_put_block_n(struct esma_mempool *mp, void *ptr, int n)
{
	if (unlikely(NULL == mp || NULL == ptr || n < 0)) {
		LOG_MSG_INVALID_ARGS();
		return 1;
	}

	_esma_mempool_init_blocks((u8 *) ptr, n, mp->block_size);

	((struct block *) ptr + mp->block_size * n)->next = mp->basket;
	mp->basket = ptr;	
	return 0;
}

void esma_mempool_free(struct esma_mempool *mp)
{
	for (int i = 0; i < mp->naddr; i++) {
		if (mp->addr[i])
			esma_free(mp->addr[i]);
	}

	esma_free(mp->addr);
}
