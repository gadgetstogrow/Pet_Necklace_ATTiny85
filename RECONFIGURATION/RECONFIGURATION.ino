
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
// *** (Note: don't change the order of includes)
// ***
#include "Photocell.h"
#include "AppManager.h"


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
 
 //Run the scheduler - doesn't return
 scheduler.runTasks();
  
}
