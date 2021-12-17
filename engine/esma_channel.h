
#ifndef ESMA_CHANNEL_H
#define ESMA_CHANNEL_H

#include <sys/signalfd.h>

#include "common/numeric_types.h"

#define ESMA_CH_EMPTY			0
#define ESMA_CH_NONE			1
#define ESMA_CH_TICK			2
#define ESMA_CH_DATA			3
#define ESMA_CH_SIGN			4

#define ESMA_TM_ONESHOT			0x1
#define ESMA_TM_PERIODIC		0x2

#define ESMA_LISTENING_CHANNEL		0x01

#define ESMA_IO_EVENT_ENABLE         0
#define ESMA_IO_EVENT_DISABLE        1

struct tick_channel_info {
	u64 interval_msec;
	int periodic;
};

struct data_channel_info {
	u32 bytes_avail;
};

struct sign_channel_info {
	struct signalfd_siginfo si;
};

struct esma_channel {
	int fd;
	u8 type;	/* ESMA_CH_XXXX */
	u8 hard;	/* If channel created by config then he is hard channel */
	int index;	/* for reactor */

	union {
		struct tick_channel_info tick;
		struct data_channel_info data;
		struct sign_channel_info sign;
	} info;

	u32 flags;
	u32 raw_data;	/* need for templtate */

	struct esma *owner;
};

static inline  u64 esma_channel_get_interval(struct esma_channel *ch)
{
	return ch->info.tick.interval_msec;
}

static inline void esma_channel_set_interval(struct esma_channel *ch, u64 time)
{
	ch->info.tick.interval_msec = time;
}

static inline u32 esma_channel_bytes_avail(struct esma_channel *ch)
{
	return ch->info.data.bytes_avail;
}

static inline char *esma_channel_type_to_str(u32 ch_type)
{
	switch (ch_type) {
	case ESMA_CH_EMPTY:
		return "ESMA_CH_EMPTY";
	case ESMA_CH_NONE:
		return "ESMA_CH_NONE";
	case ESMA_CH_TICK:
		return "ESMA_CH_TICK";
	case ESMA_CH_DATA:
		return "ESMA_CH_DATA";
	case ESMA_CH_SIGN:
		return "ESMA_CH_SIGN";
	default:
		return "invalid ch_type";
	}
}

#endif
