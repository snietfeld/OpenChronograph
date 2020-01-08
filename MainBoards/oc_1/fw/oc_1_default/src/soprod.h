
#ifndef __SOPROD_H__
#define __SOPROD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define SOPROD_TDRIVE_MS 10L       // Time coils are driven
#define SOPROD_TWAIT_MS 0L
#define SOPROD_TTICK_MS (SOPROD_TDRIVE_MS + SOPROD_TWAIT_MS)
  
#define SOPROD_TICKS_PER_REV 360

enum motor_state_t {
  IDLE,
  DRIVING_FWD,
  WAITING_FWD,
  DRIVING_REV,
  WAITING_REV,
};

  
typedef struct {
  uint8_t* port_F0;
  uint8_t mask_F0;

  uint8_t* port_F1;
  uint8_t mask_F1;

  uint8_t* port_R0;
  uint8_t mask_R0;

  uint8_t* port_R1;
  uint8_t mask_R1;
  
  uint8_t pole;            // {0,1}
  int16_t angle_meas_deg;  // 0-359 [deg]
  int16_t angle_cmd_deg;   // 0-359 [deg]

  uint8_t state;
  unsigned long t_tick_start_ms;
} motor_t;

void motor_init(motor_t* m);
void motor_tick_fwd(motor_t* m);
void motor_tick_rev(motor_t* m);
void motor_set_cmd(motor_t* m, int16_t cmd);
void motor_update_angle(motor_t* m);
void motor_update(motor_t* m, unsigned long t_ms);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // __SOPROD_H__
