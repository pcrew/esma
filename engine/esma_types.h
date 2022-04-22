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

/**
 * @brief Structure to represent esma channel.
 */
struct esma_channel {
	int fd;						/**< The file descriptor.  */
	u8 type;					/**< The type of the channel. */
	u8 hard;					/**< Channel creation method indicator. */
	int index;					/**< Index of channel in the reactor */

	/**< @brief Union contains channels information. */
	union channel_info {
		/**< @brief Structure contains timer information. */
		struct {
			u64 interval_msec;		/**< The interval in milliseconds. */
			int periodic;			/**< Pariodic or not. */
		} tick;
		/**< @brief Structure contains data info. */
		struct {
			u32 bytes_avail;		/**< Count of available bytes for reading. */
		} data;
		/**< @brief Structure cointains signal info. */
		struct {
			struct signalfd_siginfo si;	/**< Signal info. */
		} sign;
	} info;

	u32 flags;					/**< Flags. */
	u32 raw_data;					/**< Raw data for esma machine template. */

	struct esma *owner;				/**< Owner esma machine of this channel. */
};

typedef int (*action)(struct esma *src, struct esma *dst, void *dptr);


/**
 * @brief Structure to represent tansition of the esma machine.
 * @details
 * 	for Moore machine:
 *		action - always is NULL;
 *		next_state - some state.
 *
 * 	for Meely Machine:
 *		action - some action;
 *		next_state - always is NULL;
 */
struct trans {
           u32 code;					/**< Transition code. */
        action action;					/**< Action function (for Meely machine). */
        struct state *next_state;			/**< Next state (for Moore machine). */
	struct esma_channel ch;				/**< Channel. */
};

/**
 * @brief Structure to represent state of the esma machine.
 */
struct state {
	char name[64];					/**< esma machine name. */
        action enter;					/**< Enter action (can't be NULL). */
        action leave;					/**< Leave action (can be NULL). */

	struct esma_array trans;			/**< Array of transitions initiated by self. */
	struct esma_array sign_trans;			/**< Array of transitions initiated by the signal. */
	struct esma_array tick_trans;			/**< Array of transitions initiated by the timer. */

	struct trans data_pollin;			/**< Transitions initiated by the ESMA_POLLIN event. */
	struct trans data_pollout;			/**< Transitions initiated by the ESMA_POLLOUT event. */
	struct trans data_pollerr;			/**< Transitions initiated by the ESMA_POLLERR or ESMA_POLLHUP events. */
};

/**
 * @brief Structure to represent the esma machine.
 */
struct esma {
        char name[64];					/**< esma machine name. */
	 int stat;					/**< status of the esma machine (initialized or not). */

	struct esma_array states;			/**< Array of states. */
        struct state *current_state;			/**< Pointer to the current state. */

	struct esma_engine *engine;			/**< Pointer to the engine. */
        struct esma_channel io_channel;			/**< I/O channel. */
        void *data;					/**< Pointer to esma machine data. */
};

/**
 * @brief Structure to represent the esma message.
 */
struct esma_message {
	struct esma *src;				/**< Pointer to the source esma point. */
	struct esma *dst;				/**< Pointer to the destination esma point. */

	  void *ptr;					/**< Pointer to the message data. */
	   u32 code;					/**< Transition code. */
};

/**
 * @brief Structure to represent the esma reactor.
 */
struct esma_reactor {
	/**< @brief Union contains reactor information. */
	union reactor {

		/**< @brief Structure cointains epoll information. */
		struct {
			int epollfd;			/**< File descriptor. */
		} epoll;
		/**< @brief Structure cointains poll information. */
		struct {
			struct esma_array event_list;	/**< list of tracked descriptors. */
			struct esma_array channels;	/**< list of tracked esma channels. */
		} poll;
	} reactor;

