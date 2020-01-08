/*
oc_1_default.ino - Default firmware for OpenChronograph OC-1 mainboard.

ToDo:
 - Clean up
 - Improve documentation

Each cycle through:
 1) Update inputs (time, IMU, mag, etc)
 2) Update hand states {pos, vel, cmd)
 3) Step hands

Each mode defines a set of inputs required and hand state update functions.
Button events are handled as interrupts, and modify the mode. 
*/

#define MPL

//#include <math.h>
#include <pins_arduino.h>
#include "src/boardconfig_oc1.h"
#include "src/soprod.h"
#include "src/imu.h"
//#include "clock.h"
#include "src/timeval.h"
#include "src/clock_timeval.h"
#include "src/thirdparty/Time/Time.h"   // For converting RTC output to Unix time
#include "src/buttons.h"

#include <avr/sleep.h>
#include <avr/power.h>
#include "src/thirdparty/Low-Power/LowPower.h"

#ifdef MPL
// For pressure sensor driver
#include "src/thirdparty/SparkFun_MPL3115A2_Breakout_Arduino_Library/src/SparkFunMPL3115A2.h" 
#endif

// For RTC driver
#include "src/thirdparty/SparkFun_RV-1805_Arduino_Library/src/SparkFun_RV1805.h"

//#include <PinChangeInterrupt.h>    // For button interrupts
#include "src/thirdparty/PinChangeInterrupt/src/PinChangeInterrupt.h"

#include <Wire.h>                  // To configure I2C speed



#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (1/DEG2RAD)


enum btn_map {
  BTN_INCR,
  BTN_MODE,
  BTN_DECR,
};

button_t buttons[] = {
  {.btn_pin = PD2,
   .btn_state = 0,
   .last_btn_state = 0,
   .last_debounce_time = 0,
   .debounce_delay_ms = 100,
   .last_pressed_time = 0,
   .last_repeat_time = 0},
  
  {.btn_pin = A1, //PC1,
   .btn_state = 0,
   .last_btn_state = 0,
   .last_debounce_time = 0,
   .debounce_delay_ms = 100,
   .last_pressed_time = 0,
   .last_repeat_time = 0},
  
  {.btn_pin = A0, //PC0,
   .btn_state = 0,
   .last_btn_state = 0,
   .last_debounce_time = 0,
   .debounce_delay_ms = 100,
   .last_pressed_time = 0,
   .last_repeat_time = 0},
};

Clock clock_0;    // Default clock


enum DisplayMode {
  MODE_SET_CLOCK,
  MODE_SET_CLOCK_PHASE,
  MODE_SET_CLOCK_PERIOD,
  MODE_TRADITIONAL_TIME,
  MODE_MAG_HEADING,
  MODE_GYRO_HEADING,
  MODE_PRESSURE_ALT,
  MODE_TEMPERATURE,
  MODE_SET_HR_HAND,
  MODE_SET_MIN_HAND,
  MODE_SET_SEC_HAND,
  };



// Note: indexes need to be complete or compiler gets upset
const uint16_t mode_indicator_angle_map[] = {
  [MODE_SET_CLOCK] = 0,
  [MODE_SET_CLOCK_PHASE] = 15,
  [MODE_SET_CLOCK_PERIOD] = 30,
  [MODE_TRADITIONAL_TIME] = 45,
  [MODE_MAG_HEADING] = 60,
  [MODE_GYRO_HEADING] = 75,
  [MODE_PRESSURE_ALT] = 90,
  [MODE_TEMPERATURE] =  105,
  [MODE_SET_HR_HAND] = 120,
  [MODE_SET_MIN_HAND] = 135,
  [MODE_SET_SEC_HAND] = 150,
};


// Volatile b/c button press interrupts could change at any time
volatile DisplayMode display_mode = MODE_SET_HR_HAND;

#ifdef MPL
MPL3115A2 myPressure;
#endif

RV1805 rtc;
tmElements_t tmSet;
Timeval tv_rtc;

//unsigned long t_millis_slept = 0;
uint32_t ms_to_sleep = 0;     // Used to store number of ms to sleep
uint32_t total_ms_slept = 0;  // Used to correct millis() (clock is off during sleep)

volatile int16_t traditional_time_offset_minutes = 0;
volatile int16_t traditional_time_offset_hours = 0;

