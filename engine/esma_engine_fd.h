
#ifndef ESMA_ENGINE_FD_H
#define ESMA_ENGINE_FD_H

#include "common/numeric_types.h"

int esma_engine_new_signalfd(int signo);
int esma_engine_new_timerfd(void);
int esma_engine_arm_timerfd(int timerfd, u64 interval_msec, int type);
int esma_engine_disarm_timerfd(int timerfd);

#endif