	/**< @brief Structire contains reactor operations. */
	struct reactor_ops {
		/**
		 * @brief The pointer to the function that initializes the reactor.
		 * @param [out] reactor Pointer to the reactor union.
		 * @param [in] nevents Count of tracked events.
		 * @return 0 - if reactor successfuly initialized; 1 - otherwise.
		 */
		int (*init)(union reactor *reactor, u32 nevents);
		
		/**
		 * @brief The pointer to the function that release the reactor.
		 * @param [out] reactor Pointer to the reactor union.
		 * @return 0 - if reactor successfuly released; 1 - otherwise.
		 */
		void (*fini)(union reactor *reactor);

		/**
		 * @brief The pointer to the function that add fd to the reactor.
		 * @param [out] reactor	Pointer to the reactor union.
		 * @param [in] fd	File descriptor.
		 * @param [in] ch	Pointer to the channel.
		 * @return 0 - if fd successfuly added; 1 - otherwise.
		 */
		int (*add)(union reactor *reactor, int fd, struct esma_channel *ch);

		/**
		 * @brief The pointer to the function that delete fd from the reactor.
		 * @param [out] reactor	Pointer to the reactor union.
		 * @param [in] fd	File descriptor.
		 * @param [in] ch	Pointer to the channel.
		 * @return 0 - if fd successfuly deleted; 1 - otherwise.
		 */
		int (*del)(union reactor *reactor, int fd, struct esma_channel *ch);

		/**
		 * @brief The pointer to the function that modify fd handled by reactor.
		 * @param [out] reactor	Pointer to the reactor union.
		 * @param [in] fd	File descriptor.
		 * @param [in] ch	Pointer to the channel.
		 * @param [in] events	Events set mask.
		 * @return 0 - if fd successfuly modified; 1 - otherwise.
		 */
		int (*mod)(union reactor *reactor, int fd, struct esma_channel *ch, u32 events);

		/**
		 * @brief The pointer to the function that wait event in the reactor.
		 * @param [in] reactor	Pointer to the reactor union.
		 */
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

	struct msg_queue_ops {
		int  (*init)(union queue *queue, u32 flags);
		void (*fini)(union queue *queue);
		void *(*put)(union queue *queue);
		void *(*get)(union queue *queue);
	} ops;
};

/**
 * @brief Structure to represent the esma engine.
 */
struct esma_engine {
	struct esma_reactor reactor;	/**< Reactor. */
	struct esma_msg_queue queue;	/**< Queue. */
	u32 engine_id;			/**< Engine id. */
};

struct esma_state_template;
struct esma_trans_template;
struct esma_channel_template;

/**
 * @brief Structure to represent the esma channel template.
 */
struct esma_channel_template {
	u32 type;	/**< Channel type. */
	u64 data;	/**< Channel data: for tick - interval in milliseconds; for data - events flag, for signal - signo. */
};

/**
 * @brief Structure to represent the esma transiotion template.
 */
struct esma_trans_template {
	  char action_name[64];			/**< Action name. */
	   u32 action_code;			/**< Action code. */
	   u32 ch_type;				/**< Channel type. */
	   u32 ch_data;				/**< Channel data. */
	   int ch_periodic;			/**< Periodic or not. */
	struct esma_state_template *next_state;	/**< Pointer to the next state. */
};

/**
 * @brief Structure to represent the esma state template.
 * @class esma_state_template
 */
struct esma_state_template {
	char name[64];			/**< State name.  */

	struct esma_array trans;	/**< Array of transitions. */
	u32 ntrans;			/**< count of transitions. */
	u32 max_code;			/**< Max self transition code. */
	u32 max_sign_code;		/**< Max signal transition code. */
	u32 max_data_code;		/**< Max data transition code. */
	u32 max_tick_code;		/**< Max timer transition code. */
};

/**
 * @brief Structure to represent the esma template.
 */
struct esma_template {
	char name[64];			/**< Esma name. */

	struct esma_array states;	/**< Array of states. */
	u32 nstates;			/**< Count of states. */

	struct esma_array tm_channels;	/**< Array of timer channels. */
	u32 ntimers;			/**< Count of timer channels. */
};

#endif
