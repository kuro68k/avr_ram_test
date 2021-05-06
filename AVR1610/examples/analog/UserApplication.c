/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief
 *		This in an demo application for the ADC and DAC tests. 
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
#include "classb_analog.h"

//! \brief Global error flag
NO_INIT volatile uint8_t classb_error;

//! \name Board configuration 
//@{
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
//@}

//! \brief Setup LEDs and button.
void setup_led_switches_pmic() {
	
	// Set direction and state of LEDs 
	LEDPORT.DIRSET = PIN0_bm|PIN1_bm;
	PORTCFG.MPCMASK = PIN0_bm|PIN1_bm;
	LEDPORT.PIN0CTRL |= PORT_INVEN_bm;
	
	// Set sensitivity, polarity and pullup for button SW0
	SWITCHPORT0.PIN5CTRL |= PORT_ISC_FALLING_gc | PORT_INVEN_bm;	
	SWITCHPORT0.INT0MASK |= PIN5_bm;
	SWITCHPORT0.INTCTRL |= PORT_INT0LVL_LO_gc;
		
	// Enable LOW level interrupts in the INT controller
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
	
	// Turn on LED that signals correct operation.
	LEDPORT.OUTSET = PIN0_bm;
}


int main(void)
{
	// Initial setup
	setup_led_switches_pmic();
	
	// Interrupts enabled in the system
	sei();	
	
    while(!classb_error) 
	{ 
		// Do nothing 
	};	
	
	// If this is executed there has been an error. 
	// Turn off LED
	LEDPORT.OUTCLR = PIN0_bm;
}


/*! \brief Interrupt for SW0 press
 *		
 */
ISR(SWITCH0_INT0_vect) 
{
	 cli();
	 // Test DAC and first ADC module
	 classb_analog_io_test(&DACB, &ADCA);
	 // Test DAC and second ADC module
	 classb_analog_io_test(&DACB, &ADCB);	 
	 sei();
}


