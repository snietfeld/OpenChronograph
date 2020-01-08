/*
  Prints the time from the RV-1805 Real Time Clock
  By: Andy England
  SparkFun Electronics
  Date: 2/22/2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14642

  This example shows how to print the time fromt he RTC.

  Hardware Connections:
    Attach the Qwiic Shield to your Arduino/Photon/ESP32 or other
    Plug the RTC into the shield (any port)
    Open the serial monitor at 9600 baud
*/

#include <SparkFun_RV1805.h>

RV1805 rtc;

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

  Serial.println("RTC online!");
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

  delay(1000);
}
