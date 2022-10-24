/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_MACHINE_H
#define ESMA_MACHINE_H

#include "esma.h"

#include "core/esma_logger.h"

#include "common/numeric_types.h"

/**
 * @brief Send message between two esma machines.
 * @param [in] src	Pointer to the source esma machine.
 * @param [in] dst	Pointer to the destination esma machine.
 * @param [in] dptr	Pointer to the data for delivering.
 * @param [in] code	Transaction code for destination esma machine..
 * @return 0 - if message successfuly delivered; 1 - otherwise.
 */
ESMA_INLINE void esma_machine_send_msg(struct esma *src, struct esma *dst, void *dptr, u32 code)
{
	struct esma_message *msg = dst->engine->queue.ops.put(&dst->engine->queue.queue);

	if (unlikely(NULL == msg)) {
		esma_engine_log_err("%s() - can't send message from '%s' to '%s'\n", __func__, src->name, dst->name);
		exit(1);
	}

	msg->src = src;
	msg->dst = dst;
	msg->ptr = dptr;
	msg->code = code;
}

/**
 * @brief Create new esma machine.
 * @param [in] engine	Pointer to the esma engine.
 * @param [in] template	Pointer to the esma machine template.
 * @param [in] name	esma machine name.
 * @return 0 - if esma machine successfuly created; 1 - otherwise.
 */
struct esma *esma_machine_new(struct esma_engine *engine, struct esma_template *template, char *name);

/**
 * @brief Delete esma machine.
 * @param [out] esma	Pointer to the esma machine.
 * @return 0 - if esma machine successfuly deleted; 1 - otherwise.
 */
int esma_machine_del(struct esma *esma);

/**
 * @brief Run esma machine.
 * @param [out] esma	Pointer to the esma machine.
 * @return 0 - if esma machine successfuly ran; 1 - otherwise.
 */
int esma_machine_run(struct esma *esma, void *dptr);

/**
 * @brief Restart esma machine.
 * @param [out] esma	Pointer to the esma machine.
 * @details For successful restart esma machine must be in the FINI state.
 * @return 0 - if esma machine successfuly restarted; 1 - otherwise.
 */
int esma_machine_restart(struct esma *esma);

/**
 * @brief Initialize esma machine I/O channel.
 * @param [out] esma	Pointer to the esma machine.
 * @param [in] fd	File descriptor.
 * @return 0 - if esma machine I/O channel successfuly initialized; 1 - otherwise.
 */
int esma_machine_init_io_channel(struct esma *self, int fd);

/**
 * @brief Release esma machine I/O channel.
 * @param [out] esma	Pointer to the esma machine.
 * @param [in] fd	File descriptor.
 * @return 0 - if esma machine I/O channel successfuly released; 1 - otherwise.
 */
int esma_machine_free_io_channel(struct esma *self);

/**
 * @brief Modify esma machine I/O channel.
 * @param [out] esma	Pointer to the esma machine.
 * @param [in] events	Events flags.
 * @param [in] action	Action type.
 * @return 0 - if esma machine I/O channel successfuly modified; 1 - otherwise.
 */
int esma_machine_mod_io_channel(struct esma *self, u32 events, int action);

/**
 * @brief Modify esma machine channel.
 * @param [out] esma	Pointer to the esma machine.
 * @param [in] ch	Pointer to the channel.
 * @param [in] events	Events flags.
 * @return 0 - if esma machine channel successfuly modified; 1 - otherwise.
 */
int esma_machine_mod_channel(struct esma *esma, struct esma_channel *ch, u32 events);

/**
 * @brief Arm esma machine channel.
 * @param [in] ch	Pointer to the channel.
 * @return 0 - if esma machine channel successfuly armed; 1 - otherwise.
 */
int esma_machine_arm_tick_channel(struct esma_channel *ch);

/**
 * @brief Disarm esma machine channel.
 * @param [in] ch	Pointer to the channel.
 * @return 0 - if esma machine channel successfuly disarmed; 1 - otherwise.
 */
int esma_machine_disarm_tick_channel(struct esma_channel *ch);

/**
 * @brief Find esma machine state.
 * @param [in] esma	Pointer to the esma machine.
 * @param [in] name	State name.
 * @return Pointer to state if state successfuly found; NULL - otherwise.
 */
struct state *esma_machine_find_state_by_name(struct esma *esma, char *name);

/**
 * @brief Get esma machine channel.
 * @param [in] esma		Pointer to the esma machine.
 * @param [in] state_name	State name.
 * @param [in] id		Channel id.
 * @param [in] type		Channel type.
 * @return Pointer to esma channel if channel successfuly found; NULL - otherwise.
 */
struct esma_channel *esma_machine_get_channel(struct esma *esma, char *state_name, int id, u32 type);

#endif
