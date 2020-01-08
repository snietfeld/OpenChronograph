
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>    // For abs()
#include <math.h>

#include "clock_timeval.h"
#include "timeval.h"


void clock_init(Clock* p_clock){
  p_clock->t_0 = {0};
  p_clock->t_now = p_clock->t_0;
  p_clock->t = {0};
  p_clock->period = {0};

  p_clock->theta_0 = 0.0;
  p_clock->theta_1 = 0.0;
}



void clock_update(Clock* p_clock, Timeval t_now){
  float theta;    // Current clock phase angle

  // Update current time
  p_clock->t_now = t_now;

  // Find time since t_0
  timeval_subtract(&(p_clock->t), &(p_clock->t_now), &(p_clock->t_0));

  float t_norm = get_normalized_time(p_clock->period, p_clock->t);


  // Calculate phase angle
  theta = 2.0 * PI * t_norm;
  
  //printf("theta: %f\n", theta);

  // Now update hand angles
  p_clock->theta_0 = theta;
  p_clock->theta_1 = theta;
}


// |----------|----------|--------  |    2.8s
// |         2 sec       |800000usec|
float get_normalized_time(Timeval T, Timeval t){
  // Convert timevals into long long usecs
  uint64_t T_usec = timeval_to_uint64_usecs(T);
  uint64_t t_usec = timeval_to_uint64_usecs(t);
  

  uint64_t temp = t_usec % T_usec;    // get # of microseconds into the cycle

  // Multiply by scalar so we can use integer division, convert to float, and divide back
  float scal = 100000;
  float t_norm = (float)((temp*scal) / T_usec) / scal;

  return t_norm;
}


Timeval get_timeval_into_cycle(Timeval T, Timeval t){
  // Convert timevals into long long usecs
  uint64_t T_usec = timeval_to_uint64_usecs(T);
  uint64_t t_usec = timeval_to_uint64_usecs(t);

  uint64_t temp = t_usec % T_usec;    // get # of microseconds into the cycle

  Timeval t_into_cycle = uint64_usecs_to_timeval(temp);

  return t_into_cycle;
}


uint64_t timeval_to_uint64_usecs(Timeval t){
  return (uint64_t)t.tv_sec * 1000000 + t.tv_usec;
}

Timeval uint64_usecs_to_timeval(uint64_t t_usecs){
  Timeval t;

  t.tv_sec = t_usecs / 1000000;
  t.tv_usec = t_usecs % 1000000;

  return t;
}


/* Returns the time at a given hand hangle
Args:
    T - Period of clock [timeval]
    theta - Hand angle [rad]
 */
Timeval get_time_at_theta(Timeval T, float theta){
  float t_norm = theta * ONE_OVER_TWOPI;

  uint64_t T_usecs = timeval_to_uint64_usecs(T); 
  uint64_t t_usecs = (T_usecs * (uint64_t)(t_norm * 10000)) / 10000;

  Timeval t = uint64_usecs_to_timeval(t_usecs); 

  return t;
}



Timeval get_time_at_next_theta(Clock* p_clock, float theta){

  // Calculate usecs at theta
  Timeval t_theta = get_time_at_theta(p_clock->period, theta);

  uint64_t usecs_theta = timeval_to_uint64_usecs(t_theta);

  
  // Calculate number of usecs we are into current cycle
  uint64_t T_usecs = timeval_to_uint64_usecs(p_clock->period);
  uint64_t t_usecs = timeval_to_uint64_usecs(p_clock->t);
  
  uint64_t usecs_into_cycle = t_usecs % T_usecs;


  // Subtract to find usecs to next theta
  uint64_t usecs_next;
  if (usecs_into_cycle <= usecs_theta){
    usecs_next = usecs_theta - usecs_into_cycle;
  }
  else{
    usecs_next = T_usecs - (usecs_into_cycle - usecs_theta);
  }

  // Convert back to timeval
  Timeval t_next = uint64_usecs_to_timeval(usecs_next);

  return t_next;
  }


