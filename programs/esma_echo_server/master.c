
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "core/esma_alloc.h"
#include "core/esma_socket.h"
#include "core/esma_logger.h"
#include "core/esma_objpool.h"

#include "engine/esma.h"

#include "common/numeric_types.h"

#define __unbox__	struct esma *from, struct esma *me, void *dptr

struct master_info {
        struct esma_channel *tick_0;
        struct esma_channel *tick_1;

        struct esma_socket socket;

        struct esma_objpool slaves;
};

struct esma_template slave_tmpl = {0};

static int __master_init(struct esma *master)
{
	struct master_info *mi = NULL;

	mi = esma_malloc(sizeof(struct master_info));
	if (NULL == mi) {
		esma_user_log_err("%s()/%s - can't allocae memory for data\n", __func__, master->name);
		goto __fail;
	}

	mi->tick_0 = esma_get_channel(master, "work", 0, ESMA_CH_TICK);
	mi->tick_1 = esma_get_channel(master, "work", 1, ESMA_CH_TICK);
	if (NULL == mi->tick_0 || NULL == mi->tick_1) {
		esma_user_log_err("%s()/%s - can't get channels\n", __func__, master->name);
		exit(1);
	}

#if 0
	esma_channel_set_interval(mi->tick_0, 100);
	esma_channel_set_interval(mi->tick_1, 1000ULL * 60ULL * 60ULL * 24ULL);	/* one day */
#endif

	master->data = mi;
	return 0;

__fail:
	if (mi)
		esma_free(mi);

	return 1;
}

static int __master_init_socket(struct esma *master, u16 port)
{
	struct master_info *mi = master->data;
	   int err;

	err = esma_socket_init(&mi->socket);
	if (err) {
		esma_user_log_err("%s()/%s - esma_socket_init(): failed\n", __func__, master->name);
		goto __fail;
	}

	err = esma_socket_create(&mi->socket, AF_INET);
	if (err) {
		esma_user_log_err("%s()/%s - esma_socket_create(): failed\n", __func__, master->name);
		goto __fail;
	}

	err = esma_socket_reset(&mi->socket);
	if (err) {
		esma_user_log_err("%s()/%s - esma_socket_reset(): failed\n", __func__, master->name);
		goto __fail;
	}

	err = esma_socket_bind(&mi->socket, port, NULL);
	if (err) {
		esma_user_log_err("%s()/%s - esma_socket_bind(1771): failed\n", __func__, master->name);
		goto __fail;
	}

	err = esma_socket_listen(&mi->socket, 128);
	if (err) {
		esma_user_log_err("%s()/%s - esma_socket_listen(): failed\n", __func__, master->name);
		goto __fail;
	}

	esma_engine_init_io_channel(master, mi->socket.fd);
	esma_engine_mod_io_channel(master, ESMA_POLLIN, ESMA_IO_EVENT_ENABLE);
	master->io_channel.flags |= ESMA_LISTENING_CHANNEL;
	return 0;

__fail:
	esma_socket_close(&mi->socket);
	esma_socket_clear(&mi->socket);
	return 1;
}

static int __master_init_slaves(struct esma *master)
{
	struct master_info *mi = master->data;
	   int err;

	err = esma_template_init(&slave_tmpl, "slave");
	if (err)
		goto __fail;

	err = esma_template_set_by_path(&slave_tmpl, "slave.esma");
	if (err)
		goto __fail;

	#define NSLAVES 256
	err = esma_objpool_init(&mi->slaves, NSLAVES, 0, 0);
	if (err) {
		esma_user_log_err("%s()/%s - esma_objpool_init('slaves'): failed\n", __func__, master->name);
		goto __fail;
	}

	for (int i = 0; i < NSLAVES; i++) {
		struct esma *slave = NULL;
		  char name[16];
		   int err;

		slave = esma_malloc(sizeof(struct esma));
		if (NULL == slave) {
			esma_user_log_err("%s()/%s - esma_malloc('slave'): failed\n", __func__, master->name);
			goto __fail;
		}

		err = esma_objpool_put(&mi->slaves, slave);
		if (err) {
			esma_user_log_err("%s()/%s - esma_objpool_put('slaves'): failed\n", __func__, master->name);
			goto __fail;
		}

		sprintf(name, "slave_%d", i);

		err = esma_engine_init_machine(slave, name, &slave_tmpl);
		if (err) {
			esma_user_log_err("%s()/%s - esma_engine_init_machine(slave: '%s'): failed\n", __func__, master->name, name);
			goto __fail;
		}

		esma_engine_run_machine(slave, &mi->slaves);
	}

	return 0;

__fail:
	return 1;
}

