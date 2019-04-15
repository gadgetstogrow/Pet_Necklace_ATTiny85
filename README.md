# Pet_Necklace_ATTiny85 - Part 1
Code for Hackster.io Project - April 15, 2019

##Introduction
As described in the title, this project is Part 1 of the Pet Necklace.  This project, will focus primarily on the theory of operation and the code behind it.  The project and code can be operated from breadboard mounted components. The assembly of the 3D-printed parts and incorporation of the components (without the use of a PCB!) will be covered in Part 2.  I'm still fiddling with the final design, but the code is ready and has been tested on my prototype. My dogs have worn their necklaces 24/7 and the battery has lasted in the range of 10-14 days. Granted, they may be the laziest dogs on earth, but that's a long time when you consider it was initially intended to only be used for walks.

When I make a new gadget, of course I want to show the whole world, but more importantly I want to remember any new techniques I've learned along the way. In this project we will not only make a cool pet necklace for your dog, cat... or lizard, but hopefully we'll learn some new things as we go.

The following project is useful for those looking to journey outside the cozy confines of the Arduino Uno and the familiar Arduino IDE techniques. The example code was first developed using Atmel Studio 7 with the help of an AVR Dragon for debugging. Don't worry, It will compile and upload using the Arduino IDE. 

Note: You'll need to use a USBTiny programmer (listed in the Things section) or use the ArduinoISP example code or similar method for programming an ATTiny85. These methods are beyond the scope of this project, but a vast amount of instruction is available online.
With that said, if you're planning on working with AVRs other than the Arduino (especially ones without a UART (like the ATTiny85) I'd highly recommend taking a look at Atmel Studio and a compatible programmer/debugger. Atmel Studio is free, and you can pick up an AVR Dragon on Ebay for < $50. Both are beyond the intended scope of this project, but I will touch upon some of the benefits as the project progresses.

##Learning Objectives
This project and included example code examines the following concepts:

###Direct Register Manipulation - we'll discover and experiment with the DDRB, PINB, PORTB and other registers of the ATTiny85 to control input and output instead of using the familiar pinMode() and digitalWrite() methods. We'll also be using register manipulation for Timer/Counter1 and the Analog to Digital Converter (ADC).

###Power Management - we'll use a coin cell battery for this project, so reducing unnecessary power consumption is paramount. (e.g. sleep mode and turning off peripherals we're not using).

###Pin Change Interrupts - the Tilt Sensor is used to wake up the ATTiny85 from sleep mode via a pin change interrupt using the PCINT0 vector. Why use power for the MCU and LEDs when your pet is sleeping or still?

###ADC Setup and Measurement - we'll leave analogRead() behind, and control the ADC to measure ambient light level through direct register control. You'll see how to handle a 10-bit value, and how to enable and disable the ADC for power management.
Timer/Counter1 - instead of tying up processor time with the mundane task of blinking LEDs at a specific interval, we'll use TImer/Counter1 to handle this task and explore the benefits of this "set it and forget it" methodology.
