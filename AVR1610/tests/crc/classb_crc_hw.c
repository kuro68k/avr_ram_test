/**
 * \file
 *
 * \brief Driver for 16- and 32-bit CRC using CRC hardware module.
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

#include "classb_crc_hw.h"

//! \ingroup classb_crc_hw
//@{

/******************* CRC DRIVER Common ******************/

#if defined(CLASSB_CRC_USE_HW) || defined(__DOXYGEN__)


//! \internal 
//! 
//! \brief Initial CRC value to use on next CRC calculation.
uint32_t crc_initial_value = 0;


/**
 * \internal
 * 
 * \brief Non-Volatile Memory Execute Specific Command
 *
 * This function sets a command in the NVM.CMD register, then performs an
 * execute command by writing the CMDEX bit to the NVM.CTRLA register.
 *
 * \note The function saves and restores the NVM.CMD register, but if this
 *       function is called from an interrupt, interrupts must be disabled
 *       before this function is called.
 *
 * \param nvm_command NVM Command to execute.
 */
static inline void nvm_issue_command(NVM_CMD_t nvm_command)
{
	uint8_t old_cmd;

	old_cmd = NVM.CMD;
	NVM.CMD = nvm_command;

	CCP = CCP_IOREG_gc;
	NVM.CTRLA = NVM_CMDEX_bm;

	NVM.CMD = old_cmd;
}

/**
 * \internal
 * 
 * \brief Check if the CRC module is busy
 *
 * This function can be used to see if a CRC checksum generation is completed.
 *
 * \retval true if module is busy
 * \retval false if module is not busy
 *
 */
static inline bool crc_is_busy(void)
{
	return ((CRC_STATUS & CRC_BUSY_bm) == CRC_BUSY_bm);
}

/**
 * \internal
 * 
 * \brief Add one byte of data to CRC performed on I/O interface
 *
 * This function writes a byte to the DATAIN register. Each time this function
 * is called, the checksum is calculated for the new data appended to all
 * previous data written since the \ref crc_io_checksum_byte_start was called.
 * A new checksum is ready one clock cycle after the DATAIN register is
 * written.
 *
 * \param data  data to perform CRC on
 *
 */
void crc_io_checksum_byte_add(uint8_t data)
{
	CRC_DATAIN = data;
}

/**
 * \internal
 * 
 * \brief Reset CRC module and set CHECKSUM to initial value
 *
 * The CRC registers will be reset one peripheral clock cycle after the
 * RESET[1] bit is set. The initial value is reset to 0 after loading it into
 * the CHECKSUM registers.
 *
 */
static inline void crc_reset(void)
{
	// Reset module
	CRC_CTRL |= CRC_RESET_RESET0_gc;

	// Set initial checksum value
	CRC.CHECKSUM0 = crc_initial_value & 0xFF;
	CRC.CHECKSUM1 = (crc_initial_value >> 8) & 0xFF;
	CRC.CHECKSUM2 = (crc_initial_value >> 16) & 0xFF;
	CRC.CHECKSUM3 = (crc_initial_value >> 24) & 0xFF;

	crc_initial_value = 0;
}


/**
 * \internal
 * 
 * \brief Read CRC-16 checksum
 *
 * This function returns the computed CRC-16 checksum.
 *
 * \return checksum checksum value
 */
static inline uint16_t crc16_checksum_read(void)
{
	uint16_t checksum;
	checksum = 0;
	checksum = ((uint16_t)CRC_CHECKSUM0 & 0x00FF);
	checksum |= (((uint16_t)CRC_CHECKSUM1 << 8) & 0xFF00);

	return checksum;
}

/**
 * \internal
 * 
 * \brief Read CRC-32 checksum
 *
 * This function extracts the CRC-32 checksum from the registers.
 *
 * \note In IEEE 802.3 CRC-32 the checksum is bit reversed
 *       and complemented.
 *
 * \return checksum checksum value
 */
static inline uint32_t crc32_checksum_read(void)
{
	uint32_t checksum;

	checksum = ((uint32_t)CRC_CHECKSUM0 & 0x000000FF);
	checksum |= (((uint32_t)CRC_CHECKSUM1 << 8) & 0x0000FF00);
	checksum |= (((uint32_t)CRC_CHECKSUM2 << 16) & 0x00FF0000);
	checksum |= (((uint32_t)CRC_CHECKSUM3 << 24) & 0xFF000000);

	return checksum;
}

