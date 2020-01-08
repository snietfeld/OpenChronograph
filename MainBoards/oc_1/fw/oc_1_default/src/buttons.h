
#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#ifdef __cplusplus
extern "C" {
#endif
  
#include <stdint.h>

#define DEFAULT_DEBOUNCE_DELAY_MS 100
#define PRESS_LOCKOUT_DELAY_MS 200
#define REPEAT_START_DELAY_MS 1000    // Button will start repeat firing after this
#define REPEAT_DELAY_MS 20           // Time between repeat fires

#define BUTTON_PRESSED_SLEEP_MS 20

typedef struct {
  int btn_pin;
  int btn_state;
  int last_btn_state;
  unsigned long last_debounce_time;
  unsigned long debounce_delay_ms;
  unsigned long last_pressed_time;
  unsigned long last_repeat_time;
} button_t;

int check_button_pressed(button_t* p_btn);
void force_press_button(button_t* p_btn);

#ifdef __cplusplus
} // extern "C"
#endif
  
#endif // __BUTTONS_H__
