/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief
 *		Driver for the RTC. 
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


#include "classb_rtc_common.h"

//! \addtogroup rtc_driver
//@{

//! \internal \brief RTC32 Initialization.
//! 
//! This function initializes the VBAT module and enables the oscillator used by the RTC32.
#if defined(RTC32) || defined(__DOXYGEN__)
void vbat_init(void)
{
	// Reset the battery backup system
	CCP = CCP_IOREG_gc;
	VBAT.CTRL = VBAT_RESET_bm;
	
	// Enable access to VBAT
	VBAT.CTRL |= VBAT_ACCEN_bm;

	// Choose 1024Hz oscillator output and enable crystal oscillator failure monitor
	VBAT.CTRL |= VBAT_XOSCFDEN_bm | VBAT_XOSCSEL_bm;
	/* This delay is needed to give the voltage in the backup system some
	* time to stabilize before we turn on the oscillator. If we do not
	* have this delay we may get a failure detection.
	*/
	delay_us(200);
	
	// Enable the crystal oscillator
	VBAT.CTRL |= VBAT_XOSCEN_bm;
	
	// Wait until the crystal oscillator ready flag is set
	while (!(VBAT.STATUS & VBAT_XOSCRDY_bm));
}
#endif

//! \internal \brief This function waits until RTC synchronizes.
bool rtc_is_busy( void ) {
	#ifdef RTC32
		return RTC32.SYNCCTRL & RTC32_SYNCBUSY_bm;
	#else
		return RTC.STATUS & RTC_SYNCBUSY_bm;
	#endif
}


//! \brief This function sets up the RTC to be used in Class B tests.
//! 
//! The RTC oscillator is enabled, compare interrupts are configured and RTC is enabled.
void classb_rtc_setup() {
	#ifdef RTC32
		// If we use a device with RTC32 module, the VBAT system must be initialized before we use the RTC module
		// This sets a 1024 Hz RTC32 from the 32.768 kHz crystal oscillator 
		vbat_init();
	#else
		// Use the RTC with 1024 Hz from internal 32.768 kHz RC oscillator
		OSC.CTRL |=  OSC_RC32KEN_bm;
		while (!(OSC.STATUS & OSC_RC32KRDY_bm));
        CLK.RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;
	#endif
    
    // Set up the RTC to generate compare interrupt after RTC_TICK_REF [classb_freq.h] ticks and start the RTC.
    RTC_TEST.CTRL &= ~RTC_TEST_START_bm;  // Stop RTC
    while (rtc_is_busy());
    RTC_TEST.PER = 0xffff;  // Maximum 16bit period, for device compatibility
    RTC_TEST.CNT = 0;
    RTC_TEST.COMP = CLASSB_RTC_INT_PERIOD;
    while (rtc_is_busy());
    RTC_TEST.INTCTRL = RTC_TEST_COMPINTLVL_LO_gc;
    RTC_TEST.INTFLAGS = RTC_TEST_COMPIF_bm;
	// Start RTC
	RTC_TEST.CTRL |= RTC_TEST_START_bm;
}

//! \brief This is the RTC compare interrupt.
//!  
//!  Actions required in Class B tests are implemented as callbacks. Note that this 
//!  requires a number of symbols to be defined, either in a header file or at the 
//!  compiler level.
//!  
//!  It is possible to add user-defined code to the RTC interrupt through
//!  \ref CLASSB_ACTIONS_RTC().
ISR(RTC_TEST_COMP_vect) {
		
	// Reset the RTC 
	while (rtc_is_busy());
	RTC_TEST.CNT = 0;
	// Wait until the RTC module is ready, then start the RTC timer.
	while (rtc_is_busy());
	RTC_TEST.CTRL |= RTC_TEST_START_bm;	
	
	#ifdef CLASSB_FREQ_TEST
		classb_freq_callback();
	#endif
	
	#ifdef CLASSB_INT_MON
		classb_intmon_callback(); 
	#endif
	
	
	// User-configurable actions
	CLASSB_ACTIONS_RTC();
}

//@} 