bool IMU_is_on = 1;    // Flag indicating whether we have the IMU powered on


void setup() {
  delay(3000);
  
  //Initialize serial and wait for port to open:
  Serial.begin(38400);
  //Serial.println("Serial port opened...");
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  

  // Initialize digital IO pins
  // FIXME: Not initializing last three pins
  for(int i = 0; i < N_DIO_PINS-3; i++){
    init_pin_cfg(&pin_cfg[i]);
  }

  
  // Configure I2C bus
  Wire.begin(); //Join the bus as master. 
  //By default .begin() will set I2C SCL to Standard Speed mode of 100kHz
  Wire.setClock(400000); //Optional - set I2C SCL to High Speed Mode of 400kHz


  init_motors();    // Initialize hand motors
  
  cal();



  // Initialize RTC
  //------------------------------------------------------------  
  while(1)
    {
      if (rtc.begin() == false) {
	Serial.println("RTC init failed");
	Serial.flush();
	delay(1000);
      }
      else {
	Serial.println("RTC init OK");
	Serial.flush();
	break;
      }
    }
  

  // rtc.begin() tells rtc to use the RC oscillator
  // The RC oscillator results in invalid hundredths values
  // From pg. 46: "The Hundredths Counter is not available when the RC oscillator is selected"
  // So, let's tell him to use the crystal for now.
  // See pg. 31 in the RV-1805 application manual
  rtc.writeRegister(RV1805_CONF_KEY, RV1805_CONF_OSC); //Unlock again
  rtc.writeRegister(RV1805_OSC_CTRL, 0b00000100); //OSEL=0, ACAL=00, BOS=0, FOS=0, IOPW=1, OFIE=0, ACIE=0
  //rtc.writeRegister(RV1805_OSC_CTRL, 0b00000000); //OSEL=0, ACAL=00, BOS=0, FOS=0, IOPW=1, OFIE=0, ACIE=0
 
  rtc.setToCompilerTime();
  //rtc.setHours(9);
  //rtc.setMinutes(12);
  

  rtc.setPowerSwitchFunction(PSWS_STATIC);  // See pg 22 of the RV-1805 App Manual
  delay(100);

  //poweroff_IMU();
  //rtc.enableSleep();
  /*rtc.setPowerSwitchLock(false);
  delay(100);
  rtc.setStaticPowerSwitchOutput(true);  // Pushes PSW high, starving IMU of ground.
  delay(100);
  rtc.setPowerSwitchLock(true);*/

  delay(100);
  //Serial.println("Telling the RTC to power down the IMU...");
  //rtc.setPowerSwitchLock(false);
  //rtc.setStaticPowerSwitchOutput(false);  // Pushes PSW low, providing IMU with ground.
  //IMU_is_on = false;
  //poweroff_IMU();


  


  // Initialize IMU
  //------------------------------------------------------------
  //poweron_IMU();    // IMU may still be off if Atmega is reset without RTC being power cycled
  init_imu();
  imu_sleep();
  //poweroff_IMU();
  IMU_is_on = 0;


#ifdef MPL
  // Initialize pressure sensor
  //------------------------------------------------------------
  myPressure.begin(); // Get sensor online
  myPressure.setModeAltimeter(); // Measure altitude above sea level in meters
  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags 

  float altitude = myPressure.readAltitudeFt();
  Serial.print("Alt(ft): ");
  Serial.println(altitude, 2);

  myPressure.setModeStandby();    // Put altimeter into low-power mode (2mA)
 #endif


  // disable ADC
  ADCSRA = 0;

  // turn off everything we can
  power_adc_disable ();
  //power_spi_disable();
  //power_twi_disable();
  //power_timer0_disable();
  //power_timer1_disable();
  //power_usart0_disable();


  // Set initial clock parameters
  //------------------------------------------------------------
  // Standard min hand clock
  /*clock_init(&clock_0);
    clock_0.period.tv_sec = 3600.0;*/

  // 1 tick/s clock
  clock_init(&clock_0);
  clock_0.period.tv_sec = 360.0;


  Serial.flush();
  Serial.print("Free RAM: ");
  Serial.println(freeRam());


  // Set interrupts for buttons.
  // Pull-down resistors normally keep these pins low.
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(buttons[BTN_INCR].btn_pin),
			   button_pressed_isr, RISING);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(buttons[BTN_MODE].btn_pin),
			   button_pressed_isr, RISING);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(buttons[BTN_DECR].btn_pin),
			   button_pressed_isr, RISING);

  
  //Serial.println("Initialization complete:");
  Serial.flush();
}


