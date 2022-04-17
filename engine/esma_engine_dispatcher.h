/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_ENGINE_DISPATCHER_H
#define ESMA_ENGINE_DISPATCHER_H

/**@brief Send esma_message
 *  @param [in] msg The pointer to the struct esma_message.
 *  @return 0 if send succesfull, 1 - otherwise.
 */
int esma_engine_dispatcher_send(struct esma_message *msg);

#endif
