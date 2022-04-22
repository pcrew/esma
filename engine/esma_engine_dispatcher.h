/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_ENGINE_DISPATCHER_H
#define ESMA_ENGINE_DISPATCHER_H

/**@brief Send message between two esma machines.
 *  @param [in] msg The pointer to message.
 *  @return 0 if message sent succesfull, 1 - otherwise.
 */
int esma_engine_dispatcher_send(struct esma_message *msg);

#endif
