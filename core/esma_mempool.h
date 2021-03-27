
#ifndef ESMA_MEMPOOL_H
#define ESMA_MEMPOOL_H

#include "common/numeric_types.h"

struct esma_mempool {
	void *blocks;
	void *basket;
	 u32  block_size;
	 u32  nblocks;

	 u8 **addr;
	 u32 naddr;
};

struct esma_mempool *esma_mempool_new(u32 block_size);
   int  esma_mempool_init(struct esma_mempool *mp, u32 block_size);
  void  esma_mempool_free(struct esma_mempool *mp);

  void *esma_mempool_get_block(struct esma_mempool *mp);
  void *esma_mempool_get_block_n(struct esma_mempool *mp, int n);
   int  esma_mempool_put_block(struct esma_mempool *mp, void *block);
   int  esma_mempool_put_block_n(struct esma_mempool *mp, void *block, int n);

#endif