void loop()
{  
  // Disable button interrupts while we're doing stuff
  disablePinChangeInterrupt(digitalPinToPinChangeInterrupt(buttons[BTN_INCR].btn_pin));
  disablePinChangeInterrupt(digitalPinToPinChangeInterrupt(buttons[BTN_MODE].btn_pin));
  disablePinChangeInterrupt(digitalPinToPinChangeInterrupt(buttons[BTN_DECR].btn_pin));
  

  // Give motors opportunity to de-energize coils at beginning of cycle so we're not
  // running them unnecessarily while we update the RTC and clock
  motor_update(&hour_motor, millis() + total_ms_slept);
  motor_update(&min_motor, millis() + total_ms_slept);
  motor_update(&sec_motor, millis() + total_ms_slept);



  // Update RTC
  update_rtc_time();   // Takes 2.5ms(!)

  Serial.print("T_rtc: ");
  Serial.print(tv_rtc.tv_sec);
  Serial.print(".");
  Serial.println(tv_rtc.tv_usec);
  Serial.flush();

  
  /*String currentDate = rtc.stringDateUSA(); //Get the current date in mm/dd/yyyy
  String currentTime = rtc.stringTime(); //Get the time
  Serial.print("\n");
  Serial.print(currentDate);
  Serial.print(" ");
  Serial.println(currentTime); */

  

  // Get updated hand commands
  update_cmds();



  //Run motor updates until hands are where they should be
  while( true ){
    motor_update(&hour_motor, millis() + total_ms_slept);
    motor_update(&min_motor, millis() + total_ms_slept);
    motor_update(&sec_motor, millis() + total_ms_slept);


    // If all motors have reached their targets, break
    if ( hour_motor.angle_meas_deg == hour_motor.angle_cmd_deg &&
	 min_motor.angle_meas_deg == min_motor.angle_cmd_deg &&
	 sec_motor.angle_meas_deg == sec_motor.angle_cmd_deg)
      break;

    // Or if all motors are within 1 tick of target and just waiting for tick
    // to complete (not driving), then we can go ahead and sleep.
    if ( abs(hour_motor.angle_meas_deg - hour_motor.angle_cmd_deg) <= 1 &&
	 (hour_motor.state == WAITING_FWD || hour_motor.state== WAITING_REV) &&
	 
	 abs(min_motor.angle_meas_deg - min_motor.angle_cmd_deg) <= 1 &&
	 (min_motor.state == WAITING_FWD || min_motor.state== WAITING_REV) &&
	 
	 abs(sec_motor.angle_meas_deg - sec_motor.angle_cmd_deg) <= 1 &&
	 (sec_motor.state == WAITING_FWD || sec_motor.state== WAITING_REV))
      break;
    

    /* ---- TO SLEEP OR NOT TO SLEEP ----
      Intuitively we would sleep during motor ticks (10ms); however, the minimum amount
      we can sleep is 15ms, so going to sleep would mean driving the motor coils for an
      extra 5ms AND we get slower tick speeds (10ms/tick vs. 15ms/tick).

      The breakeven point appears to be at a drive duration of between 5ms (80uJ/tick) 
      and 6ms(116uJ/tick). For 5ms, it's more efficient to busywait. For 6ms, it's more 
      efficient to put the MCU to sleep and drive the motor for the minimum sleep time 
      of 15ms. 

      However, for slow clocks like the standard 12-hr clock the sleep current is the
      dominating factor, and the choice to busywait vs. sleep during ticks doesn't 
      make much difference power-wise. So for now we'll busywait to get higher tick speeds.

      If we change our minds, we can uncomment the line below to sleep during motor ticks:
     */
    //total_ms_slept += do_sleep(SOPROD_TTICK_MS);
  }


  
  // Normally handled by ISR, but need to check here or else if we never sleep, buttons
  // will never get checked (because ISRs are disabled during main loop)
  check_buttons();  


  // Print state
  //tm_out();
  //Serial.flush();

  
  // Find out how long we can sleep for
  ms_to_sleep = get_ms_to_next_event();


  Serial.print("ms to next event: ");
  Serial.println(ms_to_sleep);
  Serial.flush();

  
  // Re-enable interrupts so we catch button presses while asleep
  enablePinChangeInterrupt(digitalPinToPinChangeInterrupt(buttons[BTN_INCR].btn_pin));
  enablePinChangeInterrupt(digitalPinToPinChangeInterrupt(buttons[BTN_MODE].btn_pin));
  enablePinChangeInterrupt(digitalPinToPinChangeInterrupt(buttons[BTN_DECR].btn_pin));



  
  // Sleep until next event
  while ( ms_to_sleep > 0 ){
    uint32_t ms_slept;

    ms_slept = do_sleep(ms_to_sleep);
    total_ms_slept += ms_slept;

    // Can't just blindly decrement ms_to_sleep--could underflow
    ms_to_sleep = ms_slept >= ms_to_sleep ? 0 : ms_to_sleep - ms_slept;

    // Break out early if button has been pushed
    if ( buttons[BTN_MODE].btn_state || buttons[BTN_INCR].btn_state || buttons[BTN_DECR].btn_state ){
      ms_to_sleep = 0;
      break;
    }
  }
}


