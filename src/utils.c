#include "utils.h"

unsigned int tv_diff(struct timeval *tv1, struct timeval *tv2)
{
	unsigned int udiff;
	unsigned int diff;
	udiff = (unsigned int)(tv1->tv_usec-tv2->tv_usec);
	diff = (unsigned int)(tv1->tv_sec-tv2->tv_sec);
	return (diff*1000000+udiff)/1000;
}


/*
void time_format(time_t time, char *buf, uint len)
{
	struct tm tm_now;
	localtime_r(&time, &tm_now);
	snprintf(buf, len, "%04d-%02d-%02d %02d:%02d:%02d", tm_now.tm_year + 1900,
			tm_now.tm_mon + 1, tm_now.tm_mday, tm_now.tm_hour, tm_now.tm_min,
			tm_now.tm_sec);
}
*/
