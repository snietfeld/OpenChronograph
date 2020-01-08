/******************************************************************************
SparkFun_RV1805.h
RV1805 Arduino Library
Andy England @ SparkFun Electronics
February 5, 2018
https://github.com/sparkfun/Qwiic_RTC

Resources:
Uses Wire.h for i2c operation
Uses SPI.h for SPI operation

Development environment specifics:
Arduino IDE 1.6.4

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.
******************************************************************************/

#pragma once

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>

//The 7-bit I2C address of the RV1805
#define RV1805_ADDR						(uint8_t)0x69

//The upper part of the part number is always 0x18
#define RV1805_PART_NUMBER_UPPER		0x18

//Possible CONFKEY Values
#define RV1805_CONF_RST					0x3C //value written to Configuration Key for reset
#define RV1805_CONF_OSC					0xA1 //value written to Configuration Key for oscillator control register write enable
#define RV1805_CONF_WRT					0x9D //value written to Configuration Key to enable write of trickle charge, BREF, CAPRC, IO Batmode, and Output Control Registers

//Bits in Control1 Register
#define CTRL1_STOP	7
#define CTRL1_12_24	6
#define CTRL1_PSWB	5
#define CTRL1_ARST						1 << 2 //Enables reset of interrupt flags in status register 

//Bits in Hours register
#define HOURS_AM_PM						5

//Trickle Charge Control
#define TRICKLE_CHARGER_TCS_OFFSET				4
#define TRICKLE_CHARGER_DIODE_OFFSET			2
#define TRICKLE_CHARGER_ROUT_OFFSET			0
#define TRICKLE_ENABLE					0b1010
#define TRICKLE_DISABLE					0b0000
#define DIODE_DISABLE 					0b00
#define DIODE_0_3V						0b01
#define DIODE_0_6V						0b10
#define ROUT_DISABLE					0b00
#define ROUT_3K							0b01
#define ROUT_6K							0b10
#define ROUT_11K						0b11

//Interrupt Enable Bits
#define INTERRUPT_BLIE	4
#define INTERRUPT_TIE	3
#define INTERRUPT_AIE	2
#define INTERRUPT_EIE	1


//PSW Pin Function Selection Bits
#define PSWS_OFFSET     2
#define PSWS_INV_IRQ    0b000
#define PSWS_SQW        0b001
#define PSWS_INV_AIRQ   0b011
#define PSWS_TIRQ       0b100
#define PSWS_INV_TIRQ   0b101
#define PSWS_SLEEP      0b110
#define PSWS_STATIC     0b111

//Countdown Timer Control
#define COUNTDOWN_SECONDS		0b10
#define COUNTDOWN_MINUTES		0b11
#define CTDWN_TMR_TE_OFFSET		7
#define CTDWN_TMR_TM_OFFSET		6
#define CTDWN_TMR_TRPT_OFFSET	5


//Status Bits
#define STATUS_CB	7
#define STATUS_BAT 6
#define STATUS_WDF 5
#define STATUS_BLF 4
#define STATUS_TF 3
#define STATUS_AF 2
#define STATUS_EVF 1

//Reference Voltage
#define TWO_FIVE						0x70
#define TWO_ONE							0xB0
#define ONE_EIGHT						0xD0
#define ONE_FOUR						0xF0

//Register names:
#define RV1805_HUNDREDTHS               0x00
#define RV1805_SECONDS      			0x01
#define RV1805_MINUTES      			0x02
#define RV1805_HOURS        			0x03
#define RV1805_DATE         			0x04
#define RV1805_MONTHS        			0x05
#define RV1805_YEARS        			0x06
#define RV1805_WEEKDAYS      			0x07
#define RV1805_HUNDREDTHS_ALM           0x08
#define RV1805_SECONDS_ALM    			0x09
#define RV1805_MINUTES_ALM     			0x0A
#define RV1805_HOURS_ALM       			0x0B
#define RV1805_DATE_ALM        			0x0C
#define RV1805_MONTHS_ALM      			0x0D
#define RV1805_WEEKDAYS_ALM    			0x0E
#define RV1805_STATUS					0x0F
#define RV1805_CTRL1					0x10
#define RV1805_CTRL2					0x11
#define RV1805_INT_MASK					0x12
#define RV1805_SQW						0x13
#define RV1805_CAL_XT					0x14
#define RV1805_CAL_RC_UP				0x15
#define RV1805_CAL_RC_LO				0x16
#define RV1805_SLP_CTRL					0x17
#define RV1805_CTDWN_TMR_CTRL			0x18
#define RV1805_CTDWN_TMR				0x19
#define RV1805_TMR_INITIAL				0x1A
#define RV1805_WATCHDOG_TMR				0x1B
#define RV1805_OSC_CTRL					0x1C
#define RV1805_OSC_STATUS				0x1D
#define RV1805_CONF_KEY					0x1F
#define RV1805_TRICKLE_CHRG				0x20
#define RV1805_BREF_CTRL				0x21
#define RV1805_CAP_RC					0x26
#define RV1805_IOBATMODE				0x27
#define RV1805_ID0						0x28
#define RV1805_ANLG_STAT				0x2F
#define RV1805_OUT_CTRL					0x30
#define RV1805_RAM_EXT					0x3F