uint32_t do_sleep(uint32_t sleep_ms){
  uint32_t t_millis_slept;
  
  if ( sleep_ms < 15 ){
    // Sleep minimum amount
    //Serial.println("Sleeping 15ms");
    //Serial.flush();
    LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_OFF);
    t_millis_slept =  15;
  }
  else if ( sleep_ms < 30 ){
    //Serial.println("Sleeping 15ms");
    //Serial.flush();
    LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_OFF);
    t_millis_slept = 15;
  }
  else if ( sleep_ms < 60 ){
    //Serial.println("Sleeping 30ms");
    //Serial.flush();
    LowPower.powerDown(SLEEP_30MS, ADC_OFF, BOD_OFF);
    t_millis_slept = 30;
  }
  else if ( sleep_ms < 120 ){
    //Serial.println("Sleeping 60ms");
    //Serial.flush();
    LowPower.powerDown(SLEEP_60MS, ADC_OFF, BOD_OFF);
    t_millis_slept = 60;
  }
  else if ( sleep_ms < 250 ){
    //Serial.println("Sleeping 120ms");
    //Serial.flush();
    LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF);
    t_millis_slept = 120;
  }
  else if ( sleep_ms < 500 ){
    //Serial.println("Sleeping 250ms");
    //Serial.flush();
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF);
    t_millis_slept = 250;
  }
  else if ( sleep_ms < 1000 ){
    //Serial.println("Sleeping 500ms");
    //Serial.flush();
    LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF);
    t_millis_slept = 500;
  }
  else { 
    //Serial.println("Sleeping 1000ms");
    //Serial.flush();
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
    t_millis_slept = 1000;
    }
  
  return t_millis_slept;
}


void update_cmds()
{
  float cmd_hr, cmd_min, cmd_sec;

  // Check to see if we need to turn the IMU on or off
  if (display_mode == MODE_MAG_HEADING || display_mode ==  MODE_GYRO_HEADING)
    {
      if (!IMU_is_on)
	{
	  //poweron_IMU();
	  imu_wake();
	  IMU_is_on = 1;
	}

      update_imu();
    }
  else if (IMU_is_on)
    {
      //poweroff_IMU();
      imu_sleep();
      IMU_is_on = 0;
    }

  
  // All command modes must return commands in degrees
  switch(display_mode){

  case MODE_SET_CLOCK:
  case MODE_SET_CLOCK_PHASE:
  case MODE_SET_CLOCK_PERIOD:
    clock_update(&clock_0, tv_rtc);
    
    cmd_hr = clock_0.theta_0 * RAD2DEG;
    cmd_min = clock_0.theta_1 * RAD2DEG;
    break;

  case MODE_TRADITIONAL_TIME:
    get_cmd_traditional_time(&cmd_hr, &cmd_min);
    break;

  case MODE_MAG_HEADING:    
    get_cmd_heading_mag(&cmd_hr, &cmd_min);
    break;
    
  case MODE_GYRO_HEADING:
    get_cmd_heading_gyro(&cmd_hr, &cmd_min);
    break;

#ifdef MPL
  case MODE_PRESSURE_ALT:
    //TODO: Get current pressure altitude
    break;

  case MODE_TEMPERATURE:
    //TODO: Get current temperature
    break;
#endif


  case MODE_SET_HR_HAND:
  case MODE_SET_MIN_HAND:
  case MODE_SET_SEC_HAND:
    cmd_hr = 0;
    cmd_min = 0;
    cmd_sec = 0;
    break;
  }

  // Motors accept commands as integer degrees, so cast to int
  motor_set_cmd(&hour_motor, (int)cmd_hr );
  motor_set_cmd(&min_motor, (int)cmd_min );

  // Second hand indicates mode
  cmd_sec = mode_indicator_angle_map[display_mode];
  motor_set_cmd(&sec_motor, (int)cmd_sec );
}




