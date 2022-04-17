/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_ENGINE_FD_H
#define ESMA_ENGINE_FD_H

#include "common/numeric_types.h"

/**
 * @brief Create new signal fd.
 * @param [in] signo	Signal number.
 * @return new signal fd if fd successfuly created; -1 - otherwise.
 */
int esma_engine_new_signalfd(int signo);

/**
 * @brief Create new timerfd fd.
 * @return new timer fd if fd successfuly created; -1 - otherwise.
 */
int esma_engine_new_timerfd(void);

/**
 * @brief arm timer fd.
 * @param [in] timerfd		Timer fd.
 * @param [in] interval_msec	Interval in milliseconds.
 * @param [in] type		Type.
 * @return 0 if timer fd successfuly armed; -1 - otherwise.
 */
int esma_engine_arm_timerfd(int timerfd, u64 interval_msec, int type);

/**
 * @brief disarm timer fd.
 * @param [in] timerfd		Timer fd.
 * @return 0 if timer fd successfuly disarmed; -1 - otherwise.
 */
int esma_engine_disarm_timerfd(int timerfd);

#endif
