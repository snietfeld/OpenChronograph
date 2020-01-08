/******************************************************************************
SparkFun_RV1805.h
RV1805 Arduino Library
Andy England @ SparkFun Electronics
February 5, 2018
https://github.com/sparkfun/Qwiic_RTC

Development environment specifics:
Arduino IDE 1.6.4

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.
******************************************************************************/

#include "SparkFun_RV1805.h"

//****************************************************************************//
//
//  Settings and configuration
//
//****************************************************************************//

// Parse the __DATE__ predefined macro to generate date defaults:
// __Date__ Format: MMM DD YYYY (First D may be a space if <10)
// <MONTH>
#define BUILD_MONTH_JAN ((__DATE__[0] == 'J') && (__DATE__[1] == 'a')) ? 1 : 0
#define BUILD_MONTH_FEB (__DATE__[0] == 'F') ? 2 : 0
#define BUILD_MONTH_MAR ((__DATE__[0] == 'M') && (__DATE__[1] == 'a') && (__DATE__[2] == 'r')) ? 3 : 0
#define BUILD_MONTH_APR ((__DATE__[0] == 'A') && (__DATE__[1] == 'p')) ? 4 : 0
#define BUILD_MONTH_MAY ((__DATE__[0] == 'M') && (__DATE__[1] == 'a') && (__DATE__[2] == 'y')) ? 5 : 0
#define BUILD_MONTH_JUN ((__DATE__[0] == 'J') && (__DATE__[1] == 'u') && (__DATE__[2] == 'n')) ? 6 : 0
#define BUILD_MONTH_JUL ((__DATE__[0] == 'J') && (__DATE__[1] == 'u') && (__DATE__[2] == 'l')) ? 7 : 0
#define BUILD_MONTH_AUG ((__DATE__[0] == 'A') && (__DATE__[1] == 'u')) ? 8 : 0
#define BUILD_MONTH_SEP (__DATE__[0] == 'S') ? 9 : 0
#define BUILD_MONTH_OCT (__DATE__[0] == 'O') ? 10 : 0
#define BUILD_MONTH_NOV (__DATE__[0] == 'N') ? 11 : 0
#define BUILD_MONTH_DEC (__DATE__[0] == 'D') ? 12 : 0
#define BUILD_MONTH BUILD_MONTH_JAN | BUILD_MONTH_FEB | BUILD_MONTH_MAR | \
BUILD_MONTH_APR | BUILD_MONTH_MAY | BUILD_MONTH_JUN | \
BUILD_MONTH_JUL | BUILD_MONTH_AUG | BUILD_MONTH_SEP | \
BUILD_MONTH_OCT | BUILD_MONTH_NOV | BUILD_MONTH_DEC
// <DATE>
#define BUILD_DATE_0 ((__DATE__[4] == ' ') ? 0 : (__DATE__[4] - 0x30))
#define BUILD_DATE_1 (__DATE__[5] - 0x30)
#define BUILD_DATE ((BUILD_DATE_0 * 10) + BUILD_DATE_1)
// <YEAR>
#define BUILD_YEAR (((__DATE__[7] - 0x30) * 1000) + ((__DATE__[8] - 0x30) * 100) + \
((__DATE__[9] - 0x30) * 10)  + ((__DATE__[10] - 0x30) * 1))

// Parse the __TIME__ predefined macro to generate time defaults:
// __TIME__ Format: HH:MM:SS (First number of each is padded by 0 if <10)
// <HOUR>
#define BUILD_HOUR_0 ((__TIME__[0] == ' ') ? 0 : (__TIME__[0] - 0x30))
#define BUILD_HOUR_1 (__TIME__[1] - 0x30)
#define BUILD_HOUR ((BUILD_HOUR_0 * 10) + BUILD_HOUR_1)
// <MINUTE>
#define BUILD_MINUTE_0 ((__TIME__[3] == ' ') ? 0 : (__TIME__[3] - 0x30))
#define BUILD_MINUTE_1 (__TIME__[4] - 0x30)
#define BUILD_MINUTE ((BUILD_MINUTE_0 * 10) + BUILD_MINUTE_1)
// <SECOND>
#define BUILD_SECOND_0 ((__TIME__[6] == ' ') ? 0 : (__TIME__[6] - 0x30))
#define BUILD_SECOND_1 (__TIME__[7] - 0x30)
#define BUILD_SECOND ((BUILD_SECOND_0 * 10) + BUILD_SECOND_1)