void update_rtc_time(){
  static uint8_t seconds_last = 0;
  static uint8_t hundredths_last = 0;
  
  if (rtc.updateTime() == false) //Updates the time variables from RTC
  {
    Serial.println("RTC failed");
  }
  
  tmSet.Year = rtc.getYear() + 2000 - 1970;   // rtc reports years since 2000, need 1970 epoch
  tmSet.Month = rtc.getMonth();
  tmSet.Day = rtc.getDate();
  tmSet.Hour = rtc.getHours();
  tmSet.Minute = rtc.getMinutes();
  tmSet.Second = rtc.getSeconds();

  uint8_t rtc_hundredths = rtc.getHundredths();
  
  tv_rtc.tv_sec = makeTime(tmSet);
  tv_rtc.tv_usec = (unsigned long)rtc_hundredths * 10000;

  // Check if seconds has rolled over (hundredths and seconds aren't synched?)
  /*if (tmSet.Second == seconds_last && rtc_hundredths < hundredths_last){
    tv_rtc.tv_sec += 1;
    }*/

  seconds_last = tmSet.Second;
  hundredths_last = rtc_hundredths;
}



unsigned long get_ms_to_next_event(){
  unsigned long t_sleep_ms = 50000;
  unsigned long temp;
  
  float next_theta;
  Timeval tv_to_next_tick;

  

  // Check hour motor tick
  unsigned long t_millis;
  t_millis = millis();
  if ( hour_motor.state != IDLE ){
    temp = t_millis - hour_motor.t_tick_start_ms;
    temp = temp >= SOPROD_TTICK_MS ? 0 : SOPROD_TTICK_MS - temp;
    
    t_sleep_ms = temp < t_sleep_ms ? temp : t_sleep_ms;
  }
  
  // Check min motor tick
  if ( min_motor.state != IDLE ){
    temp = t_millis - min_motor.t_tick_start_ms;
    temp = temp >= SOPROD_TTICK_MS ? 0 : SOPROD_TTICK_MS - temp;
    
    t_sleep_ms = temp < t_sleep_ms ? temp : t_sleep_ms;
  }
  
  // Check sec motor tick
  if ( sec_motor.state != IDLE ){
    temp = t_millis - sec_motor.t_tick_start_ms;
    temp = temp >= SOPROD_TTICK_MS ? 0 : SOPROD_TTICK_MS - temp;
    
    t_sleep_ms = temp < t_sleep_ms ? temp : t_sleep_ms;
  }

  // Check if any buttons are pressed
  if ( buttons[BTN_MODE].btn_state || buttons[BTN_INCR].btn_state || buttons[BTN_DECR].btn_state ){
    t_sleep_ms = BUTTON_PRESSED_SLEEP_MS < t_sleep_ms ? BUTTON_PRESSED_SLEEP_MS : t_sleep_ms;
  }
  
  // Check next clock tick start
  switch(display_mode){

  case MODE_SET_CLOCK:
  case MODE_SET_CLOCK_PHASE:
  case MODE_SET_CLOCK_PERIOD:   
    // Get current angles
    
    // Find time at next tick (one degree more)
    next_theta = (clock_0.theta_0 + clock_0.theta_1)/2.0f + 1.0f*DEG2RAD;

    tv_to_next_tick = get_time_at_next_theta(&clock_0, next_theta);

    // Convert to ms
    temp = tv_to_next_tick.tv_sec*1000 + tv_to_next_tick.tv_usec/1000;
    
    t_sleep_ms = temp < t_sleep_ms ? temp : t_sleep_ms;
    break;
    
  case MODE_TRADITIONAL_TIME:
    temp = 60000;
    t_sleep_ms = temp < t_sleep_ms ? temp : t_sleep_ms;
    break;
  case MODE_MAG_HEADING:
  case MODE_GYRO_HEADING:
    temp = 0;
    t_sleep_ms = temp < t_sleep_ms ? temp : t_sleep_ms;
    break;
    
  case MODE_PRESSURE_ALT:
  case MODE_TEMPERATURE:
  case MODE_SET_HR_HAND:
  case MODE_SET_MIN_HAND:
  case MODE_SET_SEC_HAND:
    temp = 100;
    t_sleep_ms = temp < t_sleep_ms ? temp : t_sleep_ms;
    break;
  }

  return t_sleep_ms;
}



