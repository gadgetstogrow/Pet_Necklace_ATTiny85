# LED Pet Necklace with the ATTiny85

This project is useful for those looking to journey outside the cozy confines of the Arduino Uno and on to other AVRs. The ATTiny85 is a great place to start. The example code was first developed using Atmel Studio 7 with the help of an AVR Dragon for debugging. Don't worry, It will compile and upload using the Arduino IDE. A complete and detailed tutorial can be found on my project page on [Hackster.io] (https://www.hackster.io/GadgetsToGrow/)

## 3D Printed Parts

The 3D parts that make up the enclosure of this project were designed using FreeCAD. [FreeCAD](https://www.freecadweb.org/) all other materials needed are available online or at your local hardware store.


## Concepts discussed on Hackster.io 

-Direct Register Manipulation - discover and experiment with the DDRB, PINB, PORTB and other registers of the ATTiny85 to control input and output instead of using the familiar pinMode() and digitalWrite() methods. We'll also be using register manipulation for Timer/Counter1 and the Analog to Digital Converter (ADC).

-Power Management - we'll use a coin cell battery for this project, so reducing unnecessary power consumption is paramount. (e.g. sleep mode and turning off peripherals we're not using).

-Pin Change Interrupts - the Tilt Sensor is used to wake up the ATTiny85 from sleep mode via a pin change interrupt using the PCINT0 vector. Why use power for the MCU and LEDs when your pet is sleeping or still?

-ADC Setup and Measurement - we'll leave analogRead() behind, and control the ADC to measure ambient light level through direct register control. You'll see how to handle a 10-bit value, and how to enable and disable the ADC for power management.
Timer/Counter1 - instead of tying up processor time with the mundane task of blinking LEDs at a specific interval, we'll use TImer/Counter1 to handle this task and explore the benefits of this "set it and forget it" methodology.

### Prerequisites

You'll need to use a USBTiny programmer, the ArduinoISP example code or similar method for programming an ATTiny85. These methods are beyond the scope of this project, but a vast amount of instruction is available online.
With that said, if you're planning on working with AVRs other than the Arduino (especially ones without a UART (like the ATTiny85) I'd highly recommend taking a look at Atmel Studio and a compatible programmer/debugger. Atmel Studio is free, and you can pick up an AVR Dragon on Ebay for < $50.

## License

This project is licensed under the GNU License - see the [LICENSE.md](LICENSE.md) file for details


