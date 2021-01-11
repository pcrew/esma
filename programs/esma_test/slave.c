
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "core/esma_dbuf.h"
#include "core/esma_alloc.h"
#include "core/esma_socket.h"
#include "core/esma_logger.h"
#include "core/esma_objpool.h"

#include "engine/esma_engine.h"

#include "common/numeric_types.h"

#define __unbox__	struct esma *from, struct esma *me, void *dptr

struct slave_info {
	struct esma_dbuf dbuf;
	struct esma_socket socket;
	struct esma_objpool *restroom;
};

int slave_init_enter(__unbox__)
{
	struct slave_info *si;
	   int err;

	si = esma_malloc(sizeof(struct slave_info));
	if (NULL == si) {
		esma_user_log_err("%s()/%s - esma_malloc('slave_info'): failed\n", __func__, me->name);
		goto __fini;
	}

#define DBYTES	1024
	err = esma_dbuf_init(&si->dbuf, DBYTES);
	if (err) {
		esma_user_log_err("%s()/%s - esma_dbuf_init('%ld' bytes): failed\n", 
				__func__, me->name, DBYTES);
		goto __fini;
	}
	esma_socket_init(&si->socket);

	si->restroom = dptr;

	me->data = si;

	esma_user_log_nrm("%s()/%s - successfuly inited\n", __func__, me->name);
	esma_msg(me, me, NULL, 0);
	return 0;

__fini:
	esma_msg(me, me, NULL, 3);
	return 0;
}

int slave_init_leave(__unbox__)
{
	esma_user_log_nrm("%s()/%s - go to idle\n", __func__, me->name);
	return 0;
}

int slave_fini_enter(__unbox__)
{
	esma_user_log_nrm("%s()/%s\n", __func__, me->name);
	return 0;
}

int slave_fini_leave(__unbox__)
{
	esma_user_log_nrm("%s()/%s\n", __func__, me->name);
	return 1;
}

int slave_idle_enter(__unbox__)
{
	esma_user_log_nrm("%s()/%s - waiting for new task\n", __func__, me->name);
	return 0;
}

int slave_idle_1(__unbox__)
{
	struct slave_info *si = me->data;
	struct esma_socket *server = dptr;
	   int err;

	if (NULL == dptr) {
		esma_user_log_err("%s()/%s dptr from %s is NULL\n", __func__, me->name, from->name);
		goto __done;
	}

	err = esma_socket_accept(&si->socket, server);
	if (err) {
		esma_user_log_err("%s()/%s - can't accept socket\n", me->name, __func__);
		goto __done;
	}

	esma_engine_init_io_channel(me, si->socket.fd);

	esma_user_log_nrm("%s()/%s - start working\n", __func__, me->name);
	esma_msg(me, me, NULL, 0);
	return 0;

__done:
	esma_user_log_nrm("%s()/%s - some error; goto done\n", __func__, me->name);
	esma_msg(me, me, NULL, 3);
	return 0;
}

int slave_idle_leave(__unbox__)
{
	esma_user_log_nrm("%s()/%s - go to recv\n", __func__, me->name);
	return 0;
}

int slave_recv_enter(__unbox__)
{
	struct slave_info *si = me->data;
	struct esma_dbuf *dbuf = &si->dbuf;

	esma_dbuf_clear(dbuf);

	esma_engine_mod_io_channel(me, ESMA_POLLIN, IO_EVENT_ENABLE);
	esma_user_log_nrm("%s()/%s - start receiving from fd '%d'\n", __func__, me->name, me->io_channel.fd);
	return 0;
}

int slave_recv_tick_0(__unbox__)
{
	esma_user_log_wrn("%s()/%s - recv data: timeout\n", __func__, me->name);
	esma_msg(me, me, NULL, 1);
	return 0;
}

