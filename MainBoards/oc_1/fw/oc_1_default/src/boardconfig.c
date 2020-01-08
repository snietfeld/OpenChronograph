
#include <Arduino.h>    // For INPUT, OUTPUT constants
#include "boardconfig.h"


void init_pin_cfg(const DigitalPin* p_pin){
  // Set pin mode
  switch(p_pin->mode){
  case OUTPUT:
    *(p_pin->p_ddport) |= (1 << p_pin->ddpin); // Set pin mode (INPUT, OUTPUT)
    break;
  case INPUT:
    *(p_pin->p_ddport) &= ~(1 << p_pin->ddpin);
    break;
  }

  // Handle pin usage
  switch(p_pin->usage){
  case MOTOR_HF0:
    hour_motor.port_F0 = p_pin->p_port;
    hour_motor.mask_F0 = (1 << p_pin->pin);
    break;
  case MOTOR_HF1:
    hour_motor.port_F1 = p_pin->p_port;
    hour_motor.mask_F1 = (1 << p_pin->pin);
    break;
  case MOTOR_HR0:
    hour_motor.port_R0 = p_pin->p_port;
    hour_motor.mask_R0 = (1 << p_pin->pin);
    break;
  case MOTOR_HR1:
    hour_motor.port_R1 = p_pin->p_port;
    hour_motor.mask_R1 = (1 << p_pin->pin);
    break;

  case MOTOR_MF0:
    min_motor.port_F0 = p_pin->p_port;
    min_motor.mask_F0 = (1 << p_pin->pin);
    break;
  case MOTOR_MF1:
    min_motor.port_F1 = p_pin->p_port;
    min_motor.mask_F1 = (1 << p_pin->pin);
    break;
  case MOTOR_MR0:
    min_motor.port_R0 = p_pin->p_port;
    min_motor.mask_R0 = (1 << p_pin->pin);
    break;
  case MOTOR_MR1:
    min_motor.port_R1 = p_pin->p_port;
    min_motor.mask_R1 = (1 << p_pin->pin);
    break;

  case MOTOR_SF0:
    sec_motor.port_F0 = p_pin->p_port;
    sec_motor.mask_F0 = (1 << p_pin->pin);
    break;
  case MOTOR_SF1:
    sec_motor.port_F1 = p_pin->p_port;
    sec_motor.mask_F1 = (1 << p_pin->pin);
    break;
  case MOTOR_SR0:
    sec_motor.port_R0 = p_pin->p_port;
    sec_motor.mask_R0 = (1 << p_pin->pin);
    break;
  case MOTOR_SR1:
    sec_motor.port_R1 = p_pin->p_port;
    sec_motor.mask_R1 = (1 << p_pin->pin);
    break;

    //case BUTTON_INCR:
  }
}

void init_motors(){
  hour_motor.pole = 0;
  hour_motor.angle_meas_deg = 0;
  hour_motor.angle_cmd_deg = 0;

  min_motor.pole = 0;
  min_motor.angle_meas_deg = 0;
  min_motor.angle_cmd_deg = 0;

  sec_motor.pole = 0;
  sec_motor.angle_meas_deg = 0;
  sec_motor.angle_cmd_deg = 0;
}
