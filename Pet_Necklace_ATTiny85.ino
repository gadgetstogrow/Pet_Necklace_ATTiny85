
/****************************************************************************
 *  Project:	Pet_Necklace_ATTiny85.ino
 *	Version:	1.4.0
 *  Contact:	Kevin Gagnon
 *	Twitter:	@GadgetsToGrow
 *
 *  YouTube:	https://www.youtube.com/channel/UCseG_fDVFWgDqoBSpghMIPg
 *	  
 *  Date:		4-15-2019
 *  
 *  Purpose:	A *glamorous* blinking pet necklace using the ATTiny85. 
 *				Blinking occurs when the tilt/inclination sensor detects
 *				movement and only turns on the main LEDs (6) in the collar
 *				portion of the necklace when the light level is at a specified
 *				threshold. There is a pendant LED that will flash if there is
 *				movement, independent of the light level. When no movement is
 *				detected, the ATTiny85 is put into power down sleep mode and
 *				awaits an interrupt from the tilt sensor.
 *			
 *  Tutorial:	
 *
 *  Source:		
 * 
 *  ATTiny85 PINOUT *********************************************************
 *
 *	SOFTWARE	PHYSICAL
 *	PIN			PIN			PURPOSE											
 *	-------------------------------------------------------------------------
 *	PB0 - PB1:	5-6			Blink the 6 Necklace LEDs
 *  PB2:		7			PhotoCell (Power Management) Is it dark enough?	
 *	PB3:		2			Pin Change Interrupt (PCINT0_vect/PCINT3)
 *	PB4:		3			Pendant LED							
 *
 ****************************************************************************/

#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// ***
// *** Task Scheduler library
// ***
#include "Task.h"
#include "TaskScheduler.h"

// ***
// *** Application Configuration File (e.g. Pin Numbers, CPU Clock Frequency, etc.)
// ***
// ***
#include "Config.h"

// ***
// *** Application specific tasks
// ***
// Note: Necklace LEDs will only be turned on at certain light level threshold.
#include "Photocell.h"

// --------------------------------------------------------------------------------------------
// Define the AppManager Class as type: TimedTask
//
// Reads the ledCycleCount variable (incremented by ISR(PCINT0_vect) every 100ms
// via the tilt sensor attached to Pin PB3. If the value is greater than zero
// (activity sensed) the AppManager Task (depending on light level) executes LED routines.
//
// If ledCycleCount is zero (no activity/lazy pet), it puts the ATTiny85 to sleep and waits to be interrupted again. 
//
// --------------------------------------------------------------------------------------------
class AppManager : public TimedTask
{
	public:
		AppManager( uint16_t _readRate,	Photocell *_ptrPhotocell);
		//Task Scheduler-------------------------------------------------------------------------------			
		virtual void run(uint32_t now);
	
	private:
		uint8_t readRate; 						// Rate to read the Tilt Sensor
		
		//Tilt Sensor Related--------------------------------------------------------------------------
		void initPinChangeInterrupt();			// Enables the Pin Change Interrupt

		//ADC Related----------------------------------------------------------------------------------
		void enableADC(bool _enabled);			// Enable/Disable the ADC
		bool isLightLevelOk();					// Compare Light Level reading to LIGHT_THRESHOLD
		
		//Pendant LED----------------------------------------------------------------------------------
		void togglePendantLED();				//Toggle the Pendant LED on each read of Tilt Sensor
												//independent of the current Light Level.
		
		//Necklace LEDs--------------------------------------------------------------------------------
		void clearTimer1Config();
		void updateTimer1Config(uint8_t _ledCycleCount);	// Timer1: Change/Update Timer1 Mode/Rate
		void initAlternatingBlink_PWM();

		//Global Control-------------------------------------------------------------------------------
		void enableGlobalInterrupts(bool _enabled);		// Enable/Disable Global Interrupts sei/cli
		void enableNecklaceLEDOutput(bool _enabled);	// Enable/Disable Output for LEDs in Necklace
		void enablePendantLEDOutput(bool _enabled);		// Enable/Disable Output for the Pendant LED
		void goToSleep();								// Turn off peripherals and go to sleep...ZZzz
		
