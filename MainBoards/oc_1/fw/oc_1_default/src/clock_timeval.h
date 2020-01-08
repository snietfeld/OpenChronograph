
#ifndef __CLOCK_TIMEVAL_H__
#define __CLOCK_TIMEVAL_H__

#include <stdint.h>
#include <math.h>
#include "timeval.h"

#define PI M_PI
#define TWOPI (2.0f * PI)
#define ONE_OVER_TWOPI (1.0f / TWOPI)    // Used in get_time_at_theta()
#define DEG_TO_RAD (M_PI / 180.0)
#define RAD_TO_DEG (180.0 / M_PI)


struct Clock{
  Timeval t_0;     // Epoch
  Timeval t_now;   // Current time
  Timeval t;       // Time since epoch

  Timeval period;

  float theta_0;
  float theta_1;
};


void clock_init(Clock* p_clock);
void clock_update(Clock* p_clock, Timeval t_now);
Timeval get_time_at_theta(Timeval T, float e, float theta);
Timeval get_time_at_next_theta(Clock* p_clock, float theta);

float get_normalized_time(Timeval T, Timeval t);

uint64_t timeval_to_uint64_usecs(Timeval t);
Timeval uint64_usecs_to_timeval(uint64_t t_usecs);

#endif // __CLOCK_TIMEVAL_H__