RV1805::RV1805( void )
{

}

boolean RV1805::begin(TwoWire &wirePort)
{
	//We require caller to begin their I2C port, with the speed of their choice
	//external to the library
	//_i2cPort->begin();
	_i2cPort = &wirePort;

	uint8_t sensorPartNumber = readRegister(RV1805_ID0);
	if (sensorPartNumber != RV1805_PART_NUMBER_UPPER) //HW version for RV1805
		return(false); //Something went wrong. IC didn't respond.

	enableTrickleCharge();
	enableLowPower();

	uint8_t setting = readRegister(RV1805_CTRL1);
	setting |= CTRL1_ARST; //Enables clearing of interrupt flags upon read of status register
	writeRegister(RV1805_CTRL1, setting);

	set12Hour();

	return(true);
}

//Configure RTC to output 1-12 hours
//Converts any current hour setting to 12 hour
void RV1805::set12Hour()
{
	//Do we need to change anything?
	if(is12Hour() == false)
	{		
		uint8_t hour = BCDtoDEC(readRegister(RV1805_HOURS)); //Get the current hour in the RTC

		//Set the 12/24 hour bit
		uint8_t setting = readRegister(RV1805_CTRL1);
		setting |= (1<<CTRL1_12_24);
		writeRegister(RV1805_CTRL1, setting);

		//Take the current hours and convert to 12, complete with AM/PM bit
		boolean pm = false;

		if(hour == 0)
			hour += 12;
		else if(hour == 12)
			pm = true;
		else if(hour > 12)
		{
			hour -= 12;
			pm = true;
		}
		
		hour = DECtoBCD(hour); //Convert to BCD

		if(pm == true) hour |= (1<<HOURS_AM_PM); //Set AM/PM bit if needed

		writeRegister(RV1805_HOURS, hour); //Record this to hours register
	}
}

//Configure RTC to output 0-23 hours
//Converts any current hour setting to 24 hour
void RV1805::set24Hour()
{
	//Do we need to change anything?
	if(is12Hour() == true)
	{		
		//Not sure what changing the CTRL1 register will do to hour register so let's get a copy
		uint8_t hour = readRegister(RV1805_HOURS); //Get the current 12 hour formatted time in BCD
		boolean pm = false;
		if(hour & (1<<HOURS_AM_PM)) //Is the AM/PM bit set?
		{
			pm = true;
			hour &= ~(1<<HOURS_AM_PM); //Clear the bit
		}
		
		//Change to 24 hour mode
		uint8_t setting = readRegister(RV1805_CTRL1);
		setting &= ~(1<<CTRL1_12_24); //Clear the 12/24 hr bit
		writeRegister(RV1805_CTRL1, setting);

		//Given a BCD hour in the 1-12 range, make it 24
		hour = BCDtoDEC(hour); //Convert core of register to DEC
		
		if(pm == true) hour += 12; //2PM becomes 14
		if(hour == 12) hour = 0; //12AM stays 12, but should really be 0
		if(hour == 24) hour = 12; //12PM becomes 24, but should really be 12

		hour = DECtoBCD(hour); //Convert to BCD

		writeRegister(RV1805_HOURS, hour); //Record this to hours register
	}
}

//Returns true if RTC has been configured for 12 hour mode
bool RV1805::is12Hour()
{
	uint8_t controlRegister = readRegister(RV1805_CTRL1);
	return(controlRegister & (1<<CTRL1_12_24));
}

