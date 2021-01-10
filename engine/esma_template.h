
#ifndef ESMA_TEMPLATE_H
#define ESMA_TEMPLATE_H

#include <stdlib.h>
#include <string.h>

#include "esma_engine_common.h" /* for __INIT__ and __FINI__ */

#include "core/esma_dbuf.h"
#include "core/esma_array.h"

#include "common/numeric_types.h"

struct esma_state_template;
struct esma_trans_template;
struct esma_channel_template;

struct esma_channel_template {
	u32 type;
	u64 data; /* for tick: interval_msec; for data: events; for sign: signal number */
};

/* filed status is must be. example: if you use trans number 0, 1 and 3 */
struct esma_trans_template {
	  char action_name[64];
	   u32 action_code;
	   u32 ch_type;
	   u32 ch_data;
	   int ch_periodic;
	struct esma_state_template *next_state;
};

struct esma_state_template {
	char name[64];

	struct esma_array trans;
	u32 ntrans;
	u32 max_code;
	u32 max_sign_code;
	u32 max_data_code;
	u32 max_tick_code;
};

struct esma_template {
	char name[64];

	struct esma_array states;
	u32 nstates;

	struct esma_array tm_channels;
	u32 ntimers;
};

struct esma_template *esma_template_new(char *name);
   int esma_template_init(struct esma_template *tmpl, char *name);
  void esma_template_free(struct esma_template *tmpl);
   int esma_template_set_by_path(struct esma_template *tmpl, char *path);
   int esma_template_set_by_dbuf(struct esma_template *tmpl, struct esma_dbuf *dbuf);

  void esma_template_print(struct esma_template *tmpl);	/* for debug */
#endif
