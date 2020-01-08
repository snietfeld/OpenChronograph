#ifndef __BOARDCONFIG_MARK_II_H__
#define __BOARDCONFIG_MARK_II_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <pins_arduino.h>
#include "boardconfig.h"

// Pin Mapping Table
//--------------------------------------------------------------------------------
//Phys Pin    ID    Arduino Pin
// 1          PC6     RESET
// 2          PD0       0     (RX)
// 3          PD1       1     (TX)
// 4          PD2       2
// 5          PD3       3
// 6          PD4       4
// 7          VCC
// 8          GND
// 9          PB6       xtal1
// 10         PB7       xtal2
// 11         PD5       5
// 12         PD6       6
// 13         PD7       7
// 14         PB0       8
// 15         PB1       9
// 16         PB2       10
// 17         PB3       11
// 18         PB4       12
// 19         PB5       13
// 20         AVCC
// 21         AREF
// 22         GND
// 23         PC0       14    (A0)
// 24         PC1       15    (A1)
// 25         PC2       16    (A2)
// 26         PC3       17    (A3)
// 27         PC4       18    (A4)
// 28         PC5       19    (A5)

// ATMEL ATMEGA8 & 168 / ARDUINO
//
//                  +-\/-+
//            PC6  1|    |28  PC5 (AI 5)
//      (D 0) PD0  2|    |27  PC4 (AI 4)
//      (D 1) PD1  3|    |26  PC3 (AI 3)
//      (D 2) PD2  4|    |25  PC2 (AI 2)
// PWM+ (D 3) PD3  5|    |24  PC1 (AI 1)
//      (D 4) PD4  6|    |23  PC0 (AI 0)
//            VCC  7|    |22  GND
//            GND  8|    |21  AREF
//            PB6  9|    |20  AVCC
//            PB7 10|    |19  PB5 (D 13)
// PWM+ (D 5) PD5 11|    |18  PB4 (D 12)
// PWM+ (D 6) PD6 12|    |17  PB3 (D 11) PWM
//      (D 7) PD7 13|    |16  PB2 (D 10) PWM
//      (D 8) PB0 14|    |15  PB1 (D 9) PWM
//                  +----+

// MR0 - Minute Hand, Reverse Drive, Contact 0
// MR1 - Minute Hand, Reverse Drive, Contact 1
// MF0 - Minute Hand, Forward Drive, Contact 0
// ...
// HF1 - Hour Hand, Forward Drive, Contact 1

#define N_DIO_PINS 15
  
const DigitalPin pin_cfg[] = {
  {&PORTC, &DDRC, PC2, DDC2, OUTPUT, MOTOR_HF0, 0},
  {&PORTC, &DDRC, PC3, DDC3, OUTPUT, MOTOR_HF1, 0},
  {&PORTB, &DDRB, PB1, DDB1, OUTPUT, MOTOR_HR0, 0},
  {&PORTB, &DDRB, PB2, DDB2, OUTPUT, MOTOR_HR1, 0},

  {&PORTB, &DDRB, PB0, DDB0, OUTPUT, MOTOR_MF0, 0},
  {&PORTD, &DDRD, PD7, DDD7, OUTPUT, MOTOR_MF1, 0},
  {&PORTD, &DDRD, PD5, DDD5, OUTPUT, MOTOR_MR0, 0},
  {&PORTD, &DDRD, PD6, DDD6, OUTPUT, MOTOR_MR1, 0},

  {&PORTB, &DDRB, PB6, DDB6, OUTPUT, MOTOR_SF0, 0},
  {&PORTB, &DDRB, PB7, DDB7, OUTPUT, MOTOR_SF1, 0},
  {&PORTD, &DDRD, PD3, DDD3, OUTPUT, MOTOR_SR0, 0},
  {&PORTD, &DDRD, PD4, DDD4, OUTPUT, MOTOR_SR1, 0},

  {&PORTC, &DDRC, A0,  DDC0, INPUT, BUTTON_DECR, 0},
  {&PORTC, &DDRC, A1,  DDC1, INPUT, BUTTON_MODE, 0},
  {&PORTD, &DDRD, PD2, DDD2, INPUT, BUTTON_INCR, 0},
};


  

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __BOARDCONFIG_MARK_II_H__
