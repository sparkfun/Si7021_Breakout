/*
 SparkFun Si7021 Temperature and HUmidity Breakout 
 By: Joel Bartlett
 SparkFun Electronics
 Date: December 10, 2015
 
 This is an Arduino library for the Si7021 Temperature and Humidity Sensor Breakout
 
 This library is based on the following libraries:

 HTU21D Temperature / Humidity Sensor Library
 By: Nathan Seidle
 https://github.com/sparkfun/HTU21D_Breakout/tree/master/Libraries

 Arduino Si7010 relative humidity + temperature sensor
 By: Jakub Kaminski, 2014
 https://github.com/teoqba/ADDRESS

 This Library is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This Library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 For a copy of the GNU General Public License, see
 <http://www.gnu.org/licenses/>.
 */

#if defined(ARDUINO)
#include "Wire.h"
#include "SparkFun_Si7021_Breakout_Library.h"
#elif defined(SPARK)
#include "SparkFun_Si7021_Breakout_Library/SparkFun_Si7021_Breakout_Library.h"
#endif


 //Initialize
 Weather::Weather(){}

 bool Weather::begin(void)
{
  Wire.begin();

  uint8_t ID_Temp_Hum = checkID();

  int x = 0;

  if(ID_Temp_Hum == 0x15)//Ping CheckID register
    x = 1;
  else if(ID_Temp_Hum == 0x32)
    x = 2;
  else
    x = 0;

  if(x == 1)
  {
    Serial.println("Si7021 Found");
    //Serial.println(ID_Temp_Hum, HEX);
  }
  else if(x == 2)
  {
    Serial.println("HTU21D Found");
    //Serial.println(ID_Temp_Hum, HEX);
  }
  else
  	Serial.println("No Devices Detected");
  	//Serial.println(ID_Temp_Hum, HEX);
}

/****************Si7021 & HTU21D Functions**************************************/


float Weather::getRH()
{
	// Measure the relative humidity
	uint16_t RH_Code = makeMeasurment(HUMD_MEASURE_NOHOLD);
	float result = (125.0*RH_Code/65536)-6;
	return result;
}

float Weather::readTemp()
{
	// Read temperature from previous RH measurement.
	uint16_t temp_Code = makeMeasurment(TEMP_PREV);
	float result = (175.72*temp_Code/65536)-46.85;
	return result;
}

float Weather::getTemp()
{
	// Measure temperature
	uint16_t temp_Code = makeMeasurment(TEMP_MEASURE_NOHOLD);
	float result = (175.72*temp_Code/65536)-46.85;
	return result;
}
//Give me temperature in fahrenheit!
float Weather::readTempF()
{
  return((readTemp() * 1.8) + 32.0); // Convert celsius to fahrenheit
}

float Weather::getTempF()
{
  return((getTemp() * 1.8) + 32.0); // Convert celsius to fahrenheit
}


void Weather::heaterOn()
{
	// Turns on the ADDRESS heater
	uint8_t regVal = readReg();
	regVal |= _BV(HTRE);
	//turn on the heater
	writeReg(regVal);
}

void Weather::heaterOff()
{
	// Turns off the ADDRESS heater
	uint8_t regVal = readReg();
	regVal &= ~_BV(HTRE);
	writeReg(regVal);
}

void Weather::changeResolution(uint8_t i)
{
	// Changes to resolution of ADDRESS measurements.
	// Set i to:
	//      RH         Temp
	// 0: 12 bit       14 bit (default)
	// 1:  8 bit       12 bit
	// 2: 10 bit       13 bit
	// 3: 11 bit       11 bit

	uint8_t regVal = readReg();
	// zero resolution bits
	regVal &= 0b011111110;
	switch (i) {
	  case 1:
	    regVal |= 0b00000001;
	    break;
	  case 2:
	    regVal |= 0b10000000;
	    break;
	  case 3:
	    regVal |= 0b10000001;
	  default:
	    regVal |= 0b00000000;
	    break;
	}
	// write new resolution settings to the register
	writeReg(regVal);
}

void Weather::reset()
{
	//Reset user resister
	writeReg(SOFT_RESET);
}

uint8_t Weather::checkID()
{
	uint8_t ID_1;

 	// Check device ID
	Wire.beginTransmission(ADDRESS);
	Wire.write(0xFC);
	Wire.write(0xC9);
	Wire.endTransmission();

    Wire.requestFrom(ADDRESS,1);

    ID_1 = Wire.read();

    return(ID_1);
}

uint16_t Weather::makeMeasurment(uint8_t command)
{
	// Take one ADDRESS measurement given by command.
	// It can be either temperature or relative humidity
	// TODO: implement checksum checking

	uint16_t nBytes = 3;
	// if we are only reading old temperature, read olny msb and lsb
	if (command == TEMP_PREV) nBytes = 2;

	Wire.beginTransmission(ADDRESS);
	Wire.write(command);
	Wire.endTransmission();
	// When not using clock stretching (*_NOHOLD commands) delay here
	// is needed to wait for the measurement.
	// According to datasheet the max. conversion time is ~22ms
	 delay(23);
	
	Wire.requestFrom(ADDRESS,nBytes);
	if(Wire.available() != nBytes)
  	return 100;

	unsigned int msb = Wire.read();
	unsigned int lsb = Wire.read();
	// Clear the last to bits of LSB to 00.
	// According to datasheet LSB of RH is always xxxxxx10
	lsb &= 0xFC;
	unsigned int mesurment = msb << 8 | lsb;

	return mesurment;
}

void Weather::writeReg(uint8_t value)
{
	// Write to user register on ADDRESS
	Wire.beginTransmission(ADDRESS);
	Wire.write(WRITE_USER_REG);
	Wire.write(value);
	Wire.endTransmission();
}

uint8_t Weather::readReg()
{
	// Read from user register on ADDRESS
	Wire.beginTransmission(ADDRESS);
	Wire.write(READ_USER_REG);
	Wire.endTransmission();
	Wire.requestFrom(ADDRESS,1);
	uint8_t regVal = Wire.read();
	return regVal;
}