/**
 * \internal
 * 
 * \brief Read checksum
 *
 * This function waits until the CRC conversion is complete and returns
 * either the 32 bit or 16 bit checksum as a 32 bit value.
 *
 * \note This function must not be called before a IO CRC calculation is
 * completed or a FLASH or DMA CRC calculation is started, as this will cause
 * it to wait for the BUSY flag to be cleared forever.
 *
 * \return checksum checksum value
 */
static inline uint32_t crc_checksum_read(void)
{
	// Wait until the CRC conversion is finished
	while (crc_is_busy()) {
		// Do nothing
	}

	// Check if we have a 32 or 16 bit checksum and return the correct one
	if ((CRC_CTRL & CRC_CRC32_bm) == CRC_CRC32_bm) {
		return crc32_checksum_read();
	} else {
		return crc16_checksum_read();
	}
}

/**
 * \internal
 * 
 * \brief Set data source for the CRC module after reset
 *
 * The selected source is locked until either the CRC generation is completed
 * or the CRC module is reset.
 * CRC generation complete is generated and signaled from the selected source
 * when used with the DMA controller or flash memory.
 * To disable the CRC module, use CRC_SOURCE_DISABLE_gc as source.
 *
 * \param source the data source for the CRC module
 */
static inline void crc_set_source(CRC_SOURCE_t source)
{
	CRC_CTRL &= ~CRC_SOURCE_gm;
	CRC_CTRL |= source;
}



/**
 * \internal
 * 
 * \brief Disable the CRC module
 *
 */
static inline void crc_disable(void)
{
	crc_set_source(CRC_SOURCE_DISABLE_gc);
}

/**
 * \internal
 * 
 * \brief Complete CRC calculation by reading checksum and disabling module
 *
 * \return checksum checksum value
 */
static inline uint32_t crc_checksum_complete(void)
{
	uint32_t checksum;

	// Read checksum
	checksum = crc_checksum_read();

	// Disable CRC module
	crc_disable();

	// Return checksum
	return checksum;
}


/**
 * \internal
 * 
 * \brief Set initial CRC value for next CRC calculation
 *
 * \note Value is reset to 0 when CRC calculation completes.
 *
 * \param value Initial CRC value
 */
void crc_set_initial_value(uint32_t value)
{
	crc_initial_value = value;
}


/**
 * \internal
 * 
 * \brief Calculate the CRC checksum for data in a buffer
 *
 * This function initializes the CRC module, calculates the checksum for the
 * selected number of bytes in a data buffer, disables the module and returns
 * the calculated checksum.
 *
 * \param data          data buffer to perform CRC on
 * \param len           the number of bytes to perform CRC on
 * \param crc_16_32     enum to indicate whether CRC-32 or CRC-16 shall be used
 *
 * \return checksum checksum
 */
uint32_t crc_io_checksum(void *data, uint16_t len, enum crc_16_32_t crc_16_32)
{
	// Initialize CRC calculations on I/O interface
	crc_io_checksum_byte_start(crc_16_32);

	// Write data to DATAIN register
	while (len--) {
		crc_io_checksum_byte_add(*(uint8_t*)data);
		data = (uint8_t*)data + 1;
	}

	// Return checksum
	return crc_io_checksum_byte_stop();
}


/**
 * \internal
 * 
 * \brief Enable CRC-32
 *
 * This function will set the CRC-32 polynomial to be used to generate
 * the checksum.
 *
 * \note This cannot be changed while the busy flag is set.
 *
 */
static inline void crc_32_enable(void)
{
	CRC_CTRL |= CRC_CRC32_bm;
}


/**
 * \internal
 *
 * \brief Reset CRC module and set source to I/O interface
 *
 * This function initializes the CRC module, and \ref crc_io_checksum_byte_add
 * can be used to add bytes to calculate checksum for. When any number of bytes
 * have been added, the \ref crc_io_checksum_byte_stop can be called to disable
 * the module and get the calculated checksum.
 *
 * \param crc_16_32     enum to indicate whether CRC-32 or CRC-16 shall be used
 */
void crc_io_checksum_byte_start(enum crc_16_32_t crc_16_32)
{
	// Initialize CRC calculations on I/O interface
	crc_reset();
	// Enable CRC-32 if chosen
	if (crc_16_32 == CRC_32BIT) {
		crc_32_enable();
	}
	// Enable CRC module using the I/O interface
	crc_set_source(CRC_SOURCE_IO_gc);
}

/**
 * \internal
 * 
 * \brief Disable CRC module and return checksum
 *
 * This function stops the CRC calculation for bytes added with the \ref
 * crc_io_checksum_byte_add and returns the calculated checksum.
 *
 * \retval checksum checksum
 *
 */
