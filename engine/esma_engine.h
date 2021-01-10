
#ifndef ESMA_ENGINE_H
#define ESMA_ENGINE_H

#include "esma_engine_common.h"
#include "esma_template.h"
#include "esma_channel.h"

#include "core/esma_array.h"

#include "common/numeric_types.h"
#include "common/api.h"

#define IO_EVENT_ENABLE		0
#define IO_EVENT_DISABLE	1

struct esma;
struct trans;
struct state;

typedef int (*action)(struct esma *src, struct esma *dst, void *dptr);

/* for Moore machine:
 *      action - always is NULL;
 *      next_state - some state.
 *
 * for Meely Machine:
 *      action - some action;
 *      next_state - always is NULL;
 */
struct trans {
           u32 code;
        action action;
        struct state *next_state;
	struct esma_channel ch;
};

struct state {
	char name[64];
        action enter;
        action leave;

	struct esma_array trans;
	struct esma_array sign_trans;
	struct esma_array tick_trans;

	struct trans data_pollin;
	struct trans data_pollout;
	struct trans data_pollerr;
};

struct esma {
        char name[64];
	 int stat;

	struct esma_array states;
        struct state *current_state;

        u32 engine_id; /* 0 for global */

        struct esma_channel io_channel;

        void *data;
};

int esma_engine_set_number_of_engines(int cnt);

   int esma_init(struct esma *esma, char *name, struct esma_template *tmpl, u32 engine_id);
struct esma *esma_new(struct esma_template *et, char *name, u32 engine_id);
  void       esma_del(struct esma *esma);
  void       esma_run(struct esma *esma, void *dptr);
  void       esma_msg(struct esma *src, struct esma *dst, void *dptr, u32 code);
  void	     esma_stop(struct esma *esma);

 int esma_engine_init(u32 engine_id);
 int esma_engine_exec(u32 engine_id);
void esma_engine_wait(void);	/* use only in main thread */

void esma_engine_init_io_channel(struct esma *self, int fd);
void esma_engine_free_io_channel(struct esma *self);
void esma_engine_mod_io_channel(struct esma *self, u32 events, int action);

 int esma_engine_mod_channel(struct esma_channel *ch, u32 events);
 int esma_engine_arm_tick_channel(struct esma_channel *ch);
 int esma_engine_disarm_tick_channel(struct esma_channel *ch);

/* helper functions */
struct state *esma_find_state_by_name(struct esma *esma, char *name);
struct esma_channel *esma_get_channel(struct esma *esma, char *state_name, int id, u32 type);

#endif