		//App Task Control Pointers-------------------------------------------------------------
		Photocell *ptrPhotocell;

};

//-------------------------------------------------------------------------------------------
// AppManager Constructor - Initializes the TimedTask to control the current application
//-------------------------------------------------------------------------------------------
AppManager::AppManager( uint16_t _readRate, Photocell *_ptrPhotocell)
	
	: TimedTask(millis()),
	readRate(_readRate),
	ptrPhotocell(_ptrPhotocell)

{
	
	// ***
	// *** Enable Tilt Sensor Pin Change Interrupt on PB3
	// ***
	initPinChangeInterrupt();

	// ***
	// *** Enable Global Interrupts - sei();
	// ***
	enableGlobalInterrupts(true);
	
	
	// ***
	// *** Enable Necklace LED Output (PB0/PB1)
	// ***
	enableNecklaceLEDOutput(true);
	
	// ***
	// *** Enable Pendant LED Output (PB4)
	// ***
	enablePendantLEDOutput(true);
		
	// ***
	// *** Clear Timer1 Configuration Settings
	// ***
	clearTimer1Config();
	
	// ***
	// *** Initialize Timer1 to handle LED control
	// ***
	initAlternatingBlink_PWM();

}

//-------------------------------------------------------------------
// Main AppManager Routine
//-------------------------------------------------------------------
void AppManager::run(uint32_t now)
{
	if(ledCycleCount > 1) //Movement/Awake - performing application functions
	{
		//Decrement  the ledCycleCount, when it finally reaches zero, go to sleep
		ledCycleCount--;

			// Toggle the LED in the Pendant. This is good for debugging and battery
			// monitoring even if the light level is too high for the Necklace LEDs.
			togglePendantLED();

			// Check the Light Level against LIGHT_THRESHOLD
			if(isLightLevelOk())
			{
				
				// Enable the Necklace LEDs on PB0 and PB1.
				enableNecklaceLEDOutput(true);
				
				// Update the Output Compare Registers (OCR1A and OCR1C) depending
				// on the current value of ledCycleCount to change the frequency and
				// period of outputs PB1/OC1A and PB0/!OC1A
				updateTimer1Config(ledCycleCount);
					
			} else {
					
				// Disable the Necklace LEDs on PB0 and PB1 to conserve power and leaves
				// the Pendant LED active and independent of the current light level
				// threshold. The Pendant will stop flashing when ledCycleCount reaches zero.
				enableNecklaceLEDOutput(false);
					
			} //End Light Level Check

	}
	else //ledCycleCount is Zero, go to Sleep
	{
		
		goToSleep();

	} //End ledCycleCount > 1 Check
	
	//Set the next time for the AppManger to execute its run() method
	incRunTime(readRate);
}


//-------------------------------------------------------------------
// LED Output Enable/Disable for the 6 Necklace LEDs
//-------------------------------------------------------------------
void AppManager::enableNecklaceLEDOutput(bool _enabled)
{
	
	//DATASHEET REFERENCE:
	/*************************************************************************
	* Port B Data Direction Register (DDRB) (Datasheet 10.2.1, Pg: 54)
	*
	* |  -  |  -  |DDB5 |DDB4 |DDB3 |DDB2 |DDB1 |DDB0 |
	* | N/A | N/A |  0  |  0  |  0  |  0  |  0  |  0  |
	**************************************************************************
	* Port B Data Register (PORTB) (Datasheet 10.2.1, Pg: 54)
	*
	* |  -  |  -  | PB5 | PB4 | PB3 | PB2 | PB1 | PB0 |
	* | N/A | N/A |  0  |  0  |  0  |  0  |  0  |  0  |
	**************************************************************************/

	if(_enabled)
	{
		//Set pins to output
		DDRB |= (1 << NECKLACE_LED_0);		//PB1
		DDRB |= (1 << NECKLACE_LED_1);		//PB0
		
	} else {
		
		//Make sure LEDs are off
		PORTB &= ~(1 << NECKLACE_LED_0);	//write LOW/0 to PB0
		PORTB &= ~(1 << NECKLACE_LED_1);	//write LOW/0 to PB1
		
		//Set pins to input
		DDRB &= ~(1 << NECKLACE_LED_0);		//PB0
		DDRB &= ~(1 << NECKLACE_LED_1);		//PB1

	}	
}

