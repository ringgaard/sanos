//
// time.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Time routines
//

#ifndef TIME_H
#define TIME_H

struct tm
{
  int tm_sec;			// Seconds after the minute [0, 59]
  int tm_min;			// Minutes after the hour [0, 59]
  int tm_hour;			// Hours since midnight [0, 23]
  int tm_mday;			// Day of the month [1, 31]
  int tm_mon;			// Months since January [0, 11]
  int tm_year;			// Years since 1900
  int tm_wday;			// Days since Sunday [0, 6]
  int tm_yday;			// Days since January 1 [0, 365]
  int tm_isdst;			// Daylight Saving Time flag
};

struct timeval 
{
  long tv_sec;		        // Seconds
  long tv_usec;		        // Microseconds
};

struct timezone 
{
  int tz_minuteswest;	        // Minutes west of Greenwich
  int tz_dsttime;	        // Type of daylight saving correction
};

struct tm *gmtime(const time_t *timer, struct tm *tmbuf);
time_t mktime(struct tm *tmbuf);

#endif

