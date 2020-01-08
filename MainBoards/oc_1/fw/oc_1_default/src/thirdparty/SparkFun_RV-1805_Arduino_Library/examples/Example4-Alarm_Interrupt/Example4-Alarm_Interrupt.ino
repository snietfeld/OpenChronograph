/*
  Getting the alarm to fire an interrupt on the RV-1805 Real Time Clock
  By: Andy England
  SparkFun Electronics
  Date: 2/22/2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14642

  This example shows how to set an alarm and make the RTC generate an interrupt when the clock time matches the alarm time
  The INT pin will be 3.3V. When the real time matches the alarm time the INT pin will go low.

  Hardware Connections:
    Attach the Qwiic Shield to your Arduino/Photon/ESP32 or other
    Plug the RTC into the shield (any port)
    Open the serial monitor at 115200 baud
*/

#include <SparkFun_RV1805.h>

RV1805 rtc;

byte secondsAlarm = 0;
byte minuteAlarm = 0;
byte hourAlarm = 0;
byte dateAlarm = 0;
byte monthAlarm = 0;

void setup() {

  Wire.begin();

  Serial.begin(9600);
  Serial.println("Alarm from RTC Example");

  if (rtc.begin() == false) {
    Serial.println("Something went wrong, check wiring");
  }

  rtc.setAlarm(secondsAlarm, minuteAlarm, hourAlarm, dateAlarm, monthAlarm); //Sets the alarm with the values initialized above

  /********************************
    Mode must be between 0 and 7 to tell when the alarm should be triggered.
    Alarm is triggered when listed characteristics match
    0: Disabled
    1: seconds, minutes, hours, date and month match (once per year)
    2: seconds, minutes, hours and date match (once per month)
    3: seconds, minutes, hours and weekday match (once per week)
    4: seconds, minutes and hours match (once per day)
    5: seconds and minutes match (once per hour)
    6: seconds match (once per minute)
    7: once per second
  ********************************/
  //rtc.setAlarmMode(7); //Once per second
  //rtc.setAlarmMode(6); //Once per minute
  //rtc.setAlarmMode(5); //Once per hour: Alarm will go off every time there is a hundredths+seconds+minutes match (each hour)
  rtc.setAlarmMode(4); //Alarm goes off every day
  rtc.enableInterrupt(INTERRUPT_AIE); //Enable the Alarm Interrupt
}

void loop() {
  if (rtc.updateTime() == false) //Updates the time variables from RTC
  {
    Serial.print("RTC failed to update");
  }

  String currentDate = rtc.stringDateUSA(); //Get the current date in mm/dd/yyyy format (we're weird)
  //String currentDate = rtc.stringDate()); //Get the current date in dd/mm/yyyy format
  String currentTime = rtc.stringTime(); //Get the time

  Serial.print(currentDate);
  Serial.print(" ");
  Serial.println(currentTime);

  /*byte rtcStatus = rtc.status(); //Get the latest status from RTC. Reading the status clears the flags
  if(rtcStatus != 0)
  {
    Serial.println("An interrupt as occured: ");
    //Determine which bits are set
    if(rtcStatus & (1<<STATUS_CB)) Serial.println("It's a new century!");
    if(rtcStatus & (1<<STATUS_BAT)) Serial.println("System is on backup battery");
    if(rtcStatus & (1<<STATUS_WDF)) Serial.println("Watchdog timer trigger");
    if(rtcStatus & (1<<STATUS_BLF)) Serial.println("Battery is below threshold");
    if(rtcStatus & (1<<STATUS_TF)) Serial.println("Countdown timer at zero");
    if(rtcStatus & (1<<STATUS_AF)) Serial.println("Alarm went off!");
    if(rtcStatus & (1<<STATUS_EVF)) Serial.println("External event detected");
  }*/

  delay(1000);
}