//Returns true if RTC has PM bit set and 12Hour bit set
bool RV1805::isPM()
{
	uint8_t hourRegister = readRegister(RV1805_HOURS);
	if(is12Hour() && (hourRegister & (1<<HOURS_AM_PM)))
		return(true);
	return(false);
}

//Strictly resets. Run .begin() afterwards
void RV1805::reset(void)
{
	writeRegister(RV1805_CONF_KEY, RV1805_CONF_RST); //Writes reset value from datasheet	
}

//Returns the status byte. This likely clears the interrupts as well.
//See .begin() for ARST bit setting
uint8_t RV1805::status(void)
{
	return(readRegister(RV1805_STATUS));
}

//Returns a pointer to array of chars that are the date in mm/dd/yyyy format because we're weird
char* RV1805::stringDateUSA()
{
	static char date[11]; //Max of mm/dd/yyyy with \0 terminator
	sprintf(date, "%02d/%02d/20%02d", BCDtoDEC(_time[TIME_MONTH]), BCDtoDEC(_time[TIME_DATE]), BCDtoDEC(_time[TIME_YEAR]));
	return(date);
}

//Returns a pointer to array of chars that are the date in dd/mm/yyyy format
char*  RV1805::stringDate()
{
	static char date[11]; //Max of dd/mm/yyyy with \0 terminator
	sprintf(date, "%02d/%02d/20%02d", BCDtoDEC(_time[TIME_DATE]), BCDtoDEC(_time[TIME_MONTH]), BCDtoDEC(_time[TIME_YEAR]));
	return(date);
}

//Returns a pointer to array of chars that represents the time in hh:mm:ss format
//Adds AM/PM if in 12 hour mode
char* RV1805::stringTime()
{
	static char time[11]; //Max of hh:mm:ssXM with \0 terminator

	if(is12Hour() == true)
	{
		char half = 'A';
		if(isPM()) half = 'P';
		
		sprintf(time, "%02d:%02d:%02d%cM", BCDtoDEC(_time[TIME_HOURS]), BCDtoDEC(_time[TIME_MINUTES]), BCDtoDEC(_time[TIME_SECONDS]), half);
	}
	else
	sprintf(time, "%02d:%02d:%02d", BCDtoDEC(_time[TIME_HOURS]), BCDtoDEC(_time[TIME_MINUTES]), BCDtoDEC(_time[TIME_SECONDS]));
	
	return(time);
}

char* RV1805::stringTimeStamp()
{
	static char timeStamp[23]; //Max of yyyy-mm-ddThh:mm:ss.ss with \0 terminator

	sprintf(timeStamp, "20%02d-%02d-%02dT%02d:%02d:%02d:%02d", BCDtoDEC(_time[TIME_YEAR]), BCDtoDEC(_time[TIME_MONTH]), BCDtoDEC(_time[TIME_DATE]), BCDtoDEC(_time[TIME_HOURS]), BCDtoDEC(_time[TIME_MINUTES]), BCDtoDEC(_time[TIME_SECONDS]), BCDtoDEC(_time[TIME_HUNDREDTHS]));
	
	return(timeStamp);
}

bool RV1805::setTime(uint8_t hund, uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t month, uint8_t year, uint8_t day)
{
	_time[TIME_HUNDREDTHS] = DECtoBCD(hund);
	_time[TIME_SECONDS] = DECtoBCD(sec);
	_time[TIME_MINUTES] = DECtoBCD(min);
	_time[TIME_HOURS] = DECtoBCD(hour);
	_time[TIME_DATE] = DECtoBCD(date);
	_time[TIME_MONTH] = DECtoBCD(month);
	_time[TIME_YEAR] = DECtoBCD(year);
	_time[TIME_DAY] = DECtoBCD(day);
		
	return setTime(_time, TIME_ARRAY_LENGTH);
}

// setTime -- Set time and date/day registers of RV1805 (using data array)
bool RV1805::setTime(uint8_t * time, uint8_t len)
{
	if (len != TIME_ARRAY_LENGTH)
		return false;
	
	return writeMultipleRegisters(RV1805_HUNDREDTHS, time, len);
}

