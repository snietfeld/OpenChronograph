/*
  Setting time from the RV-1805 Real Time Clock
  By: Andy England
  SparkFun Electronics
  Date: 2/22/2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14642

  This example shows how to set the time on the RTC to the compiler time or a custom time.

  Hardware Connections:
    Attach the Qwiic Shield to your Arduino/Photon/ESP32 or other
    Plug the RTC into the shield (any port)
    Open the serial monitor at 9600 baud
*/

#include <SparkFun_RV1805.h>

RV1805 rtc;

//The below variables control what the date will be set to
int hund = 50;
int sec = 2;
int minute = 18;
int hour = 7;
int date = 25;
int month = 6;
int year = 2018;
int day = 5;

void setup() {

  Wire.begin();

  Serial.begin(9600);
  Serial.println("Read Time from RTC Example");

  if (rtc.begin() == false) {
    Serial.println("Something went wrong, check wiring");
  }

  //Use the time from the Arduino compiler (build time) to set the RTC
  //Keep in mind that Arduino does not get the new compiler time every time it compiles. to ensure the proper time is loaded, open up a fresh version of the IDE and load the sketch.
  if (rtc.setToCompilerTime() == false) {
    Serial.println("Something went wrong setting the time");
  }
  
  //Uncomment the below code to set the RTC to your own time
  /*if (rtc.setTime(hund, sec, minute, hour, date, month, year, day) == false) {
    Serial.println("Something went wrong setting the time");
  }*/
  
  Serial.println("RTC online!");
}

void loop() {

}
