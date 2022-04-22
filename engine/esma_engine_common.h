/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_ENGINE_COMMON_H
#define ESMA_ENGINE_COMMON_H

#include "common/numeric_types.h"

#define __INIT__	0	/**< Index of init state in the array. */
#define __FINI__	1	/**< Index of fini state in the array. */

#define ESMA_POLLIN	0x01	/**< Event flag notifying about the reading. */
#define ESMA_POLLOUT	0x02	/**< Event flag notifying about the writing. */
#define ESMA_POLLHUP	0x04	/**< Event flag notifying about connection close. */
#define ESMA_POLLERR	0x08	/**< Event flag notifying about error. */

#endif