uint32_t crc_io_checksum_byte_stop(void)
{
	// Signal CRC complete
	CRC_STATUS |= CRC_BUSY_bm;

	// Stop CRC and return checksum
	return crc_checksum_complete();
}


#endif //  defined(CLASSB_CRC_USE_HW)


/******************* CRC16 Functions ******************/

#if (defined(CLASSB_CRC_USE_HW) && defined(CLASSB_CRC_16_BIT)) || defined(__DOXYGEN__)


/*! \brief Compute 16-bit CRC for EEPROM address range using hardware CRC module.
 *
 * This function returns the 16-bit CRC of the specified EEPROM address range.
 *
 * \param origDataptr Address of EEPROM location to start CRC computation at.
 * \param numBytes    Number of bytes of the data.
 * \param pchecksum	  Pointer to the checksum stored in EEPROM.
 *
 * \note No sanity checking of addresses is done. 
 */
uint16_t CLASSB_CRC16_EEPROM_HW (eepromptr_t origDataptr, crcbytenum_t numBytes, eeprom_uint16ptr_t pchecksum)
{
    eeprom_uint8ptr_t dataptr = origDataptr;
	
	crc_set_initial_value(CRC16_INITIAL_REMAINDER);

    // Ensure that EEPROM is memory mapped.
    CLASSB_EEMAP_BEGIN();
    dataptr += MAPPED_EEPROM_START;

	uint32_t checksum = crc_io_checksum((void*)dataptr, numBytes, CRC_16BIT);
	
	
	#if defined(__ICCAVR__)	
     // Disable memory mapping of EEPROM, if necessary.
     CLASSB_EEMAP_END();
	 // Compare checksums and handle error if necessary.
	 if ( checksum != *pchecksum) 
		 CLASSB_ERROR_HANDLER_CRC();
	#elif defined(__GNUC__)	 
	 // Compare checksums and handle error if necessary.
	 if ( checksum != *(eeprom_uint16ptr_t)(MAPPED_EEPROM_START + (uintptr_t) pchecksum)) 
	  	 CLASSB_ERROR_HANDLER_CRC();
     // Disable memory mapping of EEPROM, if necessary.
     CLASSB_EEMAP_END();	 
	#endif



	// Return 16 bits
    return (checksum & 0x0000FFFF);
}


/*! \brief Compute 16-bit CRC for Flash address range using hardware CRC module.
 *
 * This function returns the 16-bit CRC of the specified Flash address range.
 *
 * \param origDataptr Address of Flash location to start CRC computation at.
 * \param numBytes    Number of bytes of the data.
 * \param pchecksum	  Pointer to the checksum stored in EEPROM.
 *
 * \note 16-bit flash CRC is much slower than 32-bit for flash checking.
 * \note No sanity checking of addresses is done.
 */
uint16_t CLASSB_CRC16_Flash_HW (flashptr_t origDataptr, crcbytenum_t numBytes, eeprom_uint16ptr_t pchecksum )
{
    flash_uint8ptr_t dataptr = origDataptr;
    uint8_t dataTemp;
	
	crc_set_initial_value(CRC16_INITIAL_REMAINDER);

    crc_io_checksum_byte_start(CRC_16BIT);

    // Compute CRC for the specified data.
    for (; numBytes != 0; numBytes--) {
		#if (PROGMEM_SIZE >= 0x10000UL) 
		 dataTemp = PROGMEM_READ_BYTE_FAR(dataptr++);
		#else
         dataTemp = PROGMEM_READ_BYTE(dataptr++);
		#endif
        
		crc_io_checksum_byte_add(dataTemp);		
    }

	uint32_t checksum = crc_io_checksum_byte_stop();
	
	#if defined(__ICCAVR__)	
	 // Compare checksums and handle error if necessary.
	 if ( checksum != *pchecksum) 
		 CLASSB_ERROR_HANDLER_CRC();
	#elif defined(__GCC__)
	 // Ensure that EEPROM is memory mapped.
	 CLASSB_EEMAP_BEGIN();
	 // Compare checksums and handle error if necessary.
	 if ( checksum != *(eeprom_uint16ptr_t)(MAPPED_EEPROM_START + (uintptr_t) pchecksum)) 
	  	 CLASSB_ERROR_HANDLER_CRC();
	 // Disable memory mapping of EEPROM, if necessary.
	 CLASSB_EEMAP_END();
	#endif

    return (checksum & 0x0000FFFF);
}