//-------------------------------------------------------------------
// LED Output Enable/Disable for the 6 Necklace LEDs
//-------------------------------------------------------------------
void AppManager::enablePendantLEDOutput(bool _enabled)
{
	
	//DATASHEET REFERENCE:
	/*************************************************************************
	* Port B Data Direction Register (DDRB) (Datasheet 10.2.1, Pg: 54)
	*
	* |  -  |  -  |DDB5 |DDB4 |DDB3 |DDB2 |DDB1 |DDB0 |
	* | N/A | N/A |  0  |  0  |  0  |  0  |  0  |  0  |
	**************************************************************************
	* Port B Data Register (PORTB) (Datasheet 10.2.1, Pg: 54)
	*
	* |  -  |  -  | PB5 | PB4 | PB3 | PB2 | PB1 | PB0 |
	* | N/A | N/A |  0  |  0  |  0  |  0  |  0  |  0  |
	**************************************************************************/

	if(_enabled)
	{
		//Set pin to output
		DDRB |= (1 << PENDANT_LED);			//PB4
		
	} else {
		
		//Make sure LEDs are off
		PORTB &= ~(1 << PENDANT_LED);		//write LOW/0 to PB4
		
		//Set pins to input
		DDRB &= ~(1 << PENDANT_LED);		//PB0

	}	
}


//-------------------------------------------------------------------
// Toggle the Pendant LED (unaffected by light level)
// Blinks at a steady 100ms when ledCycleCount > 0
//
// DATASHEET REFERENCE:
// Using the PINB register to Toggle a Pin: Paragraph 10.2.2, Pg: 54
//-------------------------------------------------------------------
void AppManager::togglePendantLED()
{
	// *** 
	// *** Quick and easy method to change the state of an LED. If it
	// *** is on, it will turn it off and vice-versa.
	PINB |= (1 << PB4);
}

//---------------------------------------------------------------------------
// Adjust Frequency/Period and Duty Cycle of Timer1 - proportional to the
// ledCycleCount: more activity = faster rate
//---------------------------------------------------------------------------
void AppManager::updateTimer1Config(uint8_t _ledCycleCount)
{
	
	if (_ledCycleCount > CYCLE_RATE_THRESHOLD_FAST)
	{			
			//***********************************************************
			//***********************************************************
			// Prescaler Clock is set to:
			// Frequency 488.28Hz
			//
			// Equation: (Refer to initAlternatingBlink_PWM() method)
			//
			// 8000000/16824 = 488.28Hz <--- 8MHz System Clock
			//		or
			// 1000000/2048	= 488.28Hz	<--- 1MHz System Clock
			// 
			// (One Clock Tick = 2.048ms)
			//
			// 
			//***********************************************************
			//***********************************************************	
			//***********************************************************
			//	CYCLE_RATE_THRESHOLD_FAST:
			//
			//	Frequency:	~10Hz
			//
			//	Equation:
			//	488.28 / OCR1C=[49 + 1] = 9.77Hz
			//-----------------------------------------------------------
			//  Period:		~100ms
			//
			//  Equation: 
			//	1 Clock Tick=[2.048ms] x OCR1C=[49 + 1] = 102.4ms
			//***********************************************************
			OCR1C = 49;

			//***********************************************************				
			//	Duty Cycle:	~50%  <-- ON for 1/2 of period
			//	
			//	Equation:
			//	OCR1A=[24 + 1] / OCR1C=[49 + 1] = .50
			//***********************************************************
			OCR1A = 24;	
				
	} else if ((_ledCycleCount <= CYCLE_RATE_THRESHOLD_FAST) 
				&& (_ledCycleCount > CYCLE_RATE_THRESHOLD_MEDIUM))
	
		{
			//***********************************************************
			//	CYCLE_RATE_THRESHOLD_MEDIUM:
			//
			//	Frequency:	~5Hz
			//
			//	Equation:
			//	488.28Hz / OCR1C=[99 + 1] = 4.88Hz
			//-----------------------------------------------------------
			//  Period:		~200ms
			//
			//  Equation: 
			//	1 Clock Tick=[2.048ms] x OCR1C=[99 + 1] = 204.8ms
			//***********************************************************
			OCR1C = 99;

			//***********************************************************				
			//	Duty Cycle:	~50%  <-- ON for 1/2 of period
			//	
			//	Equation:
			//	OCR1A=[49 + 1] / OCR1C=[99 + 1] = .50
			//***********************************************************	
			OCR1A = 49;
				
	} else {	
				
			//***********************************************************
			//	CYCLE_RATE_THRESHOLD_SLOW:
			//
			//	Frequency:	~2.5Hz
			//
			//	Equation:
			//	488.28Hz / OCR1C=[199 + 1] = 2.44Hz
			//-----------------------------------------------------------
			//  Period:		~400ms
			//
			//  Equation: 
			//	1 Clock Tick=[2.048ms] x OCR1C=[199 + 1] = 409.6ms
			//***********************************************************	
			OCR1C = 199;		
				
			//***********************************************************				
			//	Duty Cycle:	~50%  <-- ON for 1/2 of period 
			//
			//	Equation:
			//  OCR1A=[99 + 1] / OCRIC=[199 + 1] = .50
			//***********************************************************	
			OCR1A = 99;
	}

}

