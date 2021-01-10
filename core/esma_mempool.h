
#ifndef ESMA_MEMPOOL_H
#define ESMA_MEMPOOL_H

#include "common/numeric_types.h"

#define MEMPOOL_BASIC			0
#define MEMPOOL_WITH_LARGE_BLOCK	1

#define MEMPOOL_LARGE_BLOCK	0x01
#define MEMPOOL_SMALL_BLOCK	0x02

struct esma_mempool {
	void *block_small;
	void *block_large;
	 u32  block_large_avail;
	 u32  block_size;

	 u8 **addr;
	 u32 naddr;
};

struct esma_mempool *esma_mempool_new(u32 block_size, u32 type);
   int  esma_mempool_init(struct esma_mempool *mp, u32 block_size, u32 type);
  void  esma_mempool_free(struct esma_mempool *mp);

  void *esma_mempool_get_block(struct esma_mempool *mp);
  void *esma_mempool_get_block_n(struct esma_mempool *mp, int n);
  void  esma_mempool_put_block(struct esma_mempool *mp, void *block);

#endif
