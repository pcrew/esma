/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <execinfo.h>

#include "esma_alloc.h"
#include "esma_logger.h"
#include "esma_backtrace.h"

#include "common/numeric_types.h"

extern int errno;

static u32 esma_backtrace_depth;
void *esma_backtrace_buff = NULL;

static void _esma_backtrace_print_stack(void)
{
	char **strings;
	int n;

	n = backtrace(esma_backtrace_buff, esma_backtrace_depth);
	strings = backtrace_symbols(esma_backtrace_buff, n);
	if (NULL == strings) {
		esma_core_log_sys("%s() - backtrace_symbols() failed: %s\n", __func__, strerror(errno));
		exit(1);
	}

	esma_core_log_inf("Backtrace (most recent call first):\n", "");
	for (int i = 0; i < n; i++) {
		esma_core_log_inf("%d: %s\n", i, strings[i]);
	}

	free(strings);
}

static void _esma_sigsegv_handler(int signal, siginfo_t *sig_info, void *xinfo)
{
	esma_core_log_ftl("%s() - have crashed (got signal '%d')\n", __func__, signal);
	_esma_backtrace_print_stack();
	exit(1);
}

int esma_backtrace_init(u32 depth)
{
	struct sigaction sa = {0};
	int err;

	if (depth > 128)
		depth = 128;

	esma_backtrace_buff = esma_malloc(sizeof(void *) * depth);
	if (NULL == esma_backtrace_buff) {
		esma_core_log_err("%s() - esma_malloc(%d bytes) failed.\n",
				__func__, sizeof(void *) * depth);
		return 1;
	}
	esma_backtrace_depth = depth;

	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = _esma_sigsegv_handler;

	err = sigaction(SIGSEGV, &sa, NULL);
	if (err) {
		esma_core_log_sys("%s() - sigaction('SIGSEGV') failed: %s\n", __func__, strerror(errno));
		return 1;
	}

	err = sigaction(SIGBUS, &sa, NULL);
	if (err) {
		esma_core_log_sys("%s() - sigaction('SIGBUS') failed: %s\n", __func__, strerror(errno));
		return 1;
	}

	err = sigaction(SIGILL, &sa, NULL);
	if (err) {
		esma_core_log_sys("%s() - sigaction('SIGILL') failed: %s\n", __func__, strerror(errno));
		return 1;
	}

	return 0;
}
