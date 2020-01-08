#ifndef __BOARDCONFIG_H__
#define __BOARDCONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "soprod.h"
  
enum pin_use {
  MOTOR_HF0,
  MOTOR_HF1,
  MOTOR_HR0,
  MOTOR_HR1,
  
  MOTOR_MF0,
  MOTOR_MF1,
  MOTOR_MR0,
  MOTOR_MR1,
  
  MOTOR_SF0,
  MOTOR_SF1,
  MOTOR_SR0,
  MOTOR_SR1,

  BUTTON_MODE,
  BUTTON_INCR,
  BUTTON_DECR,
};


typedef struct {
  uint8_t* p_port;
  uint8_t* p_ddport;
  uint8_t pin;
  uint8_t ddpin;
  uint8_t mode;
  enum pin_use usage;
  uint8_t state;
} DigitalPin;

motor_t hour_motor;
motor_t min_motor;
motor_t sec_motor;
  
void init_pin_cfg(const DigitalPin* p_pin);
void init_motors(void);
  
#ifdef __cplusplus
} // extern "C"
#endif
  
#endif // __BOARDCONFIG_H__