int slave_recv_data_0(__unbox__)
{
	struct slave_info *si = me->data;
	struct esma_dbuf *dbuf = &si->dbuf;
           u32 ba = me->io_channel.info.data.bytes_avail;
	   u32 n;

	if (0 == ba) {	/* connection close */
		esma_user_log_inf("%s()/%s - connection close\n", __func__, me->name);
		goto __done;
	}

	if (ba > dbuf->len) {
		int err = esma_dbuf_expand(dbuf, ba);
		if (err) {
			esma_user_log_err("%s()/%s - esma_dbuf_expand(): failed\n", __func__, me->name);
			goto __done;
		}
	}

	n = read(me->io_channel.fd, dbuf->pos, ba);
	if (n < 0) {
		if (EINTR == errno)
			return 0;

		esma_user_log_err("%s()/%s - read error\n", __func__, me->name);
		goto __done;
	}

	dbuf->cnt += n;

	if (n == ba) {	/* read all data */
	        dbuf->pos = dbuf->loc;	/* need for send */
		esma_user_log_nrm("%s()/%s - all data received\n", __func__, me->name);
		esma_msg(me, me, NULL, 0);
		return 0;
	}

	dbuf->pos += n;
	
	esma_user_log_inf("%s()/%s - again\n", __func__, me->name);
	return 0;

__done:
	esma_msg(me, me, NULL, 1);
	return 0;
}

int slave_recv_data_1(__unbox__)
{
	esma_user_log_err("%s()/%s - connection error\n", __func__, me->name);
	esma_msg(me, me, NULL, 1);
	return 0;
}

int slave_recv_leave(__unbox__)
{
	esma_user_log_nrm("%s()/%s - finish receiving\n", __func__, me->name);
	return 0;
}

int slave_send_enter(__unbox__)
{
	esma_user_log_nrm("%s()/%s - start sending\n", __func__, me->name);
	esma_engine_mod_io_channel(me, ESMA_POLLOUT, IO_EVENT_ENABLE);
	return 0;
}

int slave_send_tick_0(__unbox__)
{
	esma_user_log_wrn("%s()/%s - send data: timeout\n", __func__, me->name);
	esma_msg(me, me, NULL, 1);
	return 0;
}

int slave_send_data_0(__unbox__)
{
	struct slave_info *si = me->data;
	struct esma_dbuf *dbuf = &si->dbuf;
	   int n;

	n = write(si->socket.fd, dbuf->loc, dbuf->cnt);

	if (n < 0) {
		if (EINTR == errno)
			return 0;

		goto __send_error;
	}

	if (n == dbuf->cnt) {
		esma_engine_mod_io_channel(me, 0, IO_EVENT_DISABLE);
		esma_msg(me, me, NULL, 0);
		return 0;
	}

	dbuf->pos += n;
	dbuf->cnt -= n;

	esma_user_log_inf("%s()/%s - again\n", __func__, me->name);
	return 0;

__send_error:

	esma_dbuf_clear(dbuf);
	esma_user_log_err("%s()/%s - sending error\n", __func__, me->name);
	esma_msg(me, me, NULL, 1);
	return 0;
}

int slave_send_data_1(__unbox__)
{
	return 0;
}

int slave_send_leave(__unbox__)
{
	esma_user_log_nrm("%s()/%s - finish sending\n", __func__, me->name);
	return 0;
}

int slave_done_enter(__unbox__)
{
	struct slave_info *si = me->data;
	struct esma_objpool *pool = si->restroom;
	   int err;

	esma_engine_free_io_channel(me);

	esma_socket_close(&si->socket);
	esma_socket_clear(&si->socket);

	err = esma_objpool_put(pool, me);
	if (err) {
		esma_user_log_ftl("%s()/%s - esma_objpool_put(): failed\n", __func__, me->name);
		exit(1);
	}

	esma_user_log_nrm("%s()/%s - finished work with client\n", __func__, me->name);
	esma_msg(me, me, NULL, 0);
	return 0;
}

int slave_done_leave(__unbox__)
{
	esma_user_log_nrm("%s()/%s - go to idle\n", __func__, me->name);
	return 0;
}
