
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "esma_alloc.h"
#include "esma_logger.h"
#include "esma_mempool.h"

#include "common/compiler.h"
#include "common/macro_magic.h"

#define STRERR	strerror(errno)
#define BLOCKS	1024

struct block_small {
	struct block_small *next;
};

#define MEMPOOL_BLOCK(block)	((struct block_small *) block)

static void _esma_mempool_init_blocks(u8 *addr, u32 blocks_qty, u32 block_size)
{
	struct block_small *b;
	    u8 *a = addr;

	while (--blocks_qty) {
		b = (struct block_small *) a;
		b->next = (struct block_small *) (a + block_size);
		a += block_size;
	}

	b = (struct block_small *) a;
	b->next = NULL;
}

static int _esma_mempool_new_addr(struct esma_mempool *mp)
{
	u8 **addr;
	u8 *blocks;

	addr = esma_realloc(mp->addr, (mp->naddr + 1) * sizeof(u8 *));
	if (addr == NULL) {
		esma_core_log_err("%s() - esma_realloc(): failed\n", __func__);
		return 1;
	}

	mp->addr = addr;

	blocks = esma_malloc(BLOCKS * mp->block_size);
	if (NULL == blocks) {
		esma_core_log_err("%s() - esma_malloc(%ld bytes): failed\n",
				__func__, BLOCKS * mp->block_size);
		esma_free(addr);
		return 1;
	}

	mp->addr[mp->naddr] = blocks;
	mp->naddr++;

	mp->block_small = (struct block_small *) blocks;

	_esma_mempool_init_blocks(blocks, BLOCKS, mp->block_size);
	return 0;
}

static int _esma_mempool_new_large_block(struct esma_mempool *mp)
{
	void *b = NULL;

	b = esma_malloc(mp->block_size * BLOCKS);
	if (NULL == b)
		return 1;

	mp->block_large = b;
	mp->block_large_avail = BLOCKS;
	return 0;
}

int esma_mempool_init(struct esma_mempool *mp, u32 block_size, u32 type)
{
	block_size = max(block_size, sizeof(struct block_small));
	block_size = ALIGN(block_size, 8);

	mp->block_size = block_size;
	mp->addr = NULL;
	mp->naddr = 0;

	if (type & MEMPOOL_SMALL_BLOCK) {
		int err = _esma_mempool_new_addr(mp);
		if (err)
			goto __fail;
	}

	if (type & MEMPOOL_LARGE_BLOCK) {
		int err = _esma_mempool_new_large_block(mp);
		if (err)
			goto __fail;
	}

	return 0;

__fail:
	esma_core_log_err("%s() - failed\n", __func__);
	return 1;
}

struct esma_mempool *esma_mempool_new(u32 block_size, u32 type)
{
	struct esma_mempool *mp;
	   int err;

	mp = esma_malloc(sizeof(struct esma_mempool));
	if (NULL == mp) {
		esma_core_log_err("%s() - esma_malloc(%ld bytes): failed\n",
				__func__, sizeof(struct esma_mempool));
		return NULL;
	}

	err = esma_mempool_init(mp, block_size, type);
	if (err) {
		esma_core_log_err("%s() - esma_mempool_init(%ld, %ld): failed\n",
				__func__, block_size, type);
		esma_mempool_free(mp);
		return NULL;
	}

	return mp;
}

void *esma_mempool_get_block(struct esma_mempool *mp)
{
	struct block_small *block = mp->block_small;
	struct block_small *ret;

	if (NULL == mp)
		return NULL;

	if (NULL == mp->block_small) {
		return NULL;
	}

	if (NULL == mp->block_small) {
		int err = _esma_mempool_new_addr(mp);
		if (err) {
			return NULL;
		}
	}

	ret = block;
	mp->block_small = block->next;
	return ret;
}

static void _esma_mempool_split_large_block(struct esma_mempool *mp)
{
	struct block_small *b;
	u8 *a = mp->block_large + mp->block_large_avail * mp->block_size;
	u32 blocks_qty = mp->block_large_avail;

	while (--blocks_qty) {
		b = (struct block_small *) a;
		b->next = (struct block_small *) (a + mp->block_size);
		a += mp->block_size;
	}

	b = (struct block_small *) a;
	b->next = mp->block_small;
	mp->block_small = b;
}

void *esma_mempool_get_block_n(struct esma_mempool *mp, int n)
{
	void *block;
	 int  err;

	if (NULL == mp || n < 0)
		return NULL;

	if (n < mp->block_large_avail)
		goto __return_n_blocks;

	_esma_mempool_split_large_block(mp);
	err = _esma_mempool_new_large_block(mp);
	if (err)
		return NULL;

	goto __return_n_blocks;

__return_n_blocks:
	block = mp->block_large;
	mp->block_large += n * mp->block_size;
	mp->block_large_avail -= n;
	return block;
}

void esma_mempool_put_block(struct esma_mempool *mp, void *ptr)
{
	struct block_small *d;

	d = mp->block_small;
	mp->block_small = (struct block_small *) ptr;
	MEMPOOL_BLOCK(mp->block_small)->next = d;
}

void esma_mempool_free(struct esma_mempool *mp)
{
	int i;

	for (i = 0; i < mp->naddr; i++) {
		if (mp->addr[i])
			esma_free(mp->addr[i]);
	}

	esma_free(mp->addr);
}