void AppManager::clearTimer1Config()
{
	TCCR1 = 0;
	GTCCR = 0;
	OCR1A = 0;
	OCR1B = 0;
	OCR1C = 0;
	TCNT1 = 0;
	
}

void AppManager::initAlternatingBlink_PWM()
{
	//-------------------------------------------------------------------------------------------
	//Alternating PWM pulses via Timer/Counter1
	//-------------------------------------------------------------------------------------------
		/****************************************************************************************
		* TCCR1 – Timer/Counter1 Control Register Figure: 12.3.1, Pg: 89)
		*		CTC1 - Clear Timer/Counter1 on Compare Match (CTC Mode)
		*		PWM1A - Pulse Width Modulator A Enable (PMW)
		*		COM1A[1:0] - Controls output on OC1A/PB1
		*		CS[13:10] - Clock Select bits (refer to Table: 12-5, Pg: 89)
		*
		* Compare Output Mode, Fast PWM (Datasheet Table: 11-3, Pg: 78)
		*
		* |COM1A1 |COM1A0 |
		* |	  0   |   0   | <-- OC1A/PB1 Disconnected
		* |   0   |   1   | <-- Toggle OC1A/PB1
		* |   1   |   0   | <-- Clear OC1A/PB1 output line
		* |   1   |   1   | <-- Set OC1A/PB1 output line
		*
		* Note: Clock Select CS[12-10] is global to Timer/Counter1 and affects both OC1A/PB1 
		* and OC1B/PB4. We set the Prescale clock to 488.28ms
		* 
		*
		* |CTC1 |PWM1A|COM1A1 |COM1A0 |CS13 |CS12 |CS11 |CS10|
		* |  1  |  0  |   0   |   0   |  0  |  1  |  0  |  0 |  < - 
 		*****************************************************************************************/
		TCCR1 = 
				(0 << CTC1)		|	//PWM 
				(1 << PWM1A)	|	//PWM 
				(0 << COM1A1)	|	//OC1A/PB1 
				(1 << COM1A0)	|	//OC1A/PB1 
		#if F_CPU == 1000000UL		// <-----------Divide System Clock by 2048 
				(1 << CS13)		|	//CS bit 3
				(1 << CS12)		|	//CS bit 2
				(0 << CS11)		|	//CS bit 1
				(0 << CS10);		//CS bit 0
		#elif F_CPU == 8000000UL	// <-----------Divide System Clock by 16384
				(1 << CS13)		|	//CS bit 3
				(1 << CS12)		|	//CS bit 2
				(1 << CS11)		|	//CS bit 1
				(1 << CS10);		//CS bit 0
		#endif


}