// Tell RTC to remove power from the IMU
void poweroff_IMU(void)
{
  /* From pg. 68 of the RV1805 user manual:

    If the PSWS field is set to the initial value of 7, the PSW pin will be driven with 
    the static value of the PSWB bit which is initially zero. If this pin is used as the 
    power switch, setting PSWB will remove power from the system and may prevent further 
    access to the RV-1805-C3. In order to insure that this does not happen inadvertently, 
    the LKP bit must be cleared in order to change the PSWB bit to a 1.

  Killing power to the IMU gets us from 14.9 to 6.5uA sleep current (sleep current of the IMU is 8.4uA)
  */
  //Serial.println("Telling the RTC to power down the IMU...");
  rtc.setPowerSwitchLock(false);
  delay(100);
  rtc.setStaticPowerSwitchOutput(true);  // Pushes PSW high, starving IMU of ground.
  delay(100);
  rtc.setPowerSwitchLock(true);
  
  IMU_is_on = false;    // Update flag
  return;
}


// Tell RTC to begin powering the IMU
void poweron_IMU(void)
{
  /* From pg. 68 of the RV1805 user manual:

    If the PSWS field is set to the initial value of 7, the PSW pin will be driven with 
    the static value of the PSWB bit which is initially zero. If this pin is used as the 
    power switch, setting PSWB will remove power from the system and may prevent further 
    access to the RV-1805-C3. In order to insure that this does not happen inadvertently, 
    the LKP bit must be cleared in order to change the PSWB bit to a 1.

  Killing power to the IMU gets us from 14.9 to 6.5uA sleep current (sleep current of the IMU is 8.4uA)
  */
  //Serial.println("Telling the RTC to power up the IMU...");
  rtc.setPowerSwitchLock(false);
  delay(100);
  rtc.setStaticPowerSwitchOutput(false);  // Pulls PSW low, providng IMU with ground.
  delay(100);
  rtc.setPowerSwitchLock(true);
  
  IMU_is_on = true;    // Update flag
}



// Command Functions -----------------------------------------------------------
void get_cmd_traditional_time(float* p_cmd_hr, float* p_cmd_min)
{
  uint16_t minutes = (tmSet.Minute + traditional_time_offset_minutes) % 60;
  uint8_t hours = (tmSet.Hour + (tmSet.Minute +traditional_time_offset_minutes) / 60) % 12;
  *p_cmd_min = minutes * 6.0f + tmSet.Second * 0.1f;
  *p_cmd_hr = hours * 30.0f + *p_cmd_min * 0.083f;
  Serial.print(tmSet.Hour);
  Serial.print(" ");
  Serial.print(tmSet.Minute);
  Serial.print(" ");
}

void get_cmd_heading_gyro(float* p_cmd_hr, float* p_cmd_min){
  static float theta = 0.0f;
  static unsigned long millis_last = 0;
  float dt_s;

  if (millis_last == 0) millis_last = millis();

  dt_s = (float)(millis() - millis_last) / 1000.0f;
  millis_last = millis();
  theta += myIMU.gz * dt_s;

  *p_cmd_hr = theta;
  *p_cmd_min = theta;
}

void get_cmd_heading_mag(float* p_cmd_hr, float* p_cmd_min){
  float heading;
  
  heading = (atan2( myIMU.my, myIMU.mx ) * RAD2DEG) + 180.0f;  // Gives +/-180 deg

  //heading = filter( heading );

  *p_cmd_hr = heading;
  *p_cmd_min = heading;
}
//------------------------------------------------------------------------------



