
#ifndef ESMA_MD5_H
#define ESMA_MD5_H

#include "common/numeric_types.h"

struct esma_md5 {
	u64 bytes;
	u32 a, b, c, d;
	u8  buf[64];
};

void esma_md5_init(struct esma_md5 *md);
void esma_md5_update(struct esma_md5 *md, const void *data, u32 size);
void esma_md5_finalize(struct esma_md5 *md, u8 res[16]);

#endif
