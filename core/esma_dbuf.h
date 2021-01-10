
#ifndef ESMA_DBUF_H
#define ESMA_DBUF_H

#include "common/numeric_types.h"

struct esma_dbuf {
	u8 *loc;
	u8 *pos;
	u32 cnt;
	u32 len;
};

#define esma_dbuf(str) { (u8 *) (str), (u8 *) (str), 0, sizeof(str) - 1 }
#define esma_dbuf_set(dbuf, str) 	\
	(dbuf)->loc = (u8 *) str;	\
	(dbuf)->pos = (dbuf)->loc;	\
	(dbuf)->cnt = sizeof(str) - 1;	\
	(dbuf)->len = (dbuf)->cnt;

struct esma_dbuf *new_esma_dbuf(u32 len);
   int esma_dbuf_init(struct esma_dbuf *dbuf, u32 len);
   int esma_dbuf_expand(struct esma_dbuf *dbuf, u32 new_len);
  void esma_dbuf_free(struct esma_dbuf *dbuf);
  void esma_dbuf_clear(struct esma_dbuf *dbuf);

#endif
