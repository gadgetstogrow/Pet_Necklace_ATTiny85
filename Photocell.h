/*************************************************************************************************
 * Photocell.h
 * 
 * Purpose: Read 8-bit analog Light Level via photocell 
 * 
 * Copyright © 2019 Kevin Gagnon. All Rights Reserved.
 *
 * This file is part of a Pet Necklace Series:
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
 * along with the Pet Necklace code. If not, see http://www.gnu.org/licenses
 *
 **************************************************************************/ 

#ifndef PHOTOCELL_H_
#define PHOTOCELL_H_

/***************************************************************************************
*	Class:		Photocell
*	Task Type:	TimedTask
*	Purpose:	Reads the photocell attached to (PHOTOCELL_PIN: PB2) at a specific rate
*				(PHOTOCELL_READ_RATE_MS)
*
*
****************************************************************************************/

class Photocell : public TimedTask
{
	public:
	
		Photocell(uint8_t _inputPin, uint32_t _cycleRate);
		virtual void run(uint32_t now);
		void initADC();
		uint8_t getLightLevel();
	
	private:
	
		uint8_t inputPin;
		uint32_t cycleRate;
		//uint8_t lowADCByte;
		uint16_t lightLevel;
};

// ***
// *** Photocell Constructor
// ***
Photocell::Photocell(uint8_t _inputPin, uint32_t _cycleRate)
	: TimedTask(millis()),
	inputPin(_inputPin),
	cycleRate(_cycleRate),
	//lowADCByte(0),
	lightLevel(0)

{
	
	// ***
	// *** Initialize the Analog to Digital Converter to read 8-bit
	// *** value from channel ADC1 on the inputPin. This only needs
	// *** to run one time, hence it's located in the constructor
	// *** of the Photocell.
	// ***
	initADC();
	
	// ***
	// *** Set the Photocell read pin to input
	// ***
	DDRB &= ~(1 << inputPin);

}

// --------------------------------------------------------------------------
// Initialize the Analog to Digital Converter (ADC) connected to PB2
// --------------------------------------------------------------------------
void Photocell::initADC()
{
	//-----------------------------------------------------------------------
	// ADC Setup - Re: PHOTOCELL_PIN - PB2  - Port: ADC1
	//-----------------------------------------------------------------------
	
	/************************************************************************
	 * ADMUX -	ADC Multiplexer Channel Select Bits 0:3/MUX3:0 
	 *			(DataSheet Table: 17-4, Pg: 135)
	 *
	 *			Selects ADC1 on PB2 as input
	 * Note:	ADLAR is set here, indicating an 8-bit resolution. I didn't
	 *			see a need to provide 10-bit resolution considering the 
	 *			overhead of the addition clock cycle and 2-byte value.
	 *
	 * |REFS1|REFS0|ADLAR|REFS2|MUX3 |MUX2 |MUX1 |MUX0 |
	 * |  0  |  0  |  1  |  0  |  0  |  0  |  0  |  1  |
	*************************************************************************/
	ADMUX =
			(1 << ADLAR)	|   // left shift result (8-bit resolution)
			(0 << REFS2)	|   // Sets ref. voltage to Vcc, bit 2
			(0 << REFS1)	|   // Sets ref. voltage to Vcc, bit 1   
			(0 << REFS0)	|   // Sets ref. voltage to Vcc, bit 0
			(0 << MUX3)		|   // use ADC1 for input (PB2), MUX bit 3
			(0 << MUX2)		|   // use ADC1 for input (PB2), MUX bit 2
			(0 << MUX1)		|   // use ADC1 for input (PB2), MUX bit 1
			(1 << MUX0);		// use ADC1 for input (PB2), MUX bit 0
	
	/************************************************************************
	* ADCSRA -	ADC Control and Status Register
	*			(DataSheet Figure 17.13.2, Pg: 136)
	* 
	* Set Bit 7/ADEN - Enable ADC
	* Set prescaler to 50-200KHz for maximum resolution (Datasheet Pg: 125)
	* Set to 125KHz: 1MHz/64 = 125KHz (Datasheet Table: 17.5, Pg: 136)
	* The code below automatically compiles to the correct prescaler setting
	* depending on the F_CPU setting in the Config.h file.
	*	
	*							 | System Clock |
	* ADC Prescaler Selections:	 |8MHz   | 1MHz |
	*
	* |Div. |ADPS2|ADPS1|ADPS0|	 |FREQ   | FREQ |
	*----------------------------------------
	* |   2 |  0  |  0  |  0  |<- 4MHz   | 500MHz
	* |   2 |  0  |  0  |  1  |<- 4MHz   | 500MHz
	* |   4 |  0  |  1  |  0  |<- 2MHz   | 250KHz
	* |   8 |  0  |  1  |  1  |<- 1MHz   | 125KHz      <-Current Setting (1MHz)
	* |  16 |  1  |  0  |  0  |<- 500KHz | 62.5KHz
	* |  32 |  1  |  0  |  1  |<- 250KHz | 31.250KHz
	* |  64 |  1  |  1  |  0  |<- 125KHz | 15.625KHz   <--if running ATTiny at 8MHz	
	* | 128 |  1  |  1  |  1  |<- 62.5KHz| 7.812KHz		
	*
	*
	* |ADEN |ADSC |ADATE|ADIF |ADIE |ADPS2|ADPS1|ADPS0|
	* |  1  |  0  |  0  |  0  |  0  |  1  |  1  |  0  |
	****************************************************************************************/
	ADCSRA = 
			(1 << ADEN)		|			// Enable ADC 
			#if F_CPU == 1000000UL		//<--1MHz --------<
			(0 << ADPS2)	|			// clear bit 2 
			(1 << ADPS1)	|			// set bit 1 
			(1 << ADPS0);				// set bit 0 
			#elif F_CPU == 8000000UL	//<--8Mhz --------<
			(1 << ADPS2)	|			// set bit 2 
			(1 << ADPS1)	|			// set bit 1 
			(0 << ADPS0);				// clear bit 0
			#endif 			
	
}

