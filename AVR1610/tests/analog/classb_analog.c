/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief This is the code for the ADC and DAC tests.
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

#include "classb_analog.h"

// Global error variable.
extern volatile uint8_t classb_error;


/*! 
 * \internal
 * \brief This test writes a value to the DAC, and checks that the ADC readouts are
 *        within the expected range.
 */
static void classb_dac_adc_test(DAC_t *dacptr, ADC_t *adcptr, uint16_t dac_out, uint16_t adc_assert) {
	
	// Deviation in measurements.
    int16_t adcDev;

	// Write to DAC and wait until it's stable
	dacptr->CH0DATA = dac_out;
    while( !(dacptr->STATUS & DAC_CH0DRE_bm) );
	
	// Clear interrupt flag channel 3
	adcptr->CH3.INTFLAGS = ADC_CH_CHIF_bm;
    // Start AD-conversions in channels 0 to 3. 
	adcptr->CTRLA |= (ADC_CH0START_bm | ADC_CH1START_bm | ADC_CH2START_bm | ADC_CH3START_bm);
	// Use channel 3 interrupt flag to detect end of conversion 
	// (channel 3 has the lowest priority). 
    while( !(adcptr->CH3.INTFLAGS & ADC_CH_CHIF_bm) );

    // Do a range check on the conversion results.
    adcDev = adcptr->CH0RES - adc_assert;
    if (abs(adcDev) > CLASSB_ADC_DEV)
        CLASSB_ERROR_HANDLER_ANALOG();

    adcDev = adcptr->CH1RES - adc_assert;
    if (abs(adcDev) > CLASSB_ADC_DEV)
        CLASSB_ERROR_HANDLER_ANALOG();

    adcDev = adcptr->CH2RES - adc_assert;
    if (abs(adcDev) > CLASSB_ADC_DEV)
        CLASSB_ERROR_HANDLER_ANALOG();

    adcDev = adcptr->CH3RES - adc_assert;
    if (abs(adcDev) > CLASSB_ADC_DEV)
       CLASSB_ERROR_HANDLER_ANALOG();	

	// Clear interrupt flags.
    adcptr->CH0.INTFLAGS = ADC_CH_CHIF_bm;
    adcptr->CH1.INTFLAGS = ADC_CH_CHIF_bm;
    adcptr->CH2.INTFLAGS = ADC_CH_CHIF_bm;
    adcptr->CH3.INTFLAGS = ADC_CH_CHIF_bm;
}

/*! \brief Functional test for the ADC, DAC and analog MUX.
 *
 * This function configures the DAC to output a constant voltage, after which
 * the ADC is set to do 12 bit, signed conversions of the DAC.
 * Range checking of the conversion results is then done to verify that ADC and DAC
 * are working correctly. 
 *
 * \param dacptr Base address for registers of DAC module to test.
 * \param adcptr Base address for registers of ADC module to test.
 *
 */
void classb_analog_io_test(DAC_t *dacptr, ADC_t *adcptr)
{

    // Set up the DAC
    // Single channel, 1V reference, internal output, 
    dacptr->CTRLA = DAC_IDOEN_bm | DAC_ENABLE_bm;
    dacptr->CTRLB = DAC_CHSEL_SINGLE_gc;
    dacptr->CTRLC = DAC_REFSEL_INT1V_gc;
    //dacptr->TIMCTRL = DAC_CONINTVAL_32CLK_gc;

    // Set up the ADC
    // All channels will be connected to an internal input.
    adcptr->CH0.CTRL = ADC_CH_INPUTMODE_INTERNAL_gc; 
    adcptr->CH1.CTRL = ADC_CH_INPUTMODE_INTERNAL_gc;
    adcptr->CH2.CTRL = ADC_CH_INPUTMODE_INTERNAL_gc;
    adcptr->CH3.CTRL = ADC_CH_INPUTMODE_INTERNAL_gc;

    // MUX selection: measure from DAC.
    adcptr->CH0.MUXCTRL = ADC_CH_MUXINT_DAC_gc;
    adcptr->CH1.MUXCTRL = ADC_CH_MUXINT_DAC_gc;
    adcptr->CH2.MUXCTRL = ADC_CH_MUXINT_DAC_gc;
    adcptr->CH3.MUXCTRL = ADC_CH_MUXINT_DAC_gc;

    // Enable ADC 
    adcptr->CTRLA = ADC_ENABLE_bm;
	// Signed conversions, 12 bit resolution
    adcptr->CTRLB = ADC_CONMODE_bm | ADC_RESOLUTION_12BIT_gc;
	// 1V reference 
    adcptr->REFCTRL = ADC_REFSEL_INT1V_gc;
	// Pre-scale clock by 512.
    adcptr->PRESCALER = ADC_PRESCALER_DIV512_gc;

    
	// Write five values to the DAC and read them from the ADC.
	// -For the DAC the range is (0x000,0xFFF)
	// -For the ADC the positive range is (0x000,0x7FF).
	// When The DAC generates 25% of its range, we expect 25% of 
	// the range in the ADC. Similarly on the other cases.
	classb_dac_adc_test(dacptr, adcptr, 0x000, 0x000);
	classb_dac_adc_test(dacptr, adcptr, 0x400, 0x200);
	classb_dac_adc_test(dacptr, adcptr, 0x800, 0x400);
	classb_dac_adc_test(dacptr, adcptr, 0xC00, 0x600);
	classb_dac_adc_test(dacptr, adcptr, 0xFFF, 0x7FF);
	

    // Disable the ADC and DAC
    adcptr->CTRLA &= ~ADC_ENABLE_bm;
    dacptr->CTRLA &= ~DAC_ENABLE_bm;
}
