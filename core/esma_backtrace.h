/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_BACKTRACE_H
#define ESMA_BACKTRACE_H

/**
 * @brief Initialises backtrace.
 * @param depth - Depth of the stack.
 * @return 0 - if backtrace initialized successfuly; 1 - otherwise.
 */
int esma_backtrace_init(u32 depth);

#endif
