/*
  Getting the low battery indicator to send an interrupt on the RV-1805 Real Time Clock
  By: Andy England
  SparkFun Electronics
  Date: 2/22/2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14642

  Set a reference voltage and have the board alert you when the battery has charged above that voltage

  Hardware Connections:
  Attach the Qwiic Shield to your Arduino/Photon/ESP32 or other
  Plug the RTC into the shield (any port)
  Open the serial monitor at 115200 baud
*/

#include <SparkFun_RV1805.h>

RV1805 rtc;

void setup() {

  Wire.begin();

  Serial.begin(9600);
  Serial.println("Battery Interrupt from RTC Example");

  if (rtc.begin() == false) {
    Serial.println("Something went wrong, check wiring");
  }
  rtc.setEdgeTrigger(true);//Sets the EdgeTrigger to true, more information on edgeTrigger and its relation to reference voltage below.
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

  /*******************************************
    The value of edgeTrigger controls whether or not the interrupt is
    triggered by rising above or falling below the reference voltage.
    Different sets of reference voltages are available based on this value.

    edgeTrigger = FALSE; Falling Voltage
    0: 2.5V
    1: 2.1V
    2: 1.8V
    3: 1.4V

    edgeTrigger = TRUE; Rising Voltage
    0: 3.0V
    1: 2.5V
    2: 2.2V
    3: 1.6V
  *******************************************/
  if (rtc.checkBattery(1) == true)
  {
    Serial.print("Battery over 2.5V");
  }
  Serial.println();
  delay(1000);

}
