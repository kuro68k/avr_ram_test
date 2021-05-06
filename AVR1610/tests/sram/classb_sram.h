/* This file has been prepared for Doxygen automatic documentation generation.*/
/**
 * \file 
 *
 * \brief Settings for the SRAM test.
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

#ifndef _SRAM_H_
#define _SRAM_H_


//! \defgroup classb_sram Internal SRAM Test
//!
//! \brief This self-diagnostic test checks the internal SRAM memory for coupling faults. 
//! 
//!  The test \ref classb_sram_test() divides the internal SRAM into \ref CLASSB_NSECS sections that are 
//!  tested in turns with a March algorithm (see \ref marchx). The simplest behavior of the test 
//!  is when there is no overlap between memory sections. In this case all sections have the same 
//!  size, except possibly the last one (see \ref CLASSB_NSECS). The first memory section (referred to as 
//!  the buffer) is reserved: it is used by the test to store the content of the other sections 
//!  while they are being tested (for more details see \ref classb_buffer). 
//! 
//!  If there is overlap (see \ref CLASSB_OVERLAP), every time a memory section is tested a part of the 
//!  previous section is tested as well. Note that this does not apply to the buffer, since it is 
//!  the first section. The size of the buffer is then expanded with respect to the previous case. 
//!  Further, the size of the second section is decreased correspondingly. 
//!  
//!  If there should be an error in internal SRAM the error handler \ref CLASSB_ERROR_HANDLER_SRAM() 
//!  would be called.
//!  
//!  \note Interrupts must be disabled during this test.
//! 
//!  \section marchx March X
//!  
//!  The chosen algorithm is <em>March X</em>. This consists on the following steps: 
//!  \f[ \Updownarrow (w_\textbf{D}); \Uparrow(r_\textbf{D}, w_\bar{\textbf{D}}); \Downarrow (r_\bar{\textbf{D}}, w_\textbf{D}); \Updownarrow (r_\textbf{D}) \;,\f]
//!  where \f$w\f$ denotes a write operation, \f$r\f$ denotes a read operation, 
//!  \f$\textbf{D}\f$ is any data background, \f$\bar{\textbf{D}}\f$ is the complement
//!  of \f$\textbf{D}\f$ and the arrows refer to the addressing order. In our implementation 
//!  we have chosen \f$\textbf{D} = \text{0x00}\f$.
//!
//!  Under the restricted coupling faults (CFs) model, the interleaved organization of the memory 
//!  in XMEGA avoids any kind of intra-word CFs. However, in order to detect some intra-word CFs
//!  that are not considered by this fault model, we have included the following optional march 
//!  element: 
//!  \f[ \Updownarrow (w_\textbf{D_0}, r_\textbf{D_0} \ldots w_\textbf{D_d}, r_\textbf{D_d}) \;,\f]
//!  where the background sequence is \f$\text{{0x00, 0xFF, 0x55, 0xAA, 0x33, 0xCC, 0x0F, 0xF0}}\f$. 
//!  This intra-word test is only executed if \ref CLASSB_SRAM_INTRAWORD_TEST is defined. The elements 
//!  that correspond to the first two data backgrounds are redundant because the first part of the 
//!  test includes them. This optional test will detect all intra-word state CFs considered by the 
//!  unrestricted CF model.
//!  
//!  
//@{

//! \name Configuration settings
//@{

//! \brief Number of sections the SRAM is divided into for testing.
//!
//! It is advisable that \c INTERNAL_SRAM_SIZE is divisible by the number of 
//! sections and, therefore, recommended values are 2, 4, 8, 16, etc. Otherwise an extra 
//! section will be added with the remainder of the division as size. Note that the higher 
//! the number of sections the smaller the size of \ref classb_buffer, i.e. the section of memory 
//! that is reserved for the test) and the faster each partial test is completed. 
#define CLASSB_NSECS 8

/** 
 * \brief Overlap between memory sections (in % of \c CLASSB_SEC_SIZE). 
 * 
 * This should be an unsigned long, i.e. \<val\>UL. 
 */
#define CLASSB_OVERLAP 25UL

#ifdef __DOXYGEN__
 //! \brief If defined an intra-word test will be added after the inter-word test. 
 //! 
 //! Given the layout of the memory, the probability of intra-word coupling faults 
 //! is greatly diminished. However, for extra safety the test can be expanded to
 //! check some intra word coupling faults.
 #define CLASSB_SRAM_INTRAWORD_TEST
#else
// #define CLASSB_SRAM_INTRAWORD_TEST
#endif
//@}


//! \internal
//! \name Constants that are automatically computed.
//@{ 
//!\internal The size of each segment in bytes 
#define CLASSB_SEC_SIZE (INTERNAL_SRAM_SIZE / CLASSB_NSECS)

//! \internal  The size of the last segment in bytes (when \c INTERNAL_SRAM_SIZE is not divisible by \ref CLASSB_NSECS) 
#define CLASSB_SEC_REM (INTERNAL_SRAM_SIZE % CLASSB_NSECS)

//! \internal  Size of overlap in bytes. When testing a memory section, the algorithm starts \c CLASSB_OVERLAP_SIZE 
//! bytes behind the start of the section.
#define CLASSB_OVERLAP_SIZE (CLASSB_SEC_SIZE*CLASSB_OVERLAP)/100

//! \internal  Total number of segments, including remainder if present.
#if (CLASSB_SEC_REM == 0)
#  define CLASSB_NSEC_TOTAL CLASSB_NSECS
#else
#  define CLASSB_NSEC_TOTAL CLASSB_NSECS + 1
#endif
//@}

//! \name Class B Test
//@{
void classb_sram_test( void );
//@}

//! \internal\name March X Algorithm
//@{
void classb_marchX(register volatile uint8_t * p_sram, register volatile uint8_t * p_buffer, register uint16_t size);
//@}
 
 
//@}
#endif