void check_buttons()
{
  // If MODE pressed, increment mode
  if ( check_button_pressed(&buttons[BTN_MODE]) ){
    //Serial.println("MODE PRESSED");
    mode_pressed();
  }

  // If INCR pressed, increment variable indicted by mode
  if ( check_button_pressed(&buttons[BTN_INCR]) ){
    //Serial.println("INCR PRESSED");
    incr_pressed();
  }
  
  // If DECR pressed, decrement variable indicated by mode
  if ( check_button_pressed(&buttons[BTN_DECR]) ){
    //Serial.println("DECR PRESSED");
    decr_pressed();
  }
}

void button_pressed_isr()
{
  check_buttons();
}


void mode_pressed(){
  switch(display_mode){

  case MODE_SET_CLOCK:
    display_mode = MODE_SET_CLOCK_PHASE;
    
  case MODE_SET_CLOCK_PHASE:
    display_mode = MODE_SET_CLOCK_PERIOD;
    break;
      
  case MODE_SET_CLOCK_PERIOD:
    display_mode = MODE_TRADITIONAL_TIME;
    break;

  case MODE_TRADITIONAL_TIME:
    display_mode = MODE_GYRO_HEADING;

  case MODE_MAG_HEADING:
    display_mode = MODE_GYRO_HEADING;
    break;
    
  case MODE_GYRO_HEADING:
    display_mode = MODE_SET_HR_HAND;
    break;

  case MODE_PRESSURE_ALT:
    display_mode = MODE_TEMPERATURE;
    break;

  case MODE_TEMPERATURE:
    display_mode = MODE_SET_HR_HAND;
    break;

  case MODE_SET_HR_HAND:
    display_mode = MODE_SET_MIN_HAND;
    break;

  case MODE_SET_MIN_HAND:
    display_mode = MODE_SET_SEC_HAND;
    break;

  case MODE_SET_SEC_HAND:
    display_mode = MODE_SET_CLOCK_PHASE;
    break;
  }
}


void incr_pressed(){
  switch(display_mode){
    
  case MODE_SET_CLOCK:
    break;
    
  case MODE_SET_CLOCK_PHASE:
    //clock_0.t_0 *= 0.9;
    break;
    
  case MODE_SET_CLOCK_PERIOD:
    //clock_0.period *= 0.9;
    break;

  case MODE_TRADITIONAL_TIME:
    traditional_time_offset_minutes += 1;
    break;

  case MODE_MAG_HEADING:
    break;
    
  case MODE_GYRO_HEADING:
    break;

  case MODE_PRESSURE_ALT:
    break;
    
  case MODE_TEMPERATURE:
    break;

  case MODE_SET_HR_HAND:
    hour_motor.angle_meas_deg = (hour_motor.angle_meas_deg + 1) % SOPROD_TICKS_PER_REV;
    break;

  case MODE_SET_MIN_HAND:
    min_motor.angle_meas_deg = (min_motor.angle_meas_deg + 1) % SOPROD_TICKS_PER_REV;
    break;

  case MODE_SET_SEC_HAND:
    sec_motor.angle_meas_deg = (sec_motor.angle_meas_deg + 1) % SOPROD_TICKS_PER_REV;
    break;
  }
}


void decr_pressed(){
  switch(display_mode){
  case MODE_SET_CLOCK:
    break;
    
  case MODE_SET_CLOCK_PHASE:
    //clock_0.t_0 *= 1.1;
    break;
    
  case MODE_SET_CLOCK_PERIOD:
    //clock_0.t_0 *= 1.1;
    break;

  case MODE_TRADITIONAL_TIME:
    traditional_time_offset_minutes -= 1;
    break;

  case MODE_MAG_HEADING:
    break;
    
  case MODE_GYRO_HEADING:
    break;

  case MODE_PRESSURE_ALT:
    break;

  case MODE_TEMPERATURE:
    break;
    

  case MODE_SET_HR_HAND:
    hour_motor.angle_meas_deg = (hour_motor.angle_meas_deg - 1) % SOPROD_TICKS_PER_REV;
    break;

  case MODE_SET_MIN_HAND:
    min_motor.angle_meas_deg = (min_motor.angle_meas_deg - 1) % SOPROD_TICKS_PER_REV;
    break;

  case MODE_SET_SEC_HAND:
    sec_motor.angle_meas_deg = (sec_motor.angle_meas_deg - 1) % SOPROD_TICKS_PER_REV;
    break;
  }
}


