<p align="center">
  <img height="300" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/oc1/oc1_devkit.jpg">  
</p>

# What's this?
OpenChronograph is an a Arduino-compatible hybrid smartwatch platform built around Soprod micromotors. The majority of hybrid smartwatches available today (Skagen, Fossil) use these micromotors, and many are based on the same Soprod reference design. The OpenChronograph mainboards are intended to be programmable drop-in replacements for a large number of commercial hybrid smartwatches, with a suite of handy sensors. You can also create custom dials using a process described [here](https://github.com/snietfeld/OpenChronograph/tree/master/Dials).

<p align="center">
  <img height="300" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/oc1/oc1_board.PNG"> <img height="300" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/oc1/oc1_schematic.PNG">    
</p>

Platform Features:
 - Arduino-compatible Atmega328p microcontroller
 - Ultra-low-power RV-1805-C3 real-time clock
 - Precision MPU-9250 6DOF IMU + 3-axis magnetometer
 - High-precision MPL3115A2 pressure/altitude/temperature sensor
 - Support for both dual- and single-rotor micromoters (for a total of three hands)
 - Forward and retrograde hand motion
 - Custom reduced-footprint pogo-pin programmer
 
Confirmed Drop-in Compatible Watches:
 - Skagen Connected
 - Skagen Hagen
 - Fossil Q Activist
 - Fossil Q Men's Machine

# Yes, but why?
Hybrid smartwatches are neat, and they can offer fantastic battery life compared to modern smartwatches. Unfortunately, experimenting with them is difficult due to the closed nature of available designs. This project is intended to open up hybrid smartwatch design to the maker community.

# Getting started.
## What you’ll need
 - A Torx T2 driver
 - A small flathead driver
 - Watch hand pullers
 - Tweezers 
 
## Watch disassembly and board swap
[![Alt text](https://img.youtube.com/vi/Nqi1F6wrvCc/0.jpg)](https://youtu.be/Nqi1F6wrvCc) 

## Arduino IDE Set Up
Download the Arduino IDE (tested with 1.8.9)

Clone or download this repository.

### Add the board profile
From the downloaded repository, copy the \OpenChronograph\MainBoards\oc_1\hw\Arduino_OpenChronograph_OC1 folder to your Arduino hardware folder, e.g., "C:\Program Files (x86)\Arduino\hardware\"

You should now see "OpenChronograph OC-1 (8MHz internal clock)" available in your Tools-->Board list in the Arduino IDE.

WARNING: If you skip this step, you can brick your microcontroller.

## Programming
Open the \OpenChronograph\MainBoards\oc_1\fw\oc_1_default\oc-1-default.ino project file

Select Tools --> Board --> OpenChronograph OC-1 (8MHz internal clock)

Select Tools --> Programmer --> USBtinyISP

Compile & upload using the MicroPogo programmer
<p align="center">
  <img height="300" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/oc1/oc1_programmer.jpg">  
</p>
 
 ## MicroPogo Programmer
 The MicroPogo programmer is a small-footprint Arduino serial programmer. The footprint is derived from the Protofusion [ProtoProg Programmer](http://protofusion.org/wordpress/2013/05/open-hardware-pogo-pin-programmer/), and the board is derived from TinyCircuit's [TinyLilly Mini USB Adadpter](https://tinycircuits.com/products/tinylily-mini-usb-adapter).
