
#include <stdio.h>
#include <sys/time.h>

#include "esma_time.h"
#include "esma_logger.h"

#define ESMA_TIME_SLOTS	128

volatile u32 esma_log_time_size = sizeof("0000/00/00 00:00:00");

struct times {
	struct esma_time time;
	u8 log_time[sizeof("0000/00/00 00:00:00")];
};

static struct times times[ESMA_TIME_SLOTS];
static u32 slot;

volatile struct esma_time *esma_time;
volatile u8 *esma_log_time;

__attribute__((constructor))
void esma_time_init(void)
{
	esma_time_update();
}

void esma_time_update(void)
{
	struct tm tm;
	struct timeval tv;
	struct times *current_time;
	u8 *time_str;

	time_t sec;
	u32   usec;

	gettimeofday(&tv, NULL);

	sec =  tv.tv_sec;
	usec = tv.tv_usec;

	current_time = &times[slot];
	slot = slot == 127 ? 0 : slot + 1;

	current_time->time.sec = sec;
	current_time->time.usec = usec;

	esma_time = &current_time->time;
	localtime_r(&sec, &tm);

	tm.tm_mon++;
	tm.tm_year += 1900;

	time_str = current_time->log_time;
	sprintf((char *) time_str, "%4d/%02d/%02d  %02d:%02d:%02d",
		tm.tm_year, tm.tm_mon, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec);

	esma_log_time = time_str;
}
