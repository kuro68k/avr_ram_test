/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file
 *
 * \brief Definitions and settings for the CRC implemented in software.		
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

#ifndef __CRC_H_SW__
#define __CRC_H_SW__

#include "avr_compiler.h"
#include "classb_crc.h"

//! \ingroup classb_crc
//! 
//! \defgroup classb_crc_sw CRC software implementation
//! 
//! \brief CRC computations implemented in software.
//! 
//! The CPU reads the data and computes the CRC checksum. There are two available 
//! methods: 
//!   - Lookup table: this uses a CRC lookup table to speed up the computations. 
//!   The lookup table requires 512 (for 16 bit) or 1024 (for 32 bit) bytes of flash memory. 
//!   - Direct computation: this calculates the checksum for each byte using a polynomial 
//!   division each time it is called. This version occupies no space in flash memory, 
//!   but is 3.5-4x slower than lookup table method.
//! 
//! The CRC32-polynomial is in the reflected form (0xEDB88320) in the software 
//! implementation. The initial remainder is 0xFFFFFFFF, and the generated checksum 
//! is bit-reversed and complemented (in compliance with IEE802.3). The CCITT polynomial 
//! used is 0x1021, with 0x0000 as initial remainder. In this case the checksum is neither 
//! bit reversed nor complemented.
//! 
 
//@{

/**
 * \name Settings for CRC software implementation
 *
 * \brief These symbols are used to choose between lookup table or direct computation. 
 * They should be defined for lookup table implementation and commented out otherwise.
 * 
 */
//@{
#ifdef __DOXYGEN__
 //! Select the lookup table method for 16-bit CRC 
 #define CRC_USE_16BIT_LOOKUP_TABLE 
 //! Select the lookup table method for 32-bit CRC
 #define CRC_USE_32BIT_LOOKUP_TABLE 
#else
 #define CRC_USE_16BIT_LOOKUP_TABLE 
 #define CRC_USE_32BIT_LOOKUP_TABLE 
#endif
//@}


/**
 * \internal
 * 
 * \name This settings should not be modified
 * 
 * \brief This options make sure that the CRC computation complies with the 
 * standard 16-bit CCITT CRC and IEE802.3 32-bit CRC.
 */
//@{
//! \internal  \brief Polynomial used for computation of CRC16 (software, not lookup table)
#define CRC16_POLYNOMIAL 0x1021 // CCITT:  0x1021  <->  x^16 + x^12 + x^5 + 1 
//! \internal  \brief Polynomial used for computation of CRC32 (software, not lookup table)
#define CRC32_POLYNOMIAL  0xEDB88320UL //IEE802.3: 0xEDB88320 is the reflection of 0x04C11DB7
//! \internal  \brief Final XOR Value for CRC32
#define CRC32_FINAL_XOR_VALUE	0xFFFFFFFF
//@}

//! \internal 
//! 
//! \name Macros used for CRC computation

//@{

/*! \internal\brief Update CRC value for one input byte.
 *
 * \note This macro works for both 16 and 32 bit CRC.
 *
 * \param data    Input data byte.
 * \param crc     Variable that holds the CRC value.
 * \param poly    CRC polynomial.
 * \param crcbits Number of CRC bits.
 */
#define CLASSB_CRC(data,crc,poly,crcbits)  { \
    /* Bring the next byte into the checksum. */ \
    crc ^= ((uint32_t)data << (crcbits - 8)); \
\
    uint8_t bit; \
\
    /* Perform modulo-2 division. */ \
    for (bit = 8; bit > 0; --bit) { \
        /* ..only if MSB is set. */ \
        if ( crc & ((uint32_t)1 << (crcbits - 1)) ) { \
            crc = (crc << 1) ^ poly; \
        } \
        else { \
            crc <<= 1; \
        } \
    } \
}

/*! \internal \brief Update CRC value for one input byte (reflected polynomial).
 *
 * \note This macro works for both 16 and 32 bit CRC.
 *
 * \param data    Input data byte.
 * \param crc     Variable that holds the CRC value.
 * \param poly    CRC polynomial.
 * \param crcbits Number of CRC bits.
 */
#define CLASSB_CRC_REFL(data,crc,poly,crcbits) { \
    /* Bring next byte into the checksum. */ \
    crc ^= (uint32_t)data; \
\
    uint8_t bit; \
\
    /* Perform modulo-2 division. */ \
    for (bit = 8; bit > 0; --bit) { \
        /* ..only if LSB is set. */ \
        if (crc & 0x01) { \
            crc = (crc >> 1) ^ poly; \
        } \
        else { \
            crc >>= 1; \
        } \
    } \
}

/*! \internal \brief Update 32-bit CRC value for one input byte (reflected polynomial).
 *
 * \note This macro assumes that the CRC lookup table is located in the lower
 * 64 kB address range of Flash.
 * 
 * \param data  Input data byte.
 * \param crc   Variable that holds the CRC value.
 * \param table Table that contains pre-calculated CRC values.
 */
#define CLASSB_CRC_REFL_TABLE_32(data,crc,table) { \
    data ^= crc & 0xFF; \
    crc = PROGMEM_READ_DWORD(&table[data]) ^ (crc >> 8); \
}

/*! \internal\brief Update 16-bit CRC value for one input byte.
 *
 * \note This macro assumes that the CRC lookup table is located in the lower
 * 64 kB address range of Flash.
 *
 * \param data  Input data byte.
 * \param crc   Variable that holds the CRC value.
 * \param table Table that contains pre-calculated CRC values.
 */
#define CLASSB_CRC_TABLE_16(data,crc,table) { \
    data ^= crc >> (16 - 8); \
    crc = PROGMEM_READ_WORD(&table[data]) ^ (crc << 8); \
}

//@}

//! \name CRC tests
//! 
//! \brief Invariant memory tests based on CRC that are compliant with IEC60730 Class B.
//@{
uint16_t CLASSB_CRC16_EEPROM_SW (eepromptr_t dataptr, const crcbytenum_t numBytes, eeprom_uint16ptr_t pchecksum);
uint16_t CLASSB_CRC16_Flash_SW (flashptr_t dataptr, const crcbytenum_t numBytes, eeprom_uint16ptr_t pchecksum);

uint32_t CLASSB_CRC32_EEPROM_SW (eepromptr_t dataptr, const crcbytenum_t numBytes, eeprom_uint32ptr_t pchecksum);
uint32_t CLASSB_CRC32_Flash_SW (flashptr_t dataptr, const crcbytenum_t numBytes, eeprom_uint32ptr_t pchecksum);
//@}


//@}

#endif