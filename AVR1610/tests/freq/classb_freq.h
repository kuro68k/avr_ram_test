/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief 
 *		Settings and definitions for the CPU frequency test.
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

#ifndef TEST_CPU_VS_RTC_H_
#define TEST_CPU_VS_RTC_H_

#include "avr_compiler.h"
#include "classb_rtc_common.h"
#include "error_handler.h"


//! \defgroup cpu_freq CPU Frequency Test
//! 
//! \brief This is the self-diagnostic test for the CPU frequency. The test 
//!  consists in monitoring the frequency of a Timer/Counter (TC) using the 
//!  Real Time Counter (RTC) module as a time reference. Note that the TC 
//!  has the same clock domain as the CPU and that the RTC module in XMEGA 
//!  can have a clock source independent from the CPU clock domain. Note that 
//!  \ref CLASSB_FREQ_TEST has to be defined. For more details see \ref rtc_driver.
//!  
//!  The test is implemented as follows. Firstly, an RTC compare interrupt 
//!  is scheduled periodically with \ref classb_rtc_setup() and a TC is 
//!  started with \ref classb_freq_setup_timer(). Secondly, the RTC interrupt 
//!  calls \ref classb_freq_callback(), which compares the absolute difference 
//!  between the count in the TC and a predefined reference. If it should be 
//!  greater than expected, the error handler \ref CLASSB_ERROR_HANDLER_FREQ() 
//!  would be called.
//!  
//!  The TC overflow interrupt increases a counter that can be used as 
//!  most significant word (bits 31 to 16) for the TC counter value.  
//!  Further, there is a configurable limit for the number of overflows.
//!  If this was exceeded, the program would assume that the RTC was not 
//!  working correctly and the error handler would be called as well.
//! 
//!  The test is flexible and it is possible to choose some settings for the 
//!  TC and RTC. However, it is important to choose values for \ref CLASSB_RTC_INT_PERIOD, 
//!  \ref CLASSB_RTC_FREQ, \ref CLASSB_TC_PRESCALER and \ref CLASSB_TOLERANCE so that the frequency of 
//!  the CPU \ref F_CPU is estimated reliably. Further, there should not be an 
//!  overflow in \ref CLASSB_TC_COUNT_REF. Suggested values have been included in the 
//!  documentation.
//!  

//! \addtogroup cpu_freq
//@{


//! \defgroup freq_conf Settings
//! \brief This should be configured by the user
//@{

#ifdef __DOXYGEN__ 
 //! \brief The system frequency.
 //! 
 //! This is the expected frequency (Hz) of the CPU. This value has to be defined at 
 //! the compiler level and it is only used to compute the TC count 
 //! reference, i.e. not to set the system clock. Therefore,  
 //! this value should be consistent with the system clock settings in the main 
 //! application. However, this constant could be used to test the self-diagnostic routine, 
 //! i.e. a wrong value could be set to check whether the failure is detected. 
 //! \note The system runs from the internal 2Mhz RC oscillator by default.
 #define F_CPU <value>
#endif

//! \brief TC module selection
//! 
//!	This is the number of the TC module that should be used for the CPU frequency 
//!	test, e.g. 0 -> TCC0.
#define CLASSB_TC_MOD				0

//!\brief TC prescaler
//!
//! The TC runs on the system clock scaled down by this parameter.
//! Possible values are 1, 2, 4, 8, 64, 256 or 1024. 
#define CLASSB_TC_PRESCALER		8

//! \brief Tolerance for the CPU frequency deviation
//! 
//! This value defines the tolerance (an integer, %) for the deviation between the measured
//! and expected CPU, e.g. 25 -> 25%.
#define CLASSB_TOLERANCE			25UL

//@}


//! \internal \defgroup int_conf_freq Internal settings
//! 
//! \brief This constants should not be modified.
//@{

//! \internal \brief  Label for the TC module
#define CLASSB_TEST_TC				LABEL(TCC, CLASSB_TC_MOD,) 

//! \internal \brief  Label for the overflow interrupt vector for the chosen TC
#define CLASSB_TEST_TC_OVF_vect	LABEL(TCC, CLASSB_TC_MOD, _OVF_vect) 

//! \internal \brief Label for the TC prescaler group configuration
#define CLASSB_TC_PRESCALER_gc		LABEL(TC_CLKSEL_DIV, CLASSB_TC_PRESCALER, _gc)

//! \internal \brief TC period
//! 
//! The 16-bit TC will generate an interrupt when the count reaches this value.
//! The code is built counting that this is the largest possible value, i.e. 0xFFFF.
#define CLASSB_TC_PER				0xFFFF

//! \internal \brief TC frequency
//! 
//! The frequency of the TC is F_CPU divided by the prescaling factor.
#define CLASSB_TC_FREQ				(uint32_t) (F_CPU/CLASSB_TC_PRESCALER)

//! \internal \brief TC count reference
//! 
//!	This is the reference value for TC count within one RTC interrupt period. 
//!	Reference = F_tc * Interrupt_Period_rtc / F_rtc 
//!	This can overflow if \ref CLASSB_TC_FREQ and \ref CLASSB_RTC_INT_PERIOD are too large.
//!	Note that multiplying by 1e0L forces the preprocessor to do operations with 
//!	higher precision.
#define CLASSB_TC_COUNT_REF		(uint32_t) ( ( (1e0L * CLASSB_TC_FREQ) * CLASSB_RTC_INT_PERIOD) / CLASSB_RTC_FREQ )
  
//! \internal \brief Maximum absolute difference between the reference and estimated frequencies 
//! 
//! Note that multiplying by 1e0L forces the preprocessor to do operations with 
//!	higher precision.<
#define CLASSB_MAX_DIF				(uint32_t) ( (1e0L * CLASSB_TC_COUNT_REF * CLASSB_TOLERANCE) / 100UL )

//! \internal \brief TC maximum number of overflows
//! 
//! This is a limit for the number of TC overflows. If this should be exceeded the RTC 
//! would be assumed to be faulty and the error handler would be called. 
#define CLASSB_COUNT_OVF_MAX		(uint16_t)  ( ( CLASSB_TC_COUNT_REF + CLASSB_MAX_DIF )>> 16)

//@}


//! \defgroup func_freq Functions
//! 
//! \brief Functions related to the CPU frequency self-test.
//@{
void classb_freq_setup_timer();
void classb_freq_callback();
//@}


//@}

#endif /* TEST_CPU_VS_RTC_H_ */