
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

        struct esma_channel io_channel;
        void *data;
};

struct esma *esma_engine_new_machine(struct esma_template *et, char *name);
   int esma_engine_init_machine(struct esma *esma, char *name, struct esma_template *tmpl);
  void esma_engine_del_machine(struct esma *esma);
  void esma_engine_run_machine(struct esma *esma, void *dptr);
  void esma_engine_send_msg(struct esma *src, struct esma *dst, void *dptr, u32 code);

   int esma_engine_copy_machine(struct esma *src, struct esma *dst, int (*copy_f)(const void *data_src, void *data_dst));
   int esma_engone_restart_machine(struct esma *esma);

   int esma_engine_init(char *reactor_name);
   int esma_engine_exec(void);
  void esma_engine_wait(void);

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
