/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_MD5_H
#define ESMA_MD5_H

#include "common/numeric_types.h"

/*
 * @brief Structure to represent md5 info. */
struct esma_md5 {
	u64 bytes;	/**< Buffer size. */
	u32 a, b, c, d; /**< Buffers initializators. */
	u8  buf[64];	/**< Buffer. */
};

/*
 * @brief Initilises md5.
 * @param [out] md	Pointer to md5.
 */
void esma_md5_init(struct esma_md5 *md);

/*
 * @brief Updates md5.
 * @param [out] md	Pointer to md5.
 * @param [in] data	Pointer to data.
 * @param [in] size	Data size.
 */
void esma_md5_update(struct esma_md5 *md, const void *data, u32 size);

/*
 * @brief Finalises md5.
 * @param [in] md	Pointer to md5.
 * @param [out] res	Pointer to result buffer.
 */
void esma_md5_finalize(struct esma_md5 *md, u8 res[16]);

#endif
