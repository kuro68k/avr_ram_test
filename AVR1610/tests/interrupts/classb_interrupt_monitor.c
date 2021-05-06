/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief
 *		This file contains functions for monitoring the frequency of execution of 
 *		registered interrupts. 
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


#include "classb_interrupt_monitor.h"

//! \brief Array of data structures for the interrupts that should be monitored
struct intmon_interrupt monitored_interrupts[N_INTERRUPTS];


/** \brief Registers an interrupt. 
 * 
 * This function registers the information that the monitor needs in order to 
 * check an interrupt. It should be called from the main application. Interrupts 
 * must be declared in \ref classb_int_identifiers in the configuration file. 
 * Note that by default the interrupt is not active for monitoring, i.e. the 
 * state is \c OFF.
 *
 *  \param identifier	Interrupt identifier. Use symbol declared in \ref classb_int_identifiers
 *  \param reference	Number of expected executions for the interrupt within an RTC period: 
 *						<tt>F_INT (Hz) * CLASSB_RTC_INT_PERIOD / CLASSB_RTC_FREQ (Hz) </tt>
 *  \param tolerance	The allowed deviation (%) with respect to interrupt_counter for the interrupt counter.
 *	\callergraph
 */
void classb_intmon_reg_int(enum classb_int_identifiers identifier, uint16_t reference, uint8_t tolerance) 
{
	
	monitored_interrupts[identifier].n = reference;
	monitored_interrupts[identifier].c = 0;
	monitored_interrupts[identifier].l = (reference * tolerance) / 100;
	monitored_interrupts[identifier].s = OFF;	
	
}

/*! \brief Increases the interrupt counter of the specified interrupt.
 *
 *  This is called from the interrupt routine and it will increases the counter 
 *  if the interrupt is \c ON.
 *
 *  \param identifier	Interrupt identifier. Use symbol declared in \ref classb_int_identifiers 
 *
 *	\callergraph
 */
void classb_intmon_increase( enum classb_int_identifiers identifier ) 
{	

	if (monitored_interrupts[identifier].s == ON)
		monitored_interrupts[identifier].c++;

}

/*! \brief Set a state for the specified interrupt.
 * 
 *	This function should be called from the main application to enable or disable monitoring this interrupt. 
 *	The application can only set the state to ENABLE or DISABLE, otherwise the error handler will be called. 
 *	i.e. only the monitor can set \c ON and \c OFF states. Further, if \ref CLASSB_STRICT is defined, it is not allowed 
 *	to enable an interrupt in the \c ON state, or disable an interrupt in the \c OFF state.
 *	
 *  \param identifier	Interrupt identifier. Use symbol declared in \ref classb_int_identifiers.
 *  \param state		Interrupt state. Use symbol declared in \ref classb_int_states.
 *
 *	\callergraph
 */
void classb_intmon_set_state( enum classb_int_identifiers identifier, enum classb_int_states state ) 
{
	
	switch (state)
	{
		case ENABLE:
#ifdef CLASSB_STRICT
			if (monitored_interrupts[identifier].s != OFF) 
			{
				CLASSB_ERROR_HANDLER_INTERRUPT();
				break;
			}			
#endif				
			break;

		case DISABLE:
#ifdef CLASSB_STRICT
			if (monitored_interrupts[identifier].s != ON) 
			{
				CLASSB_ERROR_HANDLER_INTERRUPT();
				break;
			}
#endif						
			break;		
		default:
			CLASSB_ERROR_HANDLER_INTERRUPT();
	}
	
	// Set the new state only if CLASSB_CONDITION1_INTERRUPT is true.
	if(CLASSB_CONDITION1_INTERRUPT)	
		monitored_interrupts[identifier].s = state;	
			
}



//! \internal\brief Returns the absolute value of the difference between two numbers.
static inline uint16_t abs_diff(uint16_t a, uint16_t b) 
{
	return (a > b)?(a - b):(b - a);
}

/*! \brief The interrupt monitor.
 * 
 * For each registered interrupt, this function compares the counter with 
 * the expected value. There is an error if the difference is greater 
 * than the limit or if the interrupt is disabled and the counter is different 
 * than zero. If \ref CLASSB_CONDITION2_INTERRUPT is true, the monitor will stop 
 * immediately, i.e. the remaining interrupts are not checked. 
 * 
 * \note This should be called back from the RTC interrupt. See \ref rtc_driver.
 *  
 * \callergraph
 */
void classb_intmon_callback() 
{
	
	for (uint8_t i = 0; i < N_INTERRUPTS; i++ ) 
	{
		
		switch (monitored_interrupts[i].s)
		{
			case ON:
				// Check whether the counter is within the allowed range
				if ( (uint16_t) abs_diff(monitored_interrupts[i].c, monitored_interrupts[i].n) > monitored_interrupts[i].l) {
					CLASSB_ERROR_HANDLER_INTERRUPT();
					break;
				}
				// Reset counter			
				monitored_interrupts[i].c = 0;
				break;
			case OFF:
				// The counter is only increased when the state is ON.
				if (monitored_interrupts[i].c)
					CLASSB_ERROR_HANDLER_INTERRUPT();
				break;			
			case ENABLE:											
				// Change state
				monitored_interrupts[i].s = ON;					
				break;
			case DISABLE:				
				// Change state and reset the counter
				monitored_interrupts[i].s = OFF;
				monitored_interrupts[i].c = 0;
				break;
			default:
				CLASSB_ERROR_HANDLER_INTERRUPT(); 
		}
		
		// If CLASSB_CONDITION2_INTERRUPT is true, there is no need to check the other interrupts.
		if (CLASSB_CONDITION2_INTERRUPT)
			break;								
	}
}
