/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief
 *      This is the header file for the ADC and DAC tests. 
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

#ifndef CLASSB_ADC_DAC_H_
#define CLASSB_ADC_DAC_H_

#include "avr_compiler.h"
#include "error_handler.h"

//! \defgroup adc_dac  Analog I/O Test
//! 
//! \brief This self-diagnostic test for the ADC, DAC and analog multiplexer. 
//! 
//! A plausibility check is done to make sure that ADC and DAC
//! work correctly. The test is implemented in \ref classb_analog_io_test() and consists on 
//! the following: the DAC generates 5 values (0%, 25%, 50%, 75% and 100% of the scale) and
//! the ADC will read them. If the read values should deviate from the expected values more 
//! than a configurable threshold, the test would call the error handler 
//! \ref CLASSB_ERROR_HANDLER_ANALOG().
//! 
//! The test checks one ADC module and one DAC module at a time, i.e. it has to be 
//! repeated until all modules are tested. Further, the ADC module to test should 
//! be able to read from the DAC module it is tested together with. 
//! 
//! \note 
//!  - Interrupts should be disabled during this test.
//@{

//! \internal \name Internal settings
//@{
//! \internal \brief Maximum allowed absolute deviation for the test measurements. 
//! 
//! Error offset for the ADC is +-2mV, which corresponds to +-0x40 when the 
//! reference is 1V and  \c TOP is \c 2047.
#define CLASSB_ADC_DEV      0x40
//@}


//! \name Class B Test
//@{			
void classb_analog_io_test(DAC_t *dacptr, ADC_t *adcptr);
//@}


//@}


#endif /* CLASSB_ADC_DAC_H_ */