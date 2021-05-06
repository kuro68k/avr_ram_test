/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief This is the demo application for the CPU frequency test.
 *
 *      The test basically checks periodically that the relationship 
 *      between the RTC and CPU frequencies (measured through a 
 *      Timer/Counter, TC) is within a configurable range.
 *      
 *		The main program consists of a loop where the device does not 
 *		do anything as long as there is no error in the CPU frequency. 
 *		An LED is on while the system is operating correctly. However, 		
 *		if the error flag should be set, the main program would exit the 
 *		loop and switch off the LED.
 *		
 *		In order to test that the self-diagnostic routine is correct, 
 *		the CPU frequency can be modified on-the-fly by pressing the 
 *		button SW0. By default the system runs from the internal 2 Mhz
 *		oscillator. The interrupt for button press changes the system clock
 *		to the internal 32 Mhz oscillator. The CPU frequency test will 
 *		then fail because the frequency of the system does not match the 
 *		configuration.
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
#include "classb_freq.h"

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


//! \brief This is the error flag.
NO_INIT volatile uint8_t classb_error;


//! \brief This function sets up the LED and switch.
//! The LED is switched on to show that the system is working correctly. 
//! The switch is configured to generate an interrupt after being pressed.
void setup_led_switches() 
{		
	// Set direction and state of LED pins
	LEDPORT.DIRSET = PIN0_bm;
	PORTCFG.MPCMASK = PIN0_bm;
	LEDPORT.PIN0CTRL |= PORT_INVEN_bm;
	
	// Set sensitivity, polarity and pull-up for button PIN
	SWITCHPORT.PIN5CTRL = PORT_ISC_FALLING_gc | PORT_INVEN_bm | XPLAIN_PULLUP;
	SWITCHPORT.INT0MASK |= PIN5_bm;
	SWITCHPORT.INTCTRL |= PORT_INT0LVL_LO_gc;
	
	// Enable LOW level interrupts in the INT controller
	PMIC.CTRL |= PMIC_LOLVLEN_bm;

	// Turn on LED that signals correct operation.	
	LEDPORT.OUTSET = PIN0_bm;	
}


int main(void)
{
	// Set up led and switch
	setup_led_switches();
    // Setup and start RTC and TC
	classb_rtc_setup();	
    classb_freq_setup_timer();
	
	// Enable interrupts globally
    sei();
    	
    while(!classb_error) { 
        /* Do nothing */
    };
	
	// If we reach this point, an error has been detected. 
	// Disable interrupts and turn off LED.
	cli();
    LEDPORT.OUTCLR = PIN0_bm;
}

//! \brief Interrupt service routine for button press.
//!  This will change the clock frequency to 32MHz, which should be detected by the frequency test. 
ISR(PORTE_INT0_vect) 
{	
	// Enable 32 Mhz internal oscillator and wait until the oscillator is stable.
    OSC.CTRL |=  OSC_RC32MEN_bm;
    while (!(OSC.STATUS & OSC_RC32MRDY_bm));
	
	// Select the oscillator as source for the system clock. 
	// This requires writing first to the configuration change protection register.
    CCP = CCP_IOREG_gc;
    CLK.CTRL = CLK_SCLKSEL_RC32M_gc;    
}