float filter(float cmd)
{
  static float state_x = 0.0f;
  static float state_xdot = 0.0f;
  float err;

  err = (cmd - state_x);
  state_xdot += err * 0.1 + -0.025 * state_xdot;
  state_x += state_xdot * 0.01;

  return state_x;
  
}


void console_task()
{
  return;
}


int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


// DEBUG Stuff Below
//------------------------------------------------------------

void cal()
{
  int i;

  Serial.println("Min fwd...");
  for ( i = 0; i < 360; i++)
  {
    Serial.println(i);
    motor_tick_fwd(&min_motor);
    Serial.flush();
  }
  Serial.println("Min rev...");
  for ( i = 0; i < 360; i++)
  {
    motor_tick_rev(&min_motor);
  }

  Serial.println("Hr fwd...");
  for ( i = 0; i < 360; i++)
  {
    motor_tick_fwd(&hour_motor);
  }
  Serial.println("Hr rev...");
  for ( i = 0; i < 360; i++)
  {
    motor_tick_rev(&hour_motor);
  }

  Serial.println("Sec fwd...");
  for ( i = 0; i < 360; i++)
  {
    motor_tick_fwd(&sec_motor);
  }
  Serial.println("Sec rev...");
  for ( i = 0; i < 360; i++)
  {
    motor_tick_rev(&sec_motor);
  }
}


void print_clock(){
  Serial.print(clock_0.t_0.tv_sec);
  Serial.print(".");
  Serial.print(clock_0.t_0.tv_usec);
  Serial.print(", ");
  
  Serial.print(clock_0.t_now.tv_sec);
  Serial.print(".");
  Serial.print(clock_0.t_now.tv_usec);
  Serial.print(", ");
  
  Serial.print(clock_0.t.tv_sec);
  Serial.print(".");
  Serial.print(clock_0.t.tv_usec);
  Serial.print(", ");
  
  Serial.print(clock_0.period.tv_sec);
  Serial.print(".");
  Serial.print(clock_0.period.tv_usec);
  Serial.print(", ");
  
  Serial.print(clock_0.theta_0*RAD_TO_DEG);
  Serial.print(", ");
  Serial.print(clock_0.theta_1*RAD_TO_DEG);
  Serial.print(", ");
}


void tm_out()
{
  const char* DisplayMode_Str[] = {
    [MODE_SET_CLOCK] = "SET_CLOCK",
    [MODE_SET_CLOCK_PHASE] = "SET_CLOCK_PHASE",
    [MODE_SET_CLOCK_PERIOD] = "SET_CLOCK_PERIOD",
    [MODE_TRADITIONAL_TIME] = "TRADITIONAL_TIME",
    [MODE_MAG_HEADING] = "MAG_HEADING",
    [MODE_GYRO_HEADING] = "GYRO_HEADING",
    [MODE_PRESSURE_ALT] = "PRESSURE_ALT",
    [MODE_TEMPERATURE] = "TEMERATURE",
    [MODE_SET_HR_HAND] = "SET_HR_HAND",
    [MODE_SET_MIN_HAND] = "SET_MIN_HAND",
    [MODE_SET_SEC_HAND] = "SET_SEC_HAND",
  }; 
  
  print_clock();
  
    // Print time
  Serial.print("Time: ");
  Serial.print( tv_rtc.tv_sec );
  Serial.print(".");
  Serial.print( tv_rtc.tv_usec );
  Serial.print("  ");
  Serial.print(DisplayMode_Str[display_mode]);
  
  Serial.print("\t Hour: ");
  Serial.print(hour_motor.angle_cmd_deg);
  Serial.print(" / ");
  Serial.print(hour_motor.angle_meas_deg);
  Serial.print("\t Min: ");
  Serial.print(min_motor.angle_cmd_deg);
  Serial.print(" / ");
  Serial.print(min_motor.angle_meas_deg);
  Serial.print("\t Sec: ");
  Serial.print(sec_motor.angle_cmd_deg);
  Serial.print(" / ");
  Serial.print(sec_motor.angle_meas_deg);

  Serial.print("  ");
  Serial.print( buttons[BTN_INCR].btn_state );
  Serial.print("  ");
  Serial.print( buttons[BTN_MODE].btn_state );
  Serial.print("  ");
  Serial.print( buttons[BTN_DECR].btn_state );
  
  Serial.println("");
}
