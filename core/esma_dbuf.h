/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_DBUF_H
#define ESMA_DBUF_H

#include "common/compiler.h"
#include "common/numeric_types.h"

/**
 * @brief Structure to represent data buffer.
 */
struct esma_dbuf {
	u8 *loc;	/**< Pointer to the buffer location. */
	u8 *pos;	/**< Pointer to the current position. */
	u32 cnt;	/**< Count of bytes in the buffer. */
	u32 len;	/**< Total len of buffer. */
};

#define esma_dbuf(str) { (u8 *) (str), (u8 *) (str), 0, sizeof(str) - 1 }

#define esma_dbuf_set(dbuf, str) 	\
	(dbuf)->loc = (u8 *) str;	\
	(dbuf)->pos = (dbuf)->loc;	\
	(dbuf)->cnt = sizeof(str) - 1;	\
	(dbuf)->len = (dbuf)->cnt;

/**
 * @brief Creates new data buffer.
 * @param [in] len	Buffer size.
 * @return Pointer to new data buffer or NULL.
 */
struct esma_dbuf *new_esma_dbuf(u32 len);

/**
 * @brief Initialises data buffer.
 * @param [out] dbuf	Pointer to the data buffer.
 * @param [in] len	Buffer size.
 * @return 0 - if memory for buffer allocated successfuly; 1 - otherwise.
 */
int esma_dbuf_init(struct esma_dbuf *dbuf, u32 len);

/**
 * @brief Expands data buffer.
 * @param [out] dbuf	Pointer to the data buffer.
 * @param [in] new_len	New Buffer size.
 * @return 0 - if memory for buffer reallocated successfuly; 1 - otherwise.
 */
int esma_dbuf_expand(struct esma_dbuf *dbuf, u32 new_len);

/**
 * @brief Releases data buffer.
 * @param [out] dbuf	Pointer to the data buffer.
 */
void esma_dbuf_free(struct esma_dbuf *dbuf);

/**
 * @brief Clears data buffer.
 * @param [out] dbuf	Pointer to the data buffer.
 * @datails function sets cnt to zero and pos to loc.
 */
static ESMA_INLINE void esma_dbuf_clear(struct esma_dbuf *dbuf)
{
	dbuf->cnt = 0;
	dbuf->pos = dbuf->loc;
}

#endif
