
#ifndef ESMA_REACTOR_H
#define ESMA_REACTOR_H

#include "common/numeric_types.h"
#include "common/api.h"

api_declaration(reactor) {

	void (*init)(u32 nevent, void *tools);
	void (*fini)(void);

	 int (*add)(int fd, void *dptr);
	 int (*del)(int fd);
	 int (*mod)(int fd, void *dptr, u32 events);

	 /* signal section */
	 int (*new_sig)(int sig);	/* returned new sigfd or -1 */

	 /* timers section */
	 int    (*new_timerfd)(void);	/* returned new timerfd or -1 */
	 int    (*arm_timerfd)(int timerfd, int time_interval, int type);
	 int (*disarm_timerfd)(int timerfd);

	void (*wait)();
};

extern struct reactor *reactor;

#endif