// ***
// *** Photocell::run()
// ***
void Photocell::run(uint32_t now)
{
	//-------------------------------------------------------------------
	// Measure the current Light Level
	//-------------------------------------------------------------------
	/******************************************************************************
	* ADCSRA - ADC Control and Status Register (DataSheet Figure 17.13.2, Pg: 136)
	* 
	* Set Bit 6/ADSC - ADC Start Conversion - Arduino: "analogRead(PB3)"
	*
	* Note: Other bits have been set during initialization
	*
	* |ADEN |ADSC |ADATE|ADIF |ADIE |ADPS2|ADPS1|ADPS0|
	* |  1  |  1  |  0  |  0  |  0  |  1  |  1  |  0  |
	*******************************************************************************/
	ADCSRA |= (1 << ADSC);	//Start Conversion
	
	//-----------------------------------------------------------------------------
	// The ADSC bit will be cleared upon completion of conversion. Wait here until
	// it's finished.
	//-----------------------------------------------------------------------------
	while(ADCSRA & (1 << ADSC));

	/******************************************************************************
	* ADCL - ADC Low Byte (DataSheet Figure 17.13.3.1, Pg: 137)
	* Note: The ADLAR bit in the ADMUX is Cleared, hence 10-bit resolution
	* The result will be right adjusted in ADCH/ADCL. If ADLAR was set to 1
	* you could just read ADCH for 8 bit resolution.
	*
	* |ADC7 |ADC6 |ADC5 |ADC4 |ADC3 |ADC2 |ADC1 |ADC0 |
	* |  0  |  0  |  0  |  0  |  0  |  0  |  0  |  0  |
	*
	* ADCH - ADC High Byte 
	*
	* |  -  |  -  |  -  |  -  |  -  |  -  |ADC8 |ADC8 |
	* | N/A | N/A | N/A | N/A | N/A | N/A |  0  |  0  |
	*******************************************************************************/
	lightLevel = ADCH; //<< 8 | lowADCByte; // <--- Bitwise to 10-bit value

	// Run again in the specified number of milliseconds
	incRunTime(cycleRate);
	
}

//-------------------------------------------------------------------
// Accessor for the lightLevel variable
//-------------------------------------------------------------------
uint8_t Photocell::getLightLevel()
{
	
	return lightLevel;
	
}


#endif /* PHOTOCELL_H_ */