bool RV1805::setHundredths(uint8_t value)
{
	_time[TIME_HUNDREDTHS] = DECtoBCD(value);
	return setTime(_time, TIME_ARRAY_LENGTH);
}

bool RV1805::setSeconds(uint8_t value)
{
	_time[TIME_SECONDS] = DECtoBCD(value);
	return setTime(_time, TIME_ARRAY_LENGTH);
}

bool RV1805::setMinutes(uint8_t value)
{
	_time[TIME_MINUTES] = DECtoBCD(value);
	return setTime(_time, TIME_ARRAY_LENGTH);
}

bool RV1805::setHours(uint8_t value)
{
	_time[TIME_HOURS] = DECtoBCD(value);
	return setTime(_time, TIME_ARRAY_LENGTH);
}

bool RV1805::setDate(uint8_t value)
{
	_time[TIME_DATE] = DECtoBCD(value);
	return setTime(_time, TIME_ARRAY_LENGTH);
}

bool RV1805::setMonth(uint8_t value)
{
	_time[TIME_MONTH] = DECtoBCD(value);
	return setTime(_time, TIME_ARRAY_LENGTH);
}

bool RV1805::setYear(uint8_t value)
{
	_time[TIME_YEAR] = DECtoBCD(value);
	return setTime(_time, TIME_ARRAY_LENGTH);
}

bool RV1805::setWeekday(uint8_t value)
{
	_time[TIME_DAY] = DECtoBCD(value);
	return setTime(_time, TIME_ARRAY_LENGTH);
}

//Move the hours, mins, sec, etc registers from RV-1805 into the _time array
//Needs to be called before printing time or date
//We do not protect the GPx registers. They will be overwritten. The user has plenty of RAM if they need it.
bool RV1805::updateTime()
{
	if (readMultipleRegisters(RV1805_HUNDREDTHS, _time, TIME_ARRAY_LENGTH) == false)
		return(false); //Something went wrong
	
	if(is12Hour()) _time[TIME_HOURS] &= ~(1<<HOURS_AM_PM); //Remove this bit from value
	
	return true;
}

uint8_t RV1805::getHundredths()
{
	return BCDtoDEC(_time[TIME_HUNDREDTHS]);
}

uint8_t RV1805::getSeconds()
{
	return BCDtoDEC(_time[TIME_SECONDS]);
}

uint8_t RV1805::getMinutes()
{
	return BCDtoDEC(_time[TIME_MINUTES]);
}

uint8_t RV1805::getHours()
{
	return BCDtoDEC(_time[TIME_HOURS]);
}

uint8_t RV1805::getWeekday()
{
	return BCDtoDEC(_time[TIME_DAY]);
}

uint8_t RV1805::getDate()
{
	return BCDtoDEC(_time[TIME_DATE]);
}

uint8_t RV1805::getMonth()
{
	return BCDtoDEC(_time[TIME_MONTH]);
}

uint8_t RV1805::getYear()
{
	return BCDtoDEC(_time[TIME_YEAR]);
}

//Takes the time from the last build and uses it as the current time
//Works very well as an arduino sketch
bool RV1805::setToCompilerTime()
{
	_time[TIME_SECONDS] = DECtoBCD(BUILD_SECOND);
	_time[TIME_MINUTES] = DECtoBCD(BUILD_MINUTE);
	_time[TIME_HOURS] = DECtoBCD(BUILD_HOUR);

	//Build_Hour is 0-23, convert to 1-12 if needed
	if (is12Hour())
	{
		uint8_t hour = BUILD_HOUR;

		boolean pm = false;

		if(hour == 0)
			hour += 12;
		else if(hour == 12)
			pm = true;
		else if(hour > 12)
		{
			hour -= 12;
			pm = true;
		}

		_time[TIME_HOURS] = DECtoBCD(hour); //Load the modified hours
	
		if(pm == true) _time[TIME_HOURS] |= (1<<HOURS_AM_PM); //Set AM/PM bit if needed
	}
	
	_time[TIME_MONTH] = DECtoBCD(BUILD_MONTH);
	_time[TIME_DATE] = DECtoBCD(BUILD_DATE);
	_time[TIME_YEAR] = DECtoBCD(BUILD_YEAR - 2000); //! Not Y2K (or Y2.1K)-proof :(
	
	// Calculate weekday (from here: http://stackoverflow.com/a/21235587)
	// 0 = Sunday, 6 = Saturday
	uint16_t d = BUILD_DATE;
	uint16_t m = BUILD_MONTH;
	uint16_t y = BUILD_YEAR;
	uint16_t weekday = (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7 + 1;
	_time[TIME_DAY] = DECtoBCD(weekday);
	
	return setTime(_time, TIME_ARRAY_LENGTH);
}

bool RV1805::setAlarm(uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t month)
{
	uint8_t alarmTime[TIME_ARRAY_LENGTH];
	
	alarmTime[TIME_HUNDREDTHS] = DECtoBCD(0); //This library assumes we are operating on RC oscillator. Hundredths alarm is not valid in this mode.
	alarmTime[TIME_SECONDS] = DECtoBCD(sec);
	alarmTime[TIME_MINUTES] = DECtoBCD(min);
	alarmTime[TIME_HOURS] = DECtoBCD(hour);
	alarmTime[TIME_DATE] = DECtoBCD(date);
	alarmTime[TIME_MONTH] = DECtoBCD(month);
	alarmTime[TIME_YEAR] = DECtoBCD(0); //Our alarm cannot read these values, so we set them to 0
	alarmTime[TIME_DAY] = DECtoBCD(0);
	
	return setAlarm(alarmTime, TIME_ARRAY_LENGTH);
}

bool RV1805::setAlarm(uint8_t * alarmTime, uint8_t len)
{
	if (len != TIME_ARRAY_LENGTH)
		return false;
	
	return writeMultipleRegisters(RV1805_HUNDREDTHS_ALM, alarmTime, TIME_ARRAY_LENGTH);
}


void RV1805::enableSleep()
{
    uint8_t value;
    value = readRegister(RV1805_SLP_CTRL);
    value |= (1 << 7);
    writeRegister(RV1805_SLP_CTRL, value);
}
 
void RV1805::setPowerSwitchFunction(uint8_t function)
{
    uint8_t value;
    value = readRegister(RV1805_CTRL2);
    //value &= 0b11000011; // Clear PSWS bits
    value &= 0b11100011; // Clear PSWS bits  See table on pg. 13 of app manual
    value |= (function << PSWS_OFFSET);
    writeRegister(RV1805_CTRL2, value);
}
 
void RV1805::setPowerSwitchLock(bool lock)
{
    uint8_t value;
    value = readRegister(RV1805_OSC_STATUS);
    value &= ~(1 << 5);
    value |= (lock << 5);
    writeRegister(RV1805_OSC_STATUS, value);
}
 
void RV1805::setStaticPowerSwitchOutput(bool psw)
{
    uint8_t value;
    value = readRegister(RV1805_CTRL1);
    value &= ~(1 << CTRL1_PSWB);
    value |= (psw << CTRL1_PSWB);
    writeRegister(RV1805_CTRL1, value);
}


/*********************************
Given a bit location, enable the interrupt
INTERRUPT_BLIE	4
INTERRUPT_TIE	3
INTERRUPT_AIE	2
INTERRUPT_EIE	1
*********************************/
void RV1805::enableInterrupt(uint8_t source)
{
	uint8_t value = readRegister(RV1805_INT_MASK);
	value |= (1<<source); //Set the interrupt enable bit
	writeRegister(RV1805_INT_MASK, value);
}

void RV1805::disableInterrupt(uint8_t source)
{
	uint8_t value = readRegister(RV1805_INT_MASK);
	value &= ~(1<<source); //Clear the interrupt enable bit
	writeRegister(RV1805_INT_MASK, value);
}

/********************************
Set Alarm Mode controls which parts of the time have to match for the alarm to trigger.
When the RTC matches a given time, make an interrupt fire.

Mode must be between 0 and 7 to tell when the alarm should be triggered. 
Alarm is triggered when listed characteristics match:
0: Disabled
1: Hundredths, seconds, minutes, hours, date and month match (once per year)
2: Hundredths, seconds, minutes, hours and date match (once per month)
3: Hundredths, seconds, minutes, hours and weekday match (once per week)
4: Hundredths, seconds, minutes and hours match (once per day)
5: Hundredths, seconds and minutes match (once per hour)
6: Hundredths and seconds match (once per minute)
7: Depends on RV1805_HUNDREDTHS_ALM (0x08) value.
	0x08: 0x00-0x99 Hundredths match (once per second)
	0x08: 0xF0-0xF9 Once per tenth (10 Hz)
	0x08: 0xFF Once per hundredth (100 Hz)
********************************/
void RV1805::setAlarmMode(uint8_t mode)
{
	if (mode > 0b111) mode = 0b111; //0 to 7 is valid
	
	uint8_t value = readRegister(RV1805_CTDWN_TMR_CTRL);
	value &= 0b11100011; //Clear ARPT bits
	value |= (mode << 2);
	writeRegister(RV1805_CTDWN_TMR_CTRL, value);
}


void RV1805::setCountdownTimer(uint8_t duration, uint8_t unit, bool repeat, bool pulse)
{
	// Invalid configurations
	if (duration == 0 || unit > 0b11) {
		return;
	}

	// Set timer value
	writeRegister(RV1805_CTDWN_TMR, (duration - 1));
	writeRegister(RV1805_TMR_INITIAL, (duration - 1));

	// Enable timer
	uint8_t value = readRegister(RV1805_CTDWN_TMR_CTRL);
	value &= 0b00011100; // Clear countdown timer bits while preserving ARPT
	value |= unit; // Set clock frequency
	value |= (!pulse << CTDWN_TMR_TM_OFFSET);
	value |= (repeat << CTDWN_TMR_TRPT_OFFSET);
	value |= (1 << CTDWN_TMR_TE_OFFSET); // Timer enable
	writeRegister(RV1805_CTDWN_TMR_CTRL, value);
}


//Enable the charger and set the diode and inline resistor
//Default is 0.3V for diode and 3k for resistor
void RV1805::enableTrickleCharge(uint8_t diode, uint8_t rOut)
{
	writeRegister(RV1805_CONF_KEY, RV1805_CONF_WRT); //Write the correct value to CONFKEY to unlock this bank of registers
	uint8_t value = 0;
	value |= (TRICKLE_ENABLE << TRICKLE_CHARGER_TCS_OFFSET);
	value |= (diode << TRICKLE_CHARGER_DIODE_OFFSET);
	value |= (rOut << TRICKLE_CHARGER_ROUT_OFFSET);
	writeRegister(RV1805_TRICKLE_CHRG, value);
}

void RV1805::disableTrickleCharge()
{
	writeRegister(RV1805_CONF_KEY, RV1805_CONF_WRT);
	writeRegister(RV1805_TRICKLE_CHRG, (TRICKLE_DISABLE << TRICKLE_CHARGER_TCS_OFFSET));
}

void RV1805::enableLowPower()
{
	//Set various registers to minimize power consumption
	writeRegister(RV1805_CONF_KEY, RV1805_CONF_WRT); //Write the correct value to CONFKEY to unlock this bank of registers
	writeRegister(RV1805_IOBATMODE, 0x00); //Disable I2C when on backup power

	writeRegister(RV1805_CONF_KEY, RV1805_CONF_WRT); //Unlock again
	writeRegister(RV1805_OUT_CTRL, 0x30); //Disable WDI input, Set bit 4, Disable RST in sleep, Disable CLK/INT in sleep

	writeRegister(RV1805_CONF_KEY, RV1805_CONF_OSC); //Unlock again
	writeRegister(RV1805_OSC_CTRL, 0b11111100); //OSEL=1, ACAL=11, BOS=1, FOS=1, IOPW=1, OFIE=0, ACIE=0	
	//Use RC Oscillator all the time (to save moar power)
	//Autocalibrate every 512 seconds to get to 22nA mode
	//Switch to RC Oscillator when powered by VBackup
}

/*******************************************
Enable Battery Interrupt

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
void RV1805::enableBatteryInterrupt(uint8_t voltage, bool edgeTrigger)
{
	setEdgeTrigger(edgeTrigger);
	enableInterrupt(INTERRUPT_BLIE); //Enable Battery Low Interrupt
	setReferenceVoltage(voltage);
}

bool RV1805::checkBattery(uint8_t voltage)
{
	setReferenceVoltage(voltage);
	uint8_t status = readRegister(RV1805_ANLG_STAT);
	if (status >= 0x80)
		return true;

	return false;
}

void RV1805::setReferenceVoltage(uint8_t voltage)
{
	if (voltage > 3) voltage = 3;
	uint8_t value;
	switch (voltage)
	{
	case 0:
		value = TWO_FIVE;
		break;
	case 1:
		value = TWO_ONE;
		break;
	case 2:
		value = ONE_EIGHT;
		break;
	case 3:
		value = ONE_FOUR;
		break;
	}
	writeRegister(RV1805_CONF_KEY, RV1805_CONF_WRT);
	writeRegister(RV1805_BREF_CTRL, value);
}

void RV1805::setEdgeTrigger(bool edgeTrigger)
{
	uint8_t value;
	value = readRegister(RV1805_RAM_EXT);
	value &= ~(1<<6); //Clear BPOL bit
	value |= (edgeTrigger << 6);
	writeRegister(RV1805_RAM_EXT, value);
}

void RV1805::clearInterrupts() //Read the status register to clear the current interrupt flags
{
	status();
}

uint8_t RV1805::BCDtoDEC(uint8_t val)
{
	return ( ( val / 0x10) * 10 ) + ( val % 0x10 );
}

// BCDtoDEC -- convert decimal to binary-coded decimal (BCD)
uint8_t RV1805::DECtoBCD(uint8_t val)
{
	return ( ( val / 10 ) * 0x10 ) + ( val % 10 );
}

uint8_t RV1805::readRegister(uint8_t addr)
{
	_i2cPort->beginTransmission(RV1805_ADDR);
	_i2cPort->write(addr);
	_i2cPort->endTransmission();

	_i2cPort->requestFrom(RV1805_ADDR, (uint8_t)1);
	if (_i2cPort->available()) {
		return (_i2cPort->read());
	}
	else {
		return (0xFF); //Error
	}
}

bool RV1805::writeRegister(uint8_t addr, uint8_t val)
{
	_i2cPort->beginTransmission(RV1805_ADDR);
	_i2cPort->write(addr);
	_i2cPort->write(val);
    if (_i2cPort->endTransmission() != 0)
      return (false); //Error: Sensor did not ack
	return(true);
}

bool RV1805::writeMultipleRegisters(uint8_t addr, uint8_t * values, uint8_t len)
{
	_i2cPort->beginTransmission(RV1805_ADDR);
	_i2cPort->write(addr);
	for (uint8_t i = 0; i < len; i++)
	{
		_i2cPort->write(values[i]);
	}

    if (_i2cPort->endTransmission() != 0)
      return (false); //Error: Sensor did not ack
	return(true);
}

bool RV1805::readMultipleRegisters(uint8_t addr, uint8_t * dest, uint8_t len)
{
	_i2cPort->beginTransmission(RV1805_ADDR);
	_i2cPort->write(addr);
    if (_i2cPort->endTransmission() != 0)
      return (false); //Error: Sensor did not ack

	_i2cPort->requestFrom(RV1805_ADDR, len);
	for (uint8_t i = 0; i < len; i++)
	{
		dest[i] = _i2cPort->read();
	}
	
	return(true);
}
