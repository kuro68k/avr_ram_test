/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief
 *		Example application for the SRAM test.
 *
 *		This example shows how the SRAM test can be embedded in an application. 
 *		The application lights up an LED that signals correct behavior of the 
 *		system. Then it enters the main loop, where the SRAM test is called 
 *		periodically. This test is based on March X and it checks a segment of 
 *		SRAM memory at a time. After the SRAM test, a second LED is toggled in 
 *		order to visualize when the partial tests are finished. If errors were 
 *		found, the global error flag would be set by the test. This would lead 
 *		to the application leaving the main loop and switching off the first LED.
 *		
 *		The configuration for memory size, amount of segments, buffer size, etc. 
 *		can	bet set up in classb_sram.h.  Note that if GCC is used and the size of 
 *		the test buffer changes (as a result of a change in the number of segments 
 *		or the amount of segment overlap), the new buffer size must be configured 
 *		in the linker as well. See \ref classb_sram for more details.
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
#include "classb_sram.h"

#if __AVR_ATxmega256A3BU__ | __ATxmega256A3BU__
#  define LEDPORT PORTR
#  define SWITCHPORT PORTE
#  define SWITCH_INT0_vect PORTE_INT0_vect
#  define XPLAIN_PULLUP 0x00
#elif defined(__AVR_ATxmega128A1__) | defined(__ATxmega128A1__)
#  define LEDPORT PORTE
#  define SWITCHPORT PORTF
#  define SWITCH_INT0_vect PORTF_INT0_vect
#  define XPLAIN_PULLUP PORT_OPC_PULLUP_gc
#endif

void setup_led_switches() {
	
	// Set direction and state of LED pins
	LEDPORT.DIRSET = PIN0_bm | PIN1_bm;
	PORTCFG.MPCMASK = PIN0_bm | PIN1_bm;
	LEDPORT.PIN0CTRL |= PORT_INVEN_bm;
	
	// Set sensitivity, polarity and pull-up for button PIN
	SWITCHPORT.PIN5CTRL = PORT_ISC_FALLING_gc | PORT_INVEN_bm | XPLAIN_PULLUP;
	SWITCHPORT.INT0MASK |= PIN5_bm;
	SWITCHPORT.INTCTRL |= PORT_INT0LVL_LO_gc;

	// Enable LOW level interrupts in the INT controller	
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
	
	// Turn on LED that signals correct operation
	LEDPORT.OUTSET = PIN0_bm;
}


/** \brief Global error variable */
NO_INIT volatile uint8_t classb_error;


/*! \brief 
 *      Main entry point of the application. Sets up board hardware,
 *		and waits until an error has occurred.
 *	\callgraph
 */
int main(void)
{
	
	setup_led_switches();
	// Turn on interrupts globally.
	sei();
    while(!classb_error) {
		cli();
		classb_sram_test();
		sei();
		//Toggle the second LED when test is ready.
		LEDPORT.OUTTGL = PIN1_bm;
	};
	
	// If this is executed there has been an error.
	// Disable interrupts
	cli();
	// Turn off LED to indicate error.
	LEDPORT.OUTCLR = PIN0_bm;
	
}

/*! \brief 
 *      Button press interrupt.
 *	\callgraph
 */
 ISR(SWITCH_INT0_vect) 
{
	
}