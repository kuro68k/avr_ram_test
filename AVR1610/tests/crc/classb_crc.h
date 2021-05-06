/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief
 *		Settings and defines used in connection with CRC memory tests.
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

#ifndef __CRC_H__
#define __CRC_H__

#include "avr_compiler.h"
#include "error_handler.h"

/**
 * \defgroup classb_crc CRC Tests
 * 
 * \brief This CRC module can be used to check the Flash and EEPROM memories.
 * 
 * \section crc_intro Introduction to CRC
 * 
 * A cyclic redundancy check (CRC) is an error detection technique test algorithm 
 * used to detect errors in data. It is commonly used to determine the correctness 
 * of data stored in memory or data that is received through some communication 
 * channel. The CRC algorithm takes a data stream or a block of data as input and 
 * generates a 16- or 32-bit output that can be used as a checksum. We suggest 
 * two possible methods to detect errors using this checksum: 
 * 
 * -Compute checksum of the data section and compare with the previous one. 
 * If they are different, there is an error.
 * 
 * -Compute checksum of the extended data section (data section and original checksum). 
 * The resulting checksum should be a fixed implementation-specific value, 
 * otherwise there is an error.
 * 
 * If there were errors in the data, the application should take corrective actions, 
 * e.g. requesting the data to be sent again or simply not using the incorrect data. 
 *
 * The CRC Tests module implements 16-bit CCITT CRC and IEE802.3 32-bit CRC algorithms 
 * in software and hardware.
 *  - \ref classb_crc_sw 
 *  - \ref classb_crc_hw 
 * 
 * \section crc_usage Usage
 * 
 * There are a number of symbols in \ref crc_conf that can be defined or undefined
 * in order to choose between software and hardware implementations, and 16-bit and 
 * 32-bit CRC. The software implementation can be further configured in the 
 * corresponding header file. To calculate an EEPROM checksum, use the functions 
 *  - \c CLASSB_CRC[16/32]_EEPROM_[SW/HW] 
 * 
 * To calculate a Flash checksum, use the functions 
 *  - \c CLASSB_CRC[16/32]_Flash_[SW/HW].
 *  
 * If there should be any error, the error handler \ref CLASSB_ERROR_HANDLER_CRC() would 
 * be called.
 *  
 *  \note Interrupts must be disabled during this test.
 * 
 * \section  crc_speed Execution speed
 * 
 * We provide an estimate for the execution speed of different CRC implementations. Note that this is 
 * based on empirical results.
 * - EEPROM
 * 	- CRC32 lookup table is ~45% slower than HW.
 * 	- CRC32 direct calculation is ~750% slower than HW.
 * 	- CRC16 lookup table is ~20% slower than HW. 
 * 	- CRC16 direct calculation is ~430% slower than HW.
 * - Flash
 * 	- CRC32 lookup table is ~4600% slower than HW. 
 * 	- CRC32 direct calculation is ~15800% slower than HW.
 * 	- CRC16 lookup table is ~3% slower than HW. 
 * 	- CRC16 direct calculation is ~275% slower than HW.
 */


//! \addtogroup classb_crc
//@{



//! \defgroup crc_conf Configuration of CRC Implementation 
//! 
//! \brief Symbols that can be defined to configure the CRC computation. 
//! 
//! This is used to avoid compiling code that will not be used.
//@{
//! \brief Compile hardware implementation
#define CLASSB_CRC_USE_HW 
//! \brief Compile software implementation
#define CLASSB_CRC_USE_SW 
//! \brief Compile 16-bit functions 
#define CLASSB_CRC_16_BIT 
//! \brief Compile 32-bit functions 
#define CLASSB_CRC_32_BIT 
//@}


//! \internal \defgroup crc_int_conf CRC settings that should not be modified 
//! 
//! \brief This options make sure that the CRC computation complies with 
//! the standard 16-bit CCITT CRC and IEE802.3 32-bit CRC.
//@{
//! \internal \brief Initial remainder for the 16-bit CRC
#define CRC16_INITIAL_REMAINDER 0x0000
//! \internal \brief Initial remainder for the 32-bit CRC
#define CRC32_INITIAL_REMAINDER	0xFFFFFFFF
//@}


//! \defgroup crc_types Type definitions for CRC functions
//! 
//! \brief Data types used in connection with CRC computations.
//! 
//! This includes the type for variables that stores the data size in bytes, 
//! memory addresses in Flash, and pointers to memory. Both for Flash and EEPROM 
//! there are void pointer types and pointers to byte. Variables can be assigned 
//! a memory attribute with IAR. Therefore, pointers will be large enough to hold 
//! the address of a given variable. However, with GCC pointers have 16 bits. This 
//! means that if a device has a large program  memory, we have to use 32-bits 
//! variables to store Flash addresses. 
//@{

/*! \brief Data type to use for byte counts in CRC computations.
 * This type should be kept as small as possible to optimize for speed.
 */
typedef uint32_t crcbytenum_t;

/*! \brief Data type for holding flash memory addresses.
 *	Depending of the size of the device it will be necessary to use 32 or 16 bits.
 */
#if (PROGMEM_SIZE >= 0x10000UL) || defined(__DOXYGEN__)
 typedef uint32_t flash_addr_t;
#else
 typedef uint16_t flash_addr_t;
#endif

// Pointer types for CRC functions, needed for IAR and GCC compatibility.
#if defined(__ICCAVR__)
 typedef  const void __eeprom * eepromptr_t;
 typedef const uint8_t __eeprom * eeprom_uint8ptr_t;
 typedef const uint16_t __eeprom * eeprom_uint16ptr_t;
 typedef const uint32_t __eeprom * eeprom_uint32ptr_t;
 typedef  const void _MEMATTR * flashptr_t;
 typedef  const uint8_t _MEMATTR * flash_uint8ptr_t;
#elif defined (__GNUC__) || defined (__DOXYGEN__)
 //! \brief Generic pointer to EEPROM. 
 typedef const void* eepromptr_t;
 //! \brief Pointer to a byte in EEPROM. 
 typedef const uint8_t* eeprom_uint8ptr_t;
 //! \brief Pointer to two bytes in EEPROM. 
  typedef const uint16_t * eeprom_uint16ptr_t;
  //! \brief Pointer to four bytes in EEPROM. 
 typedef const uint32_t * eeprom_uint32ptr_t;
 //! \brief Generic pointer to Flash. 
 typedef flash_addr_t flashptr_t; 
 //! \brief Pointer to a byte in Flash. 
 typedef flash_addr_t flash_uint8ptr_t;
#else
 #error Unknown compiler!
#endif

//@}

//@}

#ifdef CLASSB_CRC_USE_HW
 #include "classb_crc_hw.h"
#endif

#ifdef CLASSB_CRC_USE_SW
 #include "classb_crc_sw.h"
#endif

#if defined(__GNUC__) && !defined(__OPTIMIZE__)
# error Optimization must be enabled to successfully write to protected registers, due to timing constraints.
#endif


#endif

