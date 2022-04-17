/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_TYPES_H
#define ESMA_TYPES_H

#include <sys/signalfd.h>

#include "core/esma_array.h"
#include "core/esma_ring_buffer.h"

#include "common/api.h"
#include "common/numeric_types.h"

struct esma_channel;
struct esma_message;
struct esma_reactor;
struct esma_engine;
struct esma;
struct trans;
struct state;

struct esma_channel {
	int fd;
	u8 type;	/* ESMA_CH_XXXX */
	u8 hard;	/* If channel created by config then he is hard channel */
	int index;	/* for reactor */

	union {
		struct {
			u64 interval_msec;
			int periodic;
		} tick;
		struct {
			u32 bytes_avail;
		} data;
		struct {
			struct signalfd_siginfo si;
		} sign;
	} info;

	u32 flags;
	u32 raw_data;	/* need for templtate */

	struct esma *owner;
};

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

	struct esma_engine *engine;
        struct esma_channel io_channel;
        void *data;
};

struct esma_message {
	struct esma *src;
	struct esma *dst;

	  void *ptr;
	   u32 code;
};

struct esma_reactor {
	union reactor {
		struct {
			int epollfd;
		} epoll;

		struct {
			struct esma_array event_list;
			struct esma_array channels;
		} poll;
	} reactor;

	api_declaration(reactor_ops) {
		int (*init)(union reactor *reactor, u32 nevent, void *tools);
		void (*fini)(union reactor *reactor);

		int (*add)(union reactor *reactor, int fd, struct esma_channel *ch);
		int (*del)(union reactor *reactor, int fd, struct esma_channel *ch);
		int (*mod)(union reactor *reactor, int fd, struct esma_channel *ch, u32 events);

		void (*wait)(union reactor *reactor);
	} ops;
};

#define ESMA_QUEUE_WITHOUT_LOCK	1
#define ESMA_QUEUE_MUTEX	2
#define ESMA_QUEUE_CMPXCHG	3

struct esma_msg_queue {
	union queue {
		struct esma_ring_buffer ring_buffer;
	} queue;

	api_declaration(msg_queue_ops) {
		int  (*init)(union queue *queue, u32 flags);
		void (*fini)(union queue *queue);
		void *(*put)(union queue *queue);
		void *(*get)(union queue *queue);
	} ops;
};

struct esma_engine {
	struct esma_reactor reactor;
	struct esma_msg_queue queue;
	u32 engine_id;
};

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

#endif
