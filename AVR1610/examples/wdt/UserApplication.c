/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief
 *		This is the demo application for the watchdog timer (WDT) test. 
 *
 *		The self-diagnostic routine for the WDT is executed before 
 *		the main function is called. If the WDT is not working properly,
 *		the device will hang.
 *		
 *		This demo application sets up an interrupt for SW0, switches 
 *		an LED on and then stays in a loop as long as the system is 
 *		on a working state. In this loop the WDT is reset according to 
 *		its open and closed period settings. If the error flag should be  
 *		set, the main program would exit the loop and switch off the LED.
 *
 *		The following procedure can be followed in order to test that 
 *		the correct behavior of the WDT. When button SW0 is pressed, the 
 *		WDT is reset too early and it will issue a system reset. When 
 *		button SW1 is pressed, the WDT is reset too late and a system reset 
 *		will be issued as well. This "unexpected" resets should be caught 
 *		by the self-diagnostic routine, which in this demo is configured 
 *		to set the error flag, setup the WDT and continue to the main application. 		
 *		Therefore, after pressing any of the buttons the system is set on
 *		a safe state.
 *
 *		
 * \par Application note:
 *      AVR1610: Guide to IEC60730 Class B compliance with XMEGA
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler 
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 * 
 * Copyright (C) 2012 Atmel Corporation. All rights reserved.
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 * Atmel AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */
 
#include "avr_compiler.h"
#include "util/delay.h"

NO_INIT volatile uint8_t classb_error;

#if __AVR_ATxmega256A3BU__ | __ATxmega256A3BU__ | __DOXYGEN__
 #  define LEDPORT PORTR
 #  define SWITCHPORT0 PORTE
 #  define SWITCHPORT1 PORTF
 #  define SWITCH0_INT0_vect PORTE_INT0_vect
 #  define SWITCH1_INT0_vect PORTF_INT0_vect
 #  define XPLAIN_PULLUP 0x00
#elif defined(__AVR_ATxmega128A1__) | defined(__ATxmega128A1__)
 #  define LEDPORT PORTE
 #  define SWITCHPORT0 PORTF
 #  define SWITCHPORT0 PORTF
 #  define SWITCH_INT0_vect PORTF_INT0_vect
 #  define XPLAIN_PULLUP PORT_OPC_PULLUP_gc
#endif

/*! \brief Hardware setup function for button- and LED functionality.
 */
void setup_led_switches() {
	// Set direction and state of LED pins
	LEDPORT.DIRSET = PIN0_bm | PIN1_bm;
	PORTCFG.MPCMASK = PIN0_bm | PIN1_bm;
	LEDPORT.PIN0CTRL |= PORT_INVEN_bm;
	
	// Set sensitivity, polarity and pullup for button SW0
	SWITCHPORT0.PIN5CTRL |= PORT_ISC_FALLING_gc | PORT_INVEN_bm;	
	SWITCHPORT0.INT0MASK |= PIN5_bm;
	SWITCHPORT0.INTCTRL |= PORT_INT0LVL_LO_gc;
	
	// Set sensitivity, polarity and pullup for button SW1
	SWITCHPORT1.PIN1CTRL |= PORT_ISC_FALLING_gc | PORT_INVEN_bm;	
	SWITCHPORT1.INT0MASK |= PIN1_bm;
	SWITCHPORT1.INTCTRL |= PORT_INT0LVL_LO_gc;
	
	// Enable LOW level interrupts in the INT controller
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
	
	// Turn on LED that signals correct operation.
	LEDPORT.OUTSET = PIN0_bm;
}

//! This delay is longer than the closed window setting, but shorter than the total period.
void correct_timing() { _delay_ms(600); }
//! This delay is shorter than the closed window setting.
void fast_timing() { _delay_ms(100); }
//! This delay is longer than the total period. 
void slow_timing() { _delay_ms(1000); }

//! This is a function pointer, initialized to the correct delay.
void (*our_delay)(void) = &correct_timing;

int main(void)
{

	// Initial setup
	setup_led_switches();

	// Enable interrupts
	sei();
	
    while(classb_error!=1) { 				
		//This calls one of the three delay functions
		our_delay();		
		LEDPORT.OUTTGL = PIN1_bm;
		watchdog_reset();		
	};
	
	// If we reach here, an error has been detected. Turn off LED.
	LEDPORT.OUTCLR = PIN0_bm;
	
}


/*! \brief 
 *		Button SW0 interrupt function. The WDT will be updated too early. 
 */
ISR(SWITCH0_INT0_vect) {
	our_delay = &fast_timing;	
}

/*! \brief 
 *		Button SW1 interrupt function. The WDT will be updated too late. 
 */
ISR(SWITCH1_INT0_vect) {
	our_delay = &slow_timing;
}