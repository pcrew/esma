
#ifndef ESMA_HASH_H
#define ESMA_HASH_H

#include "esma_array.h"

#include "common/numeric_types.h"

struct esma_hashtable_key {
	char *key;
	u32   key_len;
	u32   key_hash;
};

struct esma_hashtable_item {
	void *dptr;
	u32   key_len;
	u8   *key_name[1]; /* using strict hucks */
};

struct esma_hashtable {
	struct esma_array buckets;
	u32 bucket_size;
};

struct esma_hashtable *esma_hashtable_new(char **keys, u32 nkeys);
int esma_hashtable_init(struct esma_hashtable *table, char **keys, u32 nkeys);

void **esma_hashtable_set(struct esma_hashtable *table, u8 *key, u32 key_len);
void  *esma_hashtable_get(struct esma_hashtable *table, u8 *key, u32 key_len);
 int   esma_hashtable_del(struct esma_hashtable *table, u8 *key, u32 key_len);
void   esma_hashtable_clear(struct esma_hashtable *table);

#endif
