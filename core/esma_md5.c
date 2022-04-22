/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "esma_md5.h"

void esma_md5_init(struct esma_md5 *md)
{
	md->a = 0x67452301;
	md->b = 0xefcdab89;
	md->c = 0x98badcfe;
	md->d = 0x10325476;

	md->bytes = 0;
}

void esma_md5_finalize(struct esma_md5 *ctx, u8 res[16])
{
}

#define F(x, y, z)		((x) & (y)) | (~(x) & (z))
#define G(x, y, z)		((x) & (z)) | (~(z) & (y))
#define H(x, y, z)		((x) ^ (y) ^ (z))
#define I(x, y, z)		((y) ^ (~(z) | x))

#define ROTATE_LEFT(x, n)	(((x) << (n)) | ((x) >> (32 - (n))))

#define STEP(f, a, b, c, d, k, s, i)		\
	(a) += F((b), (c), (d)) + (k) + (s);	\
	(a)  = ROTATE_LEFT((a), (i));		\
	(a) += (b)
