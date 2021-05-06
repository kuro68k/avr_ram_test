/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief
 *		This in an demo application for the interrupt monitor. 
 *
 *		This application sets up a periodic interrupt: a timer/counter (TC) 
 *		overflow interrupt. Further, the application configures the 
 *		interrupt monitor in order to check that the frequency of the 
 *		interrupt is correct. Two switch interrupts are configured. The 
 *		first one changes the frequency of the TC interrupt. The second
 *		one deactivates the interrupt in the monitor.
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
#include "classb_interrupt_monitor.h"

//! \name Configuration parameters
//@{

//!\brief Frequency in Hz of the T/C interrupt.
//!
//! This is used to compute the value written to the PER register.
#define F_TC_INT 30

//!\brief Tolerance for number of T/C interrupts (%).
//! 
#define TC_INT_TOL 15		

//!\brief TC prescaler
//!
//! The TC runs on the system clock scaled down by this parameter.
//! Possible values are 1, 2, 4, 8, 64, 256 or 1024. 
#define TC_PRESCALER		8

//! \brief This multiplies the TC period in SW0 interrupt.
#define PER_CHANGE 4

//@}


//! \name Internal parameters
//@{

//! \brief Reference for number of TC interrupts 
//! 
#define TC_INT_COUNT_REF		(uint32_t) ( ( (1e0L * F_TC_INT) * CLASSB_RTC_INT_PERIOD) / CLASSB_RTC_FREQ )				

//! \brief Label for the TC prescaler group configuration
#define TC_PRESCALER_gc		LABEL(TC_CLKSEL_DIV, TC_PRESCALER, _gc)

//! \brief TC frequency
//! 
//! The frequency of the TC is F_CPU divided by the prescaling factor.
#define TC_FREQ				(uint32_t) (F_CPU/TC_PRESCALER)

//! \brief TC period
//! 
//! The 16-bit TC will generate an interrupt when the count reaches this value.
#define TC_PER				(uint32_t) (TC_FREQ / F_TC_INT)

//@} 

//! \name Global variables
//@{

//! \brief Global error flag
NO_INIT volatile uint8_t classb_error;

//@}


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

//! \brief Setup LEDs and buttons.
void setup_led_switches_pmic() {
	
	// Set direction and state of LEDs 
	LEDPORT.DIRSET = PIN0_bm|PIN1_bm;
	PORTCFG.MPCMASK = PIN0_bm|PIN1_bm;
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

/*! \brief Setup TC interrupt
 *! 
 *! This sets up the TC so that it generates an overflow interrupt periodically. 
 *! This interrupt will be checked with the interrupt monitor. 
 */
void setup_example_tc_interrupt() {	
	// Set the T/C period
	TCD0.PER = TC_PER;		
	// Enable LOW level overflow interrupt.
	TCD0.INTCTRLA = TC_OVFINTLVL_LO_gc; 
	// Set the prescaler (and start TC) 
	TCD0.CTRLA = TC_PRESCALER_gc;  
}

int main(void)
{
	// Initial setup
	setup_led_switches_pmic();
	setup_example_tc_interrupt();
	
	// Register the interrupt in the monitor
	classb_intmon_reg_int(MY_INTERRUPT, TC_INT_COUNT_REF, TC_INT_TOL); 	
	
	// Setup the RTC. The monitor is called back from the RTC interrupt.
    classb_rtc_setup();	
		
	// Interrupts enabled in the system
	sei();
	
	// Enable the monitoring of the TC interrupt
	classb_intmon_set_state(MY_INTERRUPT, ENABLE);
	
    while(!classb_error) { 
		// Do nothing 
	};	
	
	// If this is executed there has been an error. Turn off LEDs
	LEDPORT.OUTCLR = PIN0_bm|PIN1_bm;
}


/*! \brief Interrupt for SW0 press: change TC period.
 *		
 *	This interrupt can be used to test the interrupt monitor. After pressing SW0 the
 *	period of the TC is changed, which means that the frequency of the TC interrupt is
 *	modified. This should be detected by the interrupt monitor, which should then set 
 *	the global error flag \ref classb_error. 
 */
ISR(SWITCH0_INT0_vect) {
	TCD0.PER = PER_CHANGE * TC_PER; 
}

/*! \brief Interrupt for SW1 press: disable TC interrupt monitoring.
 * 
 *	If the TC interrupt does not need to be monitored anymore the state of the interrupt 
 *	can be changed to DISABLE.
 *	\note This interrupt could be executed more than once because of button bouncing. 
 *	If \ref CLASSB_STRICT is defined the monitor would then set \ref classb_error.
 */
ISR(SWITCH1_INT0_vect) {
	classb_intmon_set_state(MY_INTERRUPT, DISABLE);
}


/*! \brief TC overflow interrupt
 *		
 *	The interrupt counter is incremented by calling \ref classb_intmon_increase(). 
 *	After that an LED is toggled.
 */
ISR(TCD0_OVF_vect) {

	classb_intmon_increase(MY_INTERRUPT);
	
	// Toggle LED.
	LEDPORT.OUTTGL = PIN1_bm;
}