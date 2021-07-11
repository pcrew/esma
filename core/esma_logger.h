
#ifndef ESMA_LOGGER_H
#define ESMA_LOGGER_H

#include "common/numeric_types.h"

#define ESMA_LOG_DBG	0
#define ESMA_LOG_WRN	1
#define ESMA_LOG_ERR	2
#define ESMA_LOG_SYS	3
#define ESMA_LOG_INF	4
#define ESMA_LOG_FTL	5
#define ESMA_LOG_NRM	6

#define ESMA_LOG_USER		0x01
#define ESMA_LOG_CORE		0x02
#define ESMA_LOG_PLUGIN		0x04
#define ESMA_LOG_ENGINE		0x08
#define ESMA_LOG_REACTOR	0x10
#define ESMA_LOG_DISPATCHER	0x20

#define ESMA_MAX_REPORT_STR	2048

extern u32 esma_log_level;
extern u32 esma_log_flags;

void esma_logger_set_log_level(u32 level);
void esma_logger_set_log_flags(u32 flags);

#define esma_user_log_nrm(fmt, ...)		esma_console_log(ESMA_LOG_USER, ESMA_LOG_NRM, fmt, __VA_ARGS__)
#define esma_user_log_dbg(fmt, ...)		esma_console_log(ESMA_LOG_USER, ESMA_LOG_DBG, fmt, __VA_ARGS__)
#define esma_user_log_wrn(fmt, ...)		esma_console_log(ESMA_LOG_USER, ESMA_LOG_WRN, fmt, __VA_ARGS__)
#define esma_user_log_err(fmt, ...)		esma_console_log(ESMA_LOG_USER, ESMA_LOG_ERR, fmt, __VA_ARGS__)
#define esma_user_log_sys(fmt, ...)		esma_console_log(ESMA_LOG_USER, ESMA_LOG_SYS, fmt, __VA_ARGS__)
#define esma_user_log_inf(fmt, ...)		esma_console_log(ESMA_LOG_USER, ESMA_LOG_INF, fmt, __VA_ARGS__)
#define esma_user_log_ftl(fmt, ...)		esma_console_log(ESMA_LOG_USER, ESMA_LOG_FTL, fmt, __VA_ARGS__)

#define esma_core_log_nrm(fmt, ...)		esma_console_log(ESMA_LOG_CORE, ESMA_LOG_NRM, fmt, __VA_ARGS__)
#define esma_core_log_dbg(fmt, ...)		esma_console_log(ESMA_LOG_CORE, ESMA_LOG_DBG, fmt, __VA_ARGS__)
#define esma_core_log_wrn(fmt, ...)		esma_console_log(ESMA_LOG_CORE, ESMA_LOG_WRN, fmt, __VA_ARGS__)
#define esma_core_log_err(fmt, ...)		esma_console_log(ESMA_LOG_CORE, ESMA_LOG_ERR, fmt, __VA_ARGS__)
#define esma_core_log_sys(fmt, ...)		esma_console_log(ESMA_LOG_CORE, ESMA_LOG_SYS, fmt, __VA_ARGS__)
#define esma_core_log_inf(fmt, ...)		esma_console_log(ESMA_LOG_CORE, ESMA_LOG_INF, fmt, __VA_ARGS__)
#define esma_core_log_ftl(fmt, ...)		esma_console_log(ESMA_LOG_CORE, ESMA_LOG_FTL, fmt, __VA_ARGS__)

#define esma_plugin_log_nrm(fmt, ...)		esma_console_log(ESMA_LOG_PLUGIN, ESMA_LOG_NRM, fmt, __VA_ARGS__)
#define esma_plugin_log_dbg(fmt, ...)		esma_console_log(ESMA_LOG_PLUGIN, ESMA_LOG_DBG, fmt, __VA_ARGS__)
#define esma_plugin_log_wrn(fmt, ...)		esma_console_log(ESMA_LOG_PLUGIN, ESMA_LOG_WRN, fmt, __VA_ARGS__)
#define esma_plugin_log_err(fmt, ...)		esma_console_log(ESMA_LOG_PLUGIN, ESMA_LOG_ERR, fmt, __VA_ARGS__)
#define esma_plugin_log_sys(fmt, ...)		esma_console_log(ESMA_LOG_PLUGIN, ESMA_LOG_SYS, fmt, __VA_ARGS__)
#define esma_plugin_log_inf(fmt, ...)		esma_console_log(ESMA_LOG_PLUGIN, ESMA_LOG_INF, fmt, __VA_ARGS__)
#define esma_plugin_log_ftl(fmt, ...)		esma_console_log(ESMA_LOG_PLUGIN, ESMA_LOG_FTL, fmt, __VA_ARGS__)