// ---------------------------------------------------------------------
// Enable PCINT3 on PB3 for ATTiny85
//		-Pin Change (Only LOW increments ledCycleCount via the MOVEMENT macro in Config.h
//		-Fires ISR(PCINT0_vect) on ATTiny85
// ---------------------------------------------------------------------
void AppManager::initPinChangeInterrupt()
{
	/****************************************************************************************
	* Set up Tilt Sensor to Input - Replaces: "pinMode(PB3, INPUT)" 
	*****************************************************************************************
	* Port B Data Direction Register (DDRB) (Datasheet 10.4, Pg: 64)
	* Set DDB3 to zero.
	*
	* |  -  |  -  |DDB5 |DDB4 |DDB3 |DDB2 |DDB1 |DDB0 |
	* | N/A | N/A |  0  |  0  |  0  |  0  |  0  |  0  |
	****************************************************************************************/
	DDRB  &= ~(0 << TILT_SENSOR_PIN);	//Set TILT_SENSOR_PIN(PB3) to Input -DDB3/PB3 = 0
	
	/****************************************************************************************
	* Set up Tilt Sensor Input as a Pullup Pin - Replaces: "digitalWrite(PB3, HIGH)"
	*****************************************************************************************
	* Port B Data Register (PORTB) (Datasheet 10.4, Pg: 64)
	* Set PORTB3 to one (1)
	*
	* |  -  |  -  |PORTB5 |PORTB4 |PORTB3 |PORTB2 |PORTB1 |PORTB0 |
	* | N/A | N/A |   0   |   0   |   1   |   0   |   0   |   0   |
	****************************************************************************************/	
	PORTB |= (1 << TILT_SENSOR_PIN);		//Enable the pull-up resistor on PORTB3/PB3 = 1


	// -----------------------------------------------------------------
	// Enable Pin Change Interrupt on TILT_SENSOR_PIN/PB3/PCINT3
	// -----------------------------------------------------------------
	
	/****************************************************************************************
	* GIMSK - General Interrupt Mask Register (DataSheet Figure: 9.3.2, Pg: 51)
	*
	*
	* |  -  |INT0 |PCIE |  -  |  -  |  -  |  -  |  -  |
	* | N/A |  0  |  1  | N/A | N/A | N/A | N/A | N/A |
	*****************************************************************************************/
	GIMSK = 
			(0 << INT0) |	//Disable External Interrupts
			(1 << PCIE);	//Enable Pin Change Interrupts

	/****************************************************************************************
	* PCMSK - Pin Change Mask Register (DataSheet Figure: 9.3.4, Pg: 52)
	*
	*
	* |  -  |  -   |PCINT5|PCINT4|PCINT3|PCINT2|PCINT1|PCINT0|
	* | N/A | N/A  |  0   |  0   |  1   |  0   |  0   |  0   |
	*****************************************************************************************/

	PCMSK = 
			(0 << PCINT5)			|
			(0 << PCINT4)			|
			(1 << TILT_SENSOR_PIN)	|  //Use PB3 as interrupt pin PCINT3
			(0 << PCINT2)			|
			(0 << PCINT1)			|
			(0 << PCINT0);
}		

// ---------------------------------------------------------------------
// Enable Global Interrupts in SREG
// ---------------------------------------------------------------------
void AppManager::enableGlobalInterrupts(bool _enabled)
{
	if(_enabled)
	{
		
		sei();
		
	} else {
		
		cli();
	}
	
}



// ---------------------------------------------------------------------
// Enable/Disable the ADC (disabled before sleep, then re-enabled on wakeup
// for power management considerations.
// ---------------------------------------------------------------------
void AppManager::enableADC(bool _enabled)
{
	if(_enabled)
	{

		ADCSRA = (1 << ADEN);

		} else {
		
		ADCSRA = (0 << ADEN);
		
	}
}


