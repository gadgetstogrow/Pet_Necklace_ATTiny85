/*************************************************************************************************
 * Config.h
 * 
 * Purpose: Configuration Settings for the Pet_Necklace_ATTiny85 application
 * 
 * 
 * Copyright � 2019 Kevin Gagnon. All Rights Reserved.
 *
 * This file is part of a Pet_Necklace_ATTiny85 Series:
 * https://www.hackster.io/GadgetsToGrow/
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Task Scheduler example code. If not,
 * see http://www.gnu.org/licenses/.
 ************************************************************************************************/ 

#ifndef CONFIG_H_
#define CONFIG_H_

//-------------------------------------------------------------------
// ATTiny85 fuse settings are set for 1MHz clock speed for power conservation.
// Note: the code is configured to run with both a 1MHz and 8MHz clock, and 
// will adjust according to the below (uncommented) definition. If you use the USBTiny to 
// program the ATTiny85, make sure you "Burn Bootloader" to set the appropriate
// fuses.
//-------------------------------------------------------------------
//#define F_CPU 1000000UL
//#define F_CPU 8000000UL

//-------------------------------------------------------------------
//
// Pin Definitions
//								Software		Device	Purpose
//								Pin				Pin			
//-------------------------------------------------------------------
#define TILT_SENSOR_PIN			PB3			//	2		Tilt Switch Interrupt Pin
#define PHOTOCELL_PIN			PB2			//	7		Light Level via Photocell
#define NECKLACE_LED_0			PB0			//  5		!OC1A - Timer1 (inverted)
#define NECKLACE_LED_1			PB1			//	6		OC1A - Timer1
#define PENDANT_LED				PB4			//  3		Blinks at AppManager Read Rate

//------------------------------------------------------------------
// Only increment ledCycleCount on a LOW (0) from tilt sensor
//------------------------------------------------------------------
#define MOVEMENT !(PINB & (1 << TILT_SENSOR_PIN))

//-------------------------------------------------------------------
// Volatile variable for Pin Change Interrupt PCINT2_vect ISR on PB3
// When the tilt sensor detects movement, this interrupt is fired and the 
// ledCycleCount variable is incremented.  The AppManager Task decrements
// this variable at a predefined rate.
//-------------------------------------------------------------------
volatile uint8_t ledCycleCount = 0;

// -----------------------------------------------------------------
//
// TimedTask Default Cycles 
//
// Note:	The rates below will vary based on the F_CPU setting above,
//			and provide similar operational results independent of system
//			clock speed (1MHz or 8MHz).
//
//									Rate  Purpose
//-------------------------------------------------------------------
#define APP_MANAGER_RATE_MS		100   // ~100 milliseconds readRate 
#define PHOTOCELL_READ_RATE_MS  1000  // ~1 second

//-------------------------------------------------------------------
// LED Cycle Count/Thresholds (ledCycleRate)
//-------------------------------------------------------------------
#define CYCLE_RATE_THRESHOLD_FAST	60	//ledCycleCount 100-60
#define CYCLE_RATE_THRESHOLD_MEDIUM	20	//ledCycleCount 59-20
#define CYCLE_RATE_THRESHOLD_SLOW	0	//ledCycleCount 19-0

//-------------------------------------------------------------------
// Cycle Count Definitions (ledCycleCount)
//-------------------------------------------------------------------
#define CYCLE_INCREMENT_BY_X		1	//Increment the ledCycleCount by...
#define CYCLE_COUNT_MAX				100	//Max ledCycleCount (no debounce)

//-------------------------------------------------------------------
// ADC variables (light level - 8 bit resolution)
//
// Power Management: Experiment with the LIGHT_THRESHOLD value to meet 
// your power management needs. The lower this number is, the lower the
// light level has to be, to turn on the necklace LEDs. The Pendant LED
// will remain unaffected until the ledCycleCount reaches zero.
//-------------------------------------------------------------------
enum lightThresholds {DARK = 10, SUNDOWN = 40, TWILIGHT = 70};

#define LIGHT_THRESHOLD	 SUNDOWN

#endif /* CONFIG_H_ */