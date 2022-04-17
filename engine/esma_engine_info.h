/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_ENGINE_INFO
#define ESMA_ENGINE_INFO

#include "core/esma_ring_buffer.h"

struct esma_engine_info {
	   int index;
	   int status;
	struct esma_ring_buffer msg_queue;
};

#endif
