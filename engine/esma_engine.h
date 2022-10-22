/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_ENGINE_H
#define ESMA_ENGINE_H

#include "esma.h"

#include "common/numeric_types.h"
#include "common/api.h"

/**
 * @brief Initialize esma_engine.
 * @param [in] engine		Pointer to the engine.
 * @param [in] reactor_name	Pointer to the reactor name.
 * @param [in] msg_queue_name	Pointer to the message queue name.
 * @param [in] lock type	Lock type.
 * @return 0 - if esma_engine successfuly initialized; 1 - otherwise.
 */
int esma_engine_init(struct esma_engine *engine, char *reactor_name, char *msg_queue_name, u32 lock_type);

/**
 * @brief execute esma_engine.
 * @param [in] engine		Pointer to the engine.
 * @details The function handles messages that are in the esma_msg_queue.
 * @return 0 - if esma_engine successfuly executed; 1 - otherwise.
 */
int esma_engine_exec(struct esma_engine *engine);

/**
 * @brief Waiting for a new events in the esma_engine.
 * @param [in] engine		Pointer to the engine.
 * @details The function wait in the I/O multiplexer.
 * @return 0 - if multiplexer return 0; 1 - otherwise.
 */
void esma_engine_wait(struct esma_engine *engine);

#endif
