/*
  Trickle Charging the RV-1805 Real Time Clock
  By: Andy England
  SparkFun Electronics
  Date: 2/22/2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14642

  This example shows how to set the time on the RTC to the compiler time and read it back.

  Hardware Connections:
    Attach the Qwiic Shield to your Arduino/Photon/ESP32 or other
    Plug the RTC into the shield (any port)
    Open the serial monitor at 9600 baud
*/

#include <SparkFun_RV1805.h>

RV1805 rtc;

void setup() {

  Wire.begin();

  Serial.begin(9600);
  Serial.println("Read Time from RTC Example");

  if (rtc.begin() == false) {
    Serial.println("Something went wrong, check wiring");
  }

  rtc.disableTrickleCharge();//Trickle Charge is on by default, so let's turn it off
  //The trickle charge circuit consists of a diode (.3v or .6v drop) in series with a resistor (3kOhm, 6kOhm, or 11kOhm)
  //These are available to pass into the function as DIODE_0_3V, DIODE_0_6V, ROUT_3K, ROUT_6K, ROUT_11K. 
  //The below call of enableTrickleCharge sets our trickle charge circuit up at the fastest possible rate, with a .3V diode and 3kOhm resistor
  //Note that the trickle charger should only be used for charging the supercapacitor. Disable the rickle charger if you've connected a battery.
  rtc.enableTrickleCharge(DIODE_0_3V, ROUT_3K);
  
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