int master_init_enter(__unbox__)
{
	int err;
	u16 port = *(u16 *) dptr;

	err = __master_init(me);
	if (err) {
		esma_user_log_err("%s()/%s - master_init(): failed\n", __func__, me->name);
		goto __fini;
	}

	err = __master_init_socket(me, port);
	if (err) {
		esma_user_log_err("%s()/%s - master_init_socket(): failed\n", __func__, me->name);
		goto __fini;
	}
	esma_user_log_inf("%s()/%s - create socket on port '%hu'\n", __func__, me->name, port);

	err = __master_init_slaves(me);
	if (err) {
		esma_user_log_err("%s()/%s - master_init_slaves(): failed\n", __func__, me->name);
		goto __fini;
	}

	esma_user_log_nrm("%s()/%s - successfuly inited\n", __func__, me->name);

	esma_engine_send_msg(me, me, NULL, 0);
	return 0;

__fini:
	esma_user_log_ftl("%s()/%s - can't start\n", __func__, me->name);
	esma_engine_send_msg(me, me, NULL, 3);
	return 0;
}

int master_init_leave(__unbox__)
{
	esma_user_log_nrm("%s()/%s - go to work\n", __func__, me->name);
	return 0;
}

int master_fini_enter(__unbox__)
{
	esma_user_log_nrm("%s()/%s\n", __func__, me->name);
	return 1;
}

int master_fini_leave(__unbox__)
{
	esma_user_log_nrm("%s()/%s\n", __func__, me->name);
	return 1;
}

int master_work_enter(__unbox__)
{
	esma_user_log_nrm("%s()/%s - start timer\n", __func__, me->name);
	return 0;
}

int master_work_tick_0(__unbox__)
{
	static int tick = 1;

	if (tick)
		esma_user_log_nrm("%s()/%s - tick!\n", __func__, me->name);
	else
		esma_user_log_nrm("%s()/%s - tack!\n", __func__, me->name);

	tick ^= 1;
	return 0;
}

int master_work_tick_3(__unbox__)
{
	esma_user_log_nrm("%s()/%s - timeout; go to fini\n", __func__, me->name);
	esma_engine_send_msg(me, me, NULL, 3);
	return 0;
}


int master_work_sign_0(__unbox__)
{
	esma_user_log_nrm("%s()/%s - called!\n", __func__, me->name);
	return 0;
}

int master_work_sign_1(__unbox__)
{
	esma_user_log_nrm("%s()/%s - called!\n", __func__, me->name);
	return 0;
}

int master_work_data_0(__unbox__)
{
	struct master_info *mi = me->data;
	struct esma *slave;

	esma_user_log_nrm("%s()/%s - new client\n", __func__, me->name);
	slave = esma_objpool_get(&mi->slaves);
	if (NULL == slave) {
		struct esma_socket client = {0};
		   int err;

		esma_user_log_wrn("%s()/%s - no more slaves; client gone\n", __func__, me->name);
		err = esma_socket_accept(&client, &mi->socket);
		if (err) {
			esma_user_log_err("%s()/%s - can't accept client\n", __func__, me->name);
			exit(1);
		}

		esma_socket_close(&client);
		return 0;
	}

	esma_engine_send_msg(me, slave, &mi->socket, 1);
	return 0;
}

int master_work_data_1(__unbox__)
{
	esma_user_log_ftl("%s()/%s - have fail with io channel. Exiting\n", __func__, me->name);
	esma_engine_send_msg(me, me, NULL, 3);
	return 0;
}

int master_work_leave(__unbox__)
{
	esma_user_log_wrn("%s()/%s - stop timer\n", __func__, me->name);
	return 0;
}
