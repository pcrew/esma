
#include <stdlib.h>
#include <string.h>

#include "esma_dbuf.h"
#include "esma_alloc.h"
#include "esma_logger.h"

#include "common/compiler.h"

struct esma_dbuf *new_esma_dbuf(u32 len)
{
	struct esma_dbuf *dbuf;
	   u32 err;

	dbuf = esma_malloc(sizeof(struct esma_dbuf));
	if (unlikely(NULL == dbuf)) {
		esma_core_log_err("%s() - esma_malloc(%ld bytes): failed\n",
				__func__, sizeof(struct esma_dbuf));
		return NULL;
	}

	err = esma_dbuf_init(dbuf, len);
	if (unlikely(err)) {
		esma_core_log_err("%s() - esma_dbuf_init(%ld bytes): failed\n",
				__func__, len);
		esma_free(dbuf);
		return NULL;
	}

	return dbuf;
}

int esma_dbuf_init(struct esma_dbuf *dbuf, u32 len)
{
	u8 *loc = NULL;

	if (unlikely(NULL == dbuf))
		return 1;

	loc = esma_malloc(len);
	if (unlikely(NULL == loc)) {
		esma_core_log_err("%s() - esma_malloc(%ld bytes): failed\n",
				__func__, len);
		return 1;
	}

	dbuf->loc = loc;
	dbuf->pos = loc;
	dbuf->cnt = 0;
	dbuf->len = len;
	return 0;
}

int esma_dbuf_expand(struct esma_dbuf *dbuf, u32 new_len)
{
	u8 *loc;

	if (unlikely(NULL == dbuf))
		return 1;

	loc = esma_realloc(dbuf->loc, new_len);
	if (unlikely(NULL == loc)) {
		esma_core_log_err("%s() - esma_realloc(%ld bytes): failed; prev len: %ld\n",
				__func__, new_len, dbuf->len);
		return 1;
	}

	dbuf->pos = loc + (dbuf->pos - dbuf->loc);
	dbuf->loc = loc;
	dbuf->len = new_len;
	return 0;
}

void esma_dbuf_free(struct esma_dbuf *dbuf)
{
	if (unlikely(NULL == dbuf))
		return;

	esma_free(dbuf->loc);
}

void esma_dbuf_clear(struct esma_dbuf *buf)
{
	memset(buf->loc, 0, buf->len);
	buf->cnt = 0;
	buf->pos = buf->loc;
}