#define TIME_ARRAY_LENGTH 8 // Total number of writable values in device

enum time_order {
	TIME_HUNDREDTHS, // 0
	TIME_SECONDS,    // 1
	TIME_MINUTES,    // 2
	TIME_HOURS,      // 3
	TIME_DATE,       // 4
	TIME_MONTH,      // 5
	TIME_YEAR,       // 6
	TIME_DAY,	     // 7
};

class RV1805
{
  public:
	
    RV1805( void );

    boolean begin( TwoWire &wirePort = Wire);

	bool setTime(uint8_t hund, uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t month, uint8_t year, uint8_t day);
	bool setTime(uint8_t * time, uint8_t len);
	bool setHundredths(uint8_t value);
	bool setSeconds(uint8_t value);
	bool setMinutes(uint8_t value);
	bool setHours(uint8_t value);
	bool setWeekday(uint8_t value);
	bool setDate(uint8_t value);
	bool setMonth(uint8_t value);
	bool setYear(uint8_t value);
	
	bool updateTime(); //Update the local array with the RTC registers
	
	char* stringDateUSA(); //Return date in mm-dd-yyyy
	char* stringDate(); //Return date in dd-mm-yyyy
	char* stringTime(); //Return time hh:mm:ss with AM/PM if in 12 hour mode
	char* stringTimeStamp(); //Return timeStamp in ISO 8601 format yyyy-mm-ddThh:mm:ss
	
	uint8_t getHundredths();
	uint8_t getSeconds();
	uint8_t getMinutes();
	uint8_t getHours();
	uint8_t getWeekday();
	uint8_t getDate();
	uint8_t getMonth();
	uint8_t getYear();	
	
	bool setToCompilerTime(); //Uses the hours, mins, etc from compile time to set RTC
	
	bool is12Hour(); //Returns true if 12hour bit is set
	bool isPM(); //Returns true if is12Hour and PM bit is set
	void set12Hour();
	void set24Hour();
	
	uint8_t status(); //Returns the status byte
	
	bool setAlarm(uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t month);
	bool setAlarm(uint8_t * time, uint8_t len);
	void setAlarmMode(uint8_t mode); //0 to 7, alarm goes off with match of second, minute, hour, etc
 
 	void enableSleep();
    void setPowerSwitchFunction(uint8_t function);
    void setPowerSwitchLock(bool lock);
    void setStaticPowerSwitchOutput(bool psw); // PSW pin must be unlocked using setPSWLock(false) to enable static PSW output
	
	void setCountdownTimer(uint8_t duration, uint8_t unit, bool repeat = true, bool pulse = true);

	void enableTrickleCharge(uint8_t diode = DIODE_0_3V, uint8_t rOut = ROUT_3K); //Diode default 0.3V, rOut default 3k
	void disableTrickleCharge();
	void enableLowPower();

	void enableInterrupt(uint8_t source); //Enables a given interrupt within Interrupt Enable register
	void disableInterrupt(uint8_t source); //Disables a given interrupt within Interrupt Enable register
	void enableBatteryInterrupt(uint8_t voltage, bool edgeTrigger);
	void enableAlarmInterrupt(); //Use in conjuction with setAlarm and setAlarmMode
	
	void clearInterrupts();
	
	bool checkBattery(uint8_t voltage);
	void setEdgeTrigger(bool edgeTrigger);
	void setReferenceVoltage(uint8_t voltage);
	
	//Values in RTC are stored in Binary Coded Decimal. These functions convert to/from Decimal
	uint8_t BCDtoDEC(uint8_t val); 
	uint8_t DECtoBCD(uint8_t val);

	void reset(void); //Fully reset RTC to all zeroes
	
    uint8_t readRegister(uint8_t addr);
    bool writeRegister(uint8_t addr, uint8_t val);
	bool readMultipleRegisters(uint8_t addr, uint8_t * dest, uint8_t len);
	bool writeMultipleRegisters(uint8_t addr, uint8_t * values, uint8_t len);

private:
	uint8_t _time[TIME_ARRAY_LENGTH];
	TwoWire *_i2cPort;
};
