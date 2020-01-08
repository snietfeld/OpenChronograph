
#ifndef __TIMEVAL_H__
#define __TIMEVAL_H__


#include <time.h>

struct Timeval {
  time_t tv_sec;
  unsigned long tv_usec;
};

int timeval_subtract (Timeval *result, Timeval *x, Timeval *y);

#endif // __TIMEVAL_H__