/**
 * \internal
 * 
 * \brief Appends a CRC value to the given pointer
 *
 * Taking the checksum of the value and the appended data will result in zero.
 *
 * \note At least two bytes must be reserved at the given pointer location.
 *
 * \param value CRC value to append
 * \param ptr pointer to 2 byte memory location to store value.
 *
 */
void crc16_append_value(uint16_t value, void *ptr)
{
	*(uint8_t*)ptr = (value >> 8);
	*((uint8_t*)ptr+1) = value;
}

#endif // defined(CLASSB_CRC_USE_HW) && defined(CLASSB_CRC_16_BIT)


/******************* CRC32 Functions ******************/

#if (defined(CLASSB_CRC_USE_HW) && defined(CLASSB_CRC_32_BIT)) || defined(__DOXYGEN__)

/** 
 * \brief Compute 32-bit CRC for EEPROM address range using hardware CRC module.
 *
 * This function returns the 32-bit CRC of the specified EEPROM address range.
 *
 * \param origDataptr Address of EEPROM location to start CRC computation at.
 * \param numBytes    Number of bytes of the data.
 * \param pchecksum	  Pointer to the checksum stored in EEPROM.
 *
 * \note No sanity checking of addresses is done.
 */
uint32_t CLASSB_CRC32_EEPROM_HW (eepromptr_t origDataptr, crcbytenum_t numBytes, eeprom_uint32ptr_t pchecksum)
{
    eeprom_uint8ptr_t dataptr = origDataptr; 
	crc_set_initial_value(CRC32_INITIAL_REMAINDER);

    // Ensure that EEPROM is memory mapped.
    CLASSB_EEMAP_BEGIN();
    dataptr += MAPPED_EEPROM_START;

	uint32_t checksum = crc_io_checksum((void*)dataptr, numBytes, CRC_32BIT);
	
	#if defined(__ICCAVR__)	
	 // Disable memory mapping of EEPROM, if necessary.
	 CLASSB_EEMAP_END();
	 // Compare checksums and handle error if necessary.
	 if ( checksum != *pchecksum) 
		 CLASSB_ERROR_HANDLER_CRC();
	#elif defined(__GCC__)	
	 // Compare checksums and handle error if necessary.
	 if ( checksum != *(eeprom_uint32ptr_t)(MAPPED_EEPROM_START + (uintptr_t) pchecksum)) 
	  	 CLASSB_ERROR_HANDLER_CRC();
	 // Disable memory mapping of EEPROM, if necessary.
     CLASSB_EEMAP_END();
	#endif

    

    return (checksum);
}


/** \brief Compute 32-bit CRC for Flash address range using hardware CRC module.
 *
 * This function returns the 32-bit CRC of the specified Flash address range.
 *
 * \param crc_type    Type of CRC computation: \ref CRC_FLASH_RANGE, \ref CRC_BOOT or \ref CRC_APP.
 * \param origDataptr Address of Flash location to start CRC computation at.
 * \param numBytes    Number of bytes of the data.
 * \param pchecksum	  Pointer to the checksum stored in EEPROM.
 *
 * \note No sanity checking of addresses is done.
 */
uint32_t CLASSB_CRC32_Flash_HW (NVM_CMD_t crc_type, flashptr_t origDataptr, crcbytenum_t numBytes, eeprom_uint32ptr_t pchecksum)
{
    flash_uint8ptr_t dataptr = origDataptr;
	crc_set_initial_value(CRC32_INITIAL_REMAINDER);
	
    // Compute CRC for the specified data.
	uint32_t checksum = crc_flash_checksum(crc_type, (flash_addr_t)dataptr, numBytes);
	
	#if defined(__ICCAVR__)	
	 // Compare checksums and handle error if necessary.
	 if ( checksum != *pchecksum) 
		 CLASSB_ERROR_HANDLER_CRC();
	#elif defined(__GNUC__)
	 // Ensure that EEPROM is memory mapped.
	 CLASSB_EEMAP_BEGIN();
	 // Compare checksums and handle error if necessary.
	 if ( checksum != *(eeprom_uint32ptr_t)(MAPPED_EEPROM_START + (uintptr_t) pchecksum)) 
	  	 CLASSB_ERROR_HANDLER_CRC();
	 // Disable memory mapping of EEPROM, if necessary.
	 CLASSB_EEMAP_END();
	#endif
	
    return(checksum);
}