#define esma_engine_log_nrm(fmt, ...)		esma_console_log(ESMA_LOG_ENGINE, ESMA_LOG_NRM, fmt, __VA_ARGS__)
#define esma_engine_log_dbg(fmt, ...)		esma_console_log(ESMA_LOG_ENGINE, ESMA_LOG_DBG, fmt, __VA_ARGS__)
#define esma_engine_log_wrn(fmt, ...)		esma_console_log(ESMA_LOG_ENGINE, ESMA_LOG_WRN, fmt, __VA_ARGS__)
#define esma_engine_log_err(fmt, ...)		esma_console_log(ESMA_LOG_ENGINE, ESMA_LOG_ERR, fmt, __VA_ARGS__)
#define esma_engine_log_sys(fmt, ...)		esma_console_log(ESMA_LOG_ENGINE, ESMA_LOG_SYS, fmt, __VA_ARGS__)
#define esma_engine_log_inf(fmt, ...)		esma_console_log(ESMA_LOG_ENGINE, ESMA_LOG_INF, fmt, __VA_ARGS__)
#define esma_engine_log_ftl(fmt, ...)		esma_console_log(ESMA_LOG_ENGINE, ESMA_LOG_FTL, fmt, __VA_ARGS__)

#define esma_reactor_log_nrm(fmt, ...)		esma_console_log(ESMA_LOG_REACTOR, ESMA_LOG_NRM, fmt, __VA_ARGS__)
#define esma_reactor_log_dbg(fmt, ...)		esma_console_log(ESMA_LOG_REACTOR, ESMA_LOG_DBG, fmt, __VA_ARGS__)
#define esma_reactor_log_wrn(fmt, ...)		esma_console_log(ESMA_LOG_REACTOR, ESMA_LOG_WRN, fmt, __VA_ARGS__)
#define esma_reactor_log_err(fmt, ...)		esma_console_log(ESMA_LOG_REACTOR, ESMA_LOG_ERR, fmt, __VA_ARGS__)
#define esma_reactor_log_sys(fmt, ...)		esma_console_log(ESMA_LOG_REACTOR, ESMA_LOG_SYS, fmt, __VA_ARGS__)
#define esma_reactor_log_inf(fmt, ...)		esma_console_log(ESMA_LOG_REACTOR, ESMA_LOG_INF, fmt, __VA_ARGS__)
#define esma_reactor_log_ftl(fmt, ...)		esma_console_log(ESMA_LOG_REACTOR, ESMA_LOG_FTL, fmt, __VA_ARGS__)

#define esma_dispatcher_log_nrm(fmt, ...)	esma_console_log(ESMA_LOG_DISPATCHER, ESMA_LOG_NRM, fmt, __VA_ARGS__)
#define esma_dispatcher_log_dbg(fmt, ...)	esma_console_log(ESMA_LOG_DISPATCHER, ESMA_LOG_DBG, fmt, __VA_ARGS__)
#define esma_dispatcher_log_wrn(fmt, ...)	esma_console_log(ESMA_LOG_DISPATCHER, ESMA_LOG_WRN, fmt, __VA_ARGS__)
#define esma_dispatcher_log_err(fmt, ...)	esma_console_log(ESMA_LOG_DISPATCHER, ESMA_LOG_ERR, fmt, __VA_ARGS__)
#define esma_dispatcher_log_sys(fmt, ...)	esma_console_log(ESMA_LOG_DISPATCHER, ESMA_LOG_SYS, fmt, __VA_ARGS__)
#define esma_dispatcher_log_inf(fmt, ...)	esma_console_log(ESMA_LOG_DISPATCHER, ESMA_LOG_INF, fmt, __VA_ARGS__)
#define esma_dispatcher_log_ftl(fmt, ...)	esma_console_log(ESMA_LOG_DISPATCHER, ESMA_LOG_FTL, fmt, __VA_ARGS__)

void esma_console_log(int flags, int level, char *fmt, ...);

#endif
