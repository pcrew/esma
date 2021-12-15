
#ifndef ESMA_H
#define ESMA_H

#include "esma_channel.h"
#include "core/esma_array.h"
#include "common/numeric_types.h"

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

struct esma_message {
	struct esma *src;
	struct esma *dst;

	  void *ptr;
	   u32 code;
};

#include "esma_engine_dispatcher.h"
#include "esma_engine_common.h"
#include "esma_engine_info.h"
#include "esma_engine_fd.h"
#include "esma_template.h"
#include "esma_reactor.h"
#include "esma_engine.h"
#include "esma_load.h"

#endif
