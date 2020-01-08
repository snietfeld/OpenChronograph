

#include <stdint.h>
#include <Arduino.h>
#include "buttons.h"

// We can't use millis() because our internal clock loses time during sleeps.

extern uint32_t total_ms_slept;

int check_button_pressed(button_t* p_btn){
  int reading = 0;
  int pressed = 0;
  unsigned long t_now_ms;

  t_now_ms = millis() + total_ms_slept;
  reading = digitalRead(p_btn->btn_pin);


  // If reading is high and time since last press if greater than lockout, return true
  if ( reading == 1)
    {
      if ( p_btn->btn_state == 0 && (t_now_ms - p_btn->last_pressed_time > PRESS_LOCKOUT_DELAY_MS))
	{
	  pressed = 1;
	  p_btn->last_pressed_time = t_now_ms;
	  p_btn->last_repeat_time = t_now_ms;
	}
      else if (p_btn->btn_state == 1  && 
	       (t_now_ms - p_btn->last_pressed_time > REPEAT_START_DELAY_MS) && 
	       (t_now_ms - p_btn->last_repeat_time > REPEAT_DELAY_MS) )
	{
	  pressed = 1;
	  p_btn->last_repeat_time = t_now_ms;
	}
    }
  
  p_btn->btn_state = reading;
  return pressed;
}


int check_button_pressed_debounce(button_t* p_btn){
  int reading = 0;
  int pressed = 0;
  unsigned long t_now_ms = millis();
  
  reading = digitalRead(p_btn->btn_pin);
  
  // check to see if you just pressed the button 
  // (i.e. the input went from LOW to HIGH),  and you've waited 
  // long enough since the last press to ignore any noise:  
  
  // If the switch changed, due to noise or pressing:
  if (reading != p_btn->last_btn_state) 
    {
      // reset the debouncing timer  long t_now_ms = millis();
      p_btn->last_debounce_time = t_now_ms;
    } 
  
  if ((t_now_ms - p_btn->last_debounce_time) > p_btn->debounce_delay_ms) 
    {
      // whatever the reading is at, it's been there for longer
      // than the debounce delay, so take it as the actual current state:

      // Return true if the button has just gone high
      if( (p_btn->btn_state == 0 && reading == 1) && (t_now_ms - p_btn->last_pressed_time > PRESS_LOCKOUT_DELAY_MS) )
	{
	  pressed = 1;
	  p_btn->last_pressed_time = t_now_ms;
	  p_btn->last_repeat_time = t_now_ms;
	}

      else if( p_btn->btn_state != 0 && ((t_now_ms - p_btn->last_pressed_time) > REPEAT_START_DELAY_MS) && ((t_now_ms - p_btn->last_repeat_time) > REPEAT_DELAY_MS) )
	{
	  pressed = 1;
	  p_btn->last_repeat_time = t_now_ms;
	}
      
      p_btn->btn_state = reading;
    }
  
  
  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  p_btn->last_btn_state = reading;

  return pressed;
}

void force_press_button(button_t* p_btn){
  unsigned long t_now_ms = millis();

  p_btn->last_pressed_time = t_now_ms;
  p_btn->last_repeat_time = t_now_ms;

  return;
}
