/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_MURMUR_HASH_H
#define ESMA_MURMUR_HASH_H

#include "common/numeric_types.h"

/**
 * @brief Calculates murmur hash.
 * @param [in] data	Pointer to data.
 * @param [in] size	Data size.
 * @return murmur hash.
 */
u32 esma_murmur_hash(u8 *data, u32 size);

#endif