/**
 * \internal
 * 
 * \brief Get and append data to generate checksum zero with CRC32
 *
 * This function calculates checksum value to append to the data after
 * generating its checksum.
 * The value is written to given pointer as little endian.
 * Taking the checksum of the data appended will result in checksum zero.
 *
 * \note At least four bytes must be reserved at the given pointer location.
 *
 * \param value the value to convert to append value
 * \param ptr pointer to 4 byte memory location to store the value.
 *
 */
void crc32_append_value(uint32_t value, void *ptr)
{
	*(uint8_t*)ptr = value & 0xFF;
	*((uint8_t*)ptr+1) = (value >> 8) & 0xFF;
	*((uint8_t*)ptr+2) = (value >> 16) & 0xFF;
	*((uint8_t*)ptr+3) = (value >> 24) & 0xFF;
}


/**
 * \internal
 * 
 * \brief Perform a CRC-32 calculation on the entire flash memory, on only
 * the application section, on only the boot section, or on a selectable range
 * of the flash memory.
 *
 * This function setups the type of CRC to perform and configures the memory
 * range to perform the CRC on, and starts the CRC via the NVM module.
 * When the calculation is done, the function disables the CRC module and
 * returns the checksum.
 * For CRC on the boot or application section, start address and length are not
 * needed.
 *
 * \note In the flash range mode, an even number of bytes is read. If the user
 * selects a range with an odd number of bytes, an extra byte will be read, and
 * the checksum will not correspond to the selected range.
 *
 * \param crc_type      what kind of CRC to do perform on the nvm module: \ref
 *                      CRC_FLASH_RANGE, \ref CRC_BOOT or \ref CRC_APP
 * \param flash_addr    address of first byte in flash to perform CRC on
 * \param length        number of bytes to perform CRC on
 *
 * \return checksum    CRC-32 checksum
 */
uint32_t crc_flash_checksum(NVM_CMD_t crc_type, flash_addr_t flash_addr, uint32_t length)
{
	if ((length == 0) && (crc_type == CRC_FLASH_RANGE))
		return 0;

	if (! ((crc_type != CRC_FLASH_RANGE) || (crc_type != CRC_BOOT) ||
			(crc_type != CRC_APP)) )
			return 0;

	// Initialize CRC for flash
	crc_reset();
	// Set CRC32 enable bit to ensure correct reading of the resulting checksum
	crc_32_enable();
	crc_set_source(CRC_SOURCE_FLASH_gc);

	if (crc_type == CRC_FLASH_RANGE) {
		nvm_issue_flash_range_crc(flash_addr, flash_addr + length - 1);
	} else {
		nvm_issue_command(crc_type);
	}

	// Disable CRC and return checksum
	return crc_checksum_complete();
}


/**
 * \internal
 * 
 * \brief Issue flash range CRC command
 *
 * This function sets the FLASH range CRC command in the NVM.CMD register. 
 * It then loads the start and end byte address of the part of FLASH to
 * generate a CRC-32 for into the ADDR and DATA registers and finally performs
 * the execute command.
 *
 * \note Should only be called from the CRC module. The function saves and
 *       restores the NVM.CMD register, but if this
 *       function is called from an interrupt, interrupts must be disabled
 *       before this function is called.
 *
 * \param start_addr  end byte address
 * \param end_addr    start byte address
 */
void nvm_issue_flash_range_crc(flash_addr_t start_addr, flash_addr_t end_addr)
{
	uint8_t old_cmd;
	// Save current nvm command
	old_cmd = NVM.CMD;

	// Load the NVM CMD register with the Flash Range CRC command
	NVM.CMD = NVM_CMD_FLASH_RANGE_CRC_gc;

	// Load the start byte address in the NVM Address Register
	NVM.ADDR0 = start_addr & 0xFF;
	NVM.ADDR1 = (start_addr >> 8) & 0xFF;
#if (PROGMEM_SIZE >= 0x10000UL)
	NVM.ADDR2 = (start_addr >> 16) & 0xFF;
#endif

	// Load the end byte address in NVM Data Register
	NVM.DATA0 = end_addr & 0xFF;
	NVM.DATA1 = (end_addr >> 8) & 0xFF;
#if (PROGMEM_SIZE >= 0x10000UL)	
	NVM.DATA2 = (end_addr >> 16) & 0xFF;
#endif	

	// Execute command
	CCP = CCP_IOREG_gc;
	NVM.CTRLA = NVM_CMDEX_bm;

	// Restore command register
	NVM.CMD = old_cmd;
}

#endif //defined(CLASSB_CRC_USE_HW) && defined(CLASSB_CRC_32_BIT)

//@}

