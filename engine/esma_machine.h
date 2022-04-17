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
#include "common/numeric_types.h"

struct esma *esma_machine_new(struct esma_engine *engine, struct esma_template *template, char *name);
int esma_machine_init(struct esma *esma, struct esma_engine *engine, struct esma_template *template, char *name);
int esma_machine_del(struct esma *esma);
int esma_machine_run(struct esma *esma, void *dptr);
void esma_machine_send_msg(struct esma *src, struct esma *dst, void *dptr, u32 code);
int esma_machine_restart(struct esma *esma);
int esma_machine_init_io_channel(struct esma *self, int fd);
int esma_machine_free_io_channel(struct esma *self);
int esma_machine_mod_io_channel(struct esma *self, u32 events, int action);
int esma_machine_mod_channel(struct esma *esma, struct esma_channel *ch, u32 events);
int esma_machine_arm_tick_channel(struct esma_channel *ch);
int esma_machine_disarm_tick_channel(struct esma_channel *ch);
struct state *esma_machine_find_state_by_name(struct esma *esma, char *name);
struct esma_channel *esma_machine_get_channel(struct esma *esma, char *state_name, int id, u32 type);

#endif
