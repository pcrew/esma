/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_CHANNEL_H
#define ESMA_CHANNEL_H

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

/**
 * @brief Get the interval value in millisecond.
 * @param [in] ch The pointer to the esma_channel.
 * @return Interval in milliseconds.
 */
static inline  u64 esma_channel_get_interval(struct esma_channel *ch)
{
	return ch->info.tick.interval_msec;
}

/**
 * @brief Set the interval value in millisecond.
 * @param ch [out] The pointer to the esma_channel.
 * @param time [in] The time interval in milliseconds.
 */
static inline void esma_channel_set_interval(struct esma_channel *ch, u64 time)
{
	ch->info.tick.interval_msec = time;
}

/**
 * @brief Get the number of bytes available for reading.
 * @param ch [in] The pointer to the esma_channel.
 * @return number of bytes available for reading.
 */
static inline u32 esma_channel_bytes_avail(struct esma_channel *ch)
{
	return ch->info.data.bytes_avail;
}

/**
 * @brief Convert channel type to string.
 * @param ch_type [in] The channel type.
 * @return name string of the input channel.
 */
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
		return "invalid channel type";
	}
}

#endif