// ---------------------------------------------------------------------
// Put the ATTiny into deep sleep when no movement is detected
// ---------------------------------------------------------------------
void AppManager::goToSleep()
{
		// ***
		// *** Disable Global Interrupts First
		// ***
		enableGlobalInterrupts(false);
		
		// ***
		// *** Clear the ledCycleCount variable
		// ***
		ledCycleCount = 0;
		
		// ***
		// *** Disable LED Outputs (Power Management)
		// ***
		enablePendantLEDOutput(false);
		enableNecklaceLEDOutput(false);
		
		// ***
		// *** Turn off the Analog To Digital Converter (ADC) 
		// ***
		enableADC(false);

		// ***
		// *** Sleep Mode: Deep Sleep
		// ***
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_enable();
		
		// ***
		// *** Enable Global Interrupts -to wake up from sleep PCINT0 on TILT_SENSOR_PIN/PB3/PCINT3
		// ***
		enableGlobalInterrupts(true);
		
		// ***
		// *** Go to sleep
		// ***
		//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
		//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
		sleep_mode();
		//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
		//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
		// ***
		// *** Wake Up
		// ***
		sleep_disable();
		
		// ***
		// *** Turn On the Analog To Digital Converter (ADC) for Photocell readings
		// *** 
		enableADC(true);

		//Re-enable Output LEDs
		enablePendantLEDOutput(true);
		enableNecklaceLEDOutput(true);
		
}

// ---------------------------------------------------------------------
// Check the latest Light Level via ADC/Photocell and compare to our
// predetermined threshold.
// ---------------------------------------------------------------------
bool AppManager::isLightLevelOk()
{
	
	//Use the pointer to the Photocell object to retrieve the latest reading
	//and compare it to the threshold set in Config.h
	return (ptrPhotocell->getLightLevel() < LIGHT_THRESHOLD);

}

// -----------------------------------------------------------------------
// Interrupt Service Routine [PCINT3] for tilt sensor connected to Pin 3.
// Increments the ledCycleCount to keep the led(s) enabled. When the 
// ledCycleCount reaches zero (0), the AppManager will put the 
// MCU into sleep mode to conserve power. This same interrupt will wake up
// the MCU.
//
// Note: There is no debounce logic for the application, but we restrain the
// ledCycleCount from exceeding the uint8_t range by applying a check against
// MAX_LED_CYCLE_COUNT. The AppManager will decrement this number
// on each of its Timed Tasks determined by APP_MANAGER_RATE_MS.
// 
// -----------------------------------------------------------------------
ISR(PCINT0_vect)
{
	if(MOVEMENT && (ledCycleCount < (CYCLE_COUNT_MAX - CYCLE_INCREMENT_BY_X)))
	{
		ledCycleCount += CYCLE_INCREMENT_BY_X;
	}
}


//SETUP
void setup() {}

//LOOP
void loop() 
{

/***************************************************************************************
* Description:  Instantiate the tasks and schedule task priority.
* Purpose:		This is the heart of the program.  All of the needed code has been 
*				encapsulated in the above Class definitions. The code below will
*				create the task objects based on these classes and fill a task
*				array for the TaskScheduler to manage.
*
*
* Note:     Although this is located in the loop() routine, this will only
*			run once. TaskScheduler::run() never returns control to loop().
*
***************************************************************************************/

// ***
// *** Instantiate the task objects for use by the TaskScheduler
// ***

	Photocell photocell(PHOTOCELL_PIN, PHOTOCELL_READ_RATE_MS);
	
	AppManager appManager(APP_MANAGER_RATE_MS, &photocell);

 // ***
 // *** Create an array of reference addresses to the task objects we just instantiated.
 // ***
 // *** The order matters here.  When the TaskScheduler is running it finds the first task it can
 // *** run--canRun(), runs the task--run(), then returns to the top of the list and starts the
 // *** process again.
 // ***

Task *tasks[] = {
	&appManager,
	&photocell

  
};

 //***
 //*** Instantiate the TaskScheduler and fill it with task references.
 //***
 TaskScheduler scheduler(tasks, NUM_TASKS(tasks));
 
 //GO! Run the scheduler - it never returns.
 scheduler.runTasks();
  
}
