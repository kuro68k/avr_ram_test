/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief
 *      Demo application for the CRC tests.
 *
 *      This is an example that shows how the Flash and EEPROM CRC tests can be 
 *      embedded in an user application. The CRC checksums of the Flash and a data 
 *      section in EEPROM are pre-calculated and stored in EEPROM. During normal 
 *      execution an LED is lighted while the system runs without errors. Pressing 
 *      a button leads to CRC test of the Flash and the data section in EEPROM: 
 *      new checksums are computed and compared with the ones stored in EEPROM. 
 *      If any of the pairs were different, the error flag would be set (within 
 *      the test) and the LED would be switched off. This can be tested for example 
 *      by declaring the symbols \ref MODIFY_FLASH and \ref MODIFY_EEPROM.
 *	
 *      Note that if the program is modified, the stored CRC needs to be computed 
 *      again since the content in Flash will change. This can be done by debugging 
 *      the program and placing a watch on the following variables:
 *		  - \c checksum_test_flash        
 *		  - \c checksum_test_eeprom
 *		  .
 *      Those values can be assigned then to
 *        - \ref classb_precalculated_flash_crc 
 *        - \ref classb_precalculated_eeprom_crc
 *        .
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
#include "classb_crc.h"


#ifdef __DOXYGEN__
 //! \brief If declared the content of the flash is modified.
 //! 
 //! If declared an endless loop is compiled at the end of the main application. 
 //! This changes the program code and thus the flash content. The previously stored 
 //! checksum will then be different than the new one.
 #define MODIFY_FLASH
#else 
 #define MODIFY_FLASH
#endif

#ifdef __DOXYGEN__
 //! \brief If declared the content of the EEPROM data section is modified.
 //! 
 //! If declared an endless loop is compiled at the end of the main application. 
 //! This changes the program code and thus the flash content. The previously stored 
 //! checksum will then be different than the new one.
 #define MODIFY_EEPROM
#else 
 //#define MODIFY_EEPROM
#endif	

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

//! \brief This function sets up the LED and switch.
//! The LED is switched on to show that the system is working correctly. 
//! The switch is configured to generate an interrupt after being pressed.
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
	
	// Turn on LED that signals correct operation.	
	LEDPORT.OUTSET = PIN0_bm;
}


//!\brief This variable contains the CRC checksum for the flash and is stored in EEPROM.
#if defined(__GNUC__) || defined(__DOXYGEN__)
uint32_t EEPROM_DECLARE( classb_precalculated_flash_crc )  = 0xe7138241; 
#elif defined(__ICCAVR__)
uint32_t EEPROM_DECLARE( classb_precalculated_flash_crc )  = 0x4E4C04C6;
#endif

//!\brief This variable contains the CRC checksum for the data section in EEPROM. It is stored in EEPROM.
uint16_t EEPROM_DECLARE( classb_precalculated_eeprom_crc )  = 0xcd4b;

#if !defined(MODIFY_EEPROM) | defined(__DOXYGEN__)
 //! \brief Data stored in EEPROM.
 uint8_t EEPROM_DECLARE( data_eeprom[] ) = {0,1,2,3,4,5,6,7,8,9,10};
#else
 uint8_t EEPROM_DECLARE( data_eeprom[] ) = {0,0,0,0,0,0,0,0,0,0,0};
#endif	

//! \brief Global error flag
NO_INIT volatile uint8_t classb_error;

//The current toolchain version would not show correct values unless they were declared globally.
volatile uint32_t checksum_test_flash_2;
volatile uint32_t checksum_test_flash_3;
volatile uint16_t checksum_test_flash_4;
volatile uint16_t checksum_test_flash_5;
int main(void)
{
  
    setup_led_switches();	
	// Enable interrupts:
	sei();	
	
    while(!classb_error) { 
		// Do nothing
	};
	
	// If this is executed there has been an error.
	// Disable interrupts
	cli();
	// Turn off LED to indicate error.
	LEDPORT.OUTCLR = PIN0_bm;
	
#ifdef	MODIFY_FLASH
	while(1);
#endif

}


//! \brief Interrupt service routine for button press.
//! 
//! This will compute the checksums of the Flash and data section in EEPROM and compare 
//! them with the ones stored in EEPROM. There some code lines that could be uncommented
//! in order to check the other CRC implementations.
ISR(SWITCH_INT0_vect) {
  	
	// Compute the checksum of the Flash
	volatile uint32_t checksum_test_flash = CLASSB_CRC32_Flash_HW (CRC_APP, 0, 0, &classb_precalculated_flash_crc);
	//// These lines should produce the same checksum
	//checksum_test_flash_2 = CLASSB_CRC32_Flash_HW (CRC_FLASH_RANGE, APP_SECTION_START, APP_SECTION_SIZE, &classb_precalculated_flash_crc);
	//checksum_test_flash_3 = CLASSB_CRC32_Flash_SW (APP_SECTION_START, APP_SECTION_SIZE, &classb_precalculated_flash_crc);
	//// These lines should produce the same checksum (different than the CRC16 checksum)
	//checksum_test_flash_4 = CLASSB_CRC16_Flash_HW (APP_SECTION_START, APP_SECTION_SIZE, &classb_precalculated_flash_crc);
	//checksum_test_flash_5 = CLASSB_CRC16_Flash_SW (APP_SECTION_START, APP_SECTION_SIZE, &classb_precalculated_flash_crc);
	
	// Compute the checksum of the data section in EEPROM
	volatile uint16_t checksum_test_eeprom = CLASSB_CRC16_EEPROM_HW(data_eeprom, sizeof(data_eeprom), &classb_precalculated_eeprom_crc);
	//// This line should produce the same checksum
	//volatile uint16_t checksum_test_eeprom_2 = CLASSB_CRC16_EEPROM_SW(data_eeprom, sizeof(data_eeprom), &classb_precalculated_eeprom_crc);
	//// These lines should produce the same checksum (different than the CRC16 checksum)
	//volatile uint32_t checksum_test_eeprom_3 = CLASSB_CRC32_EEPROM_HW(data_eeprom, sizeof(data_eeprom), &classb_precalculated_eeprom_crc);
	//volatile uint32_t checksum_test_eeprom_4 = CLASSB_CRC32_EEPROM_SW(data_eeprom, sizeof(data_eeprom), &classb_precalculated_eeprom_crc);
	
}


