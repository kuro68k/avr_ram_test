
/*
 * ramtest.S
 *
 *  Author: Kuro68k
 *
 * Intermal RAM test for AVR. Uses some XMEGA specific code but can easily be
 * adapted to other AVRs.
 *
 * Used MARCH C- technique, MARCH B code also supplied. Error information is
 * stored in GPIO reigsters. If GPIO is non-zero an error was found.
 *
 * Takes approximately 3.1 million cycles to execute for 8k SRAM. The clock
 * is switched to the 32MHz RC oscillator so execution time is around 100ms.
 * Execution time can be decreased to 1/8th by testing whole bytes instead of
 * single bits.
 *
 */ 

#include <avr/io.h>

failure:
	out		GPIO0, r20	// stage, 0 == no error
	out		GPIO1, r18	// bit
	out		GPIO2, zl	// address
	out		GPIO3, zh	// address
	rjmp	finished

.section .init1,"ax",@progbits
	// increase clock speed to accelerate memory test
	ldi		zl, lo8(OSC_CTRL)
	ldi		zl, hi8(OSC_CTRL)
	ld		r18, Z
	ori		r18, OSC_RC32MEN_bm
	st		Z, r18

	ldi		zl, lo8(OSC_STATUS)
	ldi		zl, hi8(OSC_STATUS)
wait_for_rc32m:
	ld		r18, Z
	sbrs	r18, OSC_RC32MRDY_bp
	rjmp	wait_for_rc32m

	ldi		zl, lo8(CLK_CTRL)
	ldi		zl, hi8(CLK_CTRL)
	ldi		r18, 0x01		// CLK_SCLKSEL_RC32M_gc
	ldi		r19, 0xD8		// CCP_IOREG_gc
	out		CCP, r19
	st		Z, r18

	// MARCH C-, approx 430,000 cycles
	clr		r1
	ldi		r18, 0x01
	ldi		r20, 0
	out		GPIO0, r20	// failure flag

	// up/down w0
	ldi		r20, 1
	ldi		zl, lo8(INTERNAL_SRAM_START)
	ldi		zh, hi8(INTERNAL_SRAM_START)
	ldi		r26, lo8(INTERNAL_SRAM_SIZE)
	ldi		r27, hi8(INTERNAL_SRAM_SIZE)
march_c_minus_stage1:
	st		Z+, r1	// w0
	sbiw	r26, 1
	brne	march_c_minus_stage1

bit_loop:
	out		GPIO1, r18	// indicate which bit failed

	// up r0,w1
	ldi		r20, 2
	ldi		zl, lo8(INTERNAL_SRAM_START)
	ldi		zh, hi8(INTERNAL_SRAM_START)
	ldi		r26, lo8(INTERNAL_SRAM_SIZE)
	ldi		r27, hi8(INTERNAL_SRAM_SIZE)
march_c_minus_stage2:
	ld		r19, Z	// r0
	cpse	r19, r1
	rjmp	failure
	st		Z+, r18	// w1
	sbiw	r26, 1
	brne	march_c_minus_stage2

	// 	up r1,w0
	ldi		r20, 3
	ldi		zl, lo8(INTERNAL_SRAM_START)
	ldi		zh, hi8(INTERNAL_SRAM_START)
	ldi		r26, lo8(INTERNAL_SRAM_SIZE)
	ldi		r27, hi8(INTERNAL_SRAM_SIZE)
march_c_minus_stage3:
	ld		r19, Z	// r1
	cpse	r19, r18
	rjmp	failure
	st		Z+, r1	// w0
	sbiw	r26, 1
	brne	march_c_minus_stage3

	// down r0,w1
	ldi		r20, 4
	ldi		zl, lo8(INTERNAL_SRAM_END)
	ldi		zh, hi8(INTERNAL_SRAM_END)
	ldi		r26, lo8(INTERNAL_SRAM_SIZE)
	ldi		r27, hi8(INTERNAL_SRAM_SIZE)
march_c_minus_stage4:
	ld		r19, Z	// r0
	cpse	r19, r1
	rjmp	failure
	st		Z, r18	// w1
	sbiw	Z, 1
	sbiw	r26, 1
	brne	march_c_minus_stage4

	// down r1,w0
	ldi		r20, 5
	ldi		zl, lo8(INTERNAL_SRAM_END)
	ldi		zh, hi8(INTERNAL_SRAM_END)
	ldi		r26, lo8(INTERNAL_SRAM_SIZE)
	ldi		r27, hi8(INTERNAL_SRAM_SIZE)
march_c_minus_stage5:
	ld		r19, Z	// r1
	cpse	r19, r18
	rjmp	failure
	st		Z, r1	// w0
	sbiw	Z, 1
	sbiw	r26, 1
	brne	march_c_minus_stage5

	// up/down r0
	ldi		r20, 6
	ldi		zl, lo8(INTERNAL_SRAM_START)
	ldi		zh, hi8(INTERNAL_SRAM_START)
	ldi		r26, lo8(INTERNAL_SRAM_SIZE)
	ldi		r27, hi8(INTERNAL_SRAM_SIZE)
march_c_minus_stage6:
	ld		r19, Z	// r0
	cpse	r19, r1
	rjmp	failure
	sbiw	r26, 1
	brne	march_c_minus_stage6

	// next bit
	lsl		r18
	cpse	r18, r1
	rjmp	bit_loop

finished:
	// reset clock speed to 2MHz
	ldi		zl, lo8(CLK_CTRL)
	ldi		zl, hi8(CLK_CTRL)
	ldi		r18, 0x00		// CLK_SCLKSEL_RC2M_gc
	ldi		r19, 0xD8		// CCP_IOREG_gc
	out		CCP, r19
	st		Z, r18


/*
// MARCH B, approx 540,000 cycles
.global ramtest_march_b
ramtest_march_b:

	clr		r1
	ldi		r18, 0xff

	// up/down w0
	ldi		r20, 1
	ldi		zl, lo8(INTERNAL_SRAM_START)
	ldi		zh, hi8(INTERNAL_SRAM_START)
	ldi		r26, lo8(INTERNAL_SRAM_SIZE)
	ldi		r27, hi8(INTERNAL_SRAM_SIZE)
march_b_stage1:
	st		Z+, r1	// w0
	sbiw	r26, 1
	brne	march_b_stage1

	// up r0,w1,r1,w0,r0,w1
	ldi		r20, 2
	ldi		zl, lo8(INTERNAL_SRAM_START)
	ldi		zh, hi8(INTERNAL_SRAM_START)
	ldi		r26, lo8(INTERNAL_SRAM_SIZE)
	ldi		r27, hi8(INTERNAL_SRAM_SIZE)
march_b_stage2:
	ld		r19, Z	// r0
	cpse	r19, r1
	rjmp	failure
	st		Z, r18	// w1
	ld		r19, Z	// r1
	cpse	r19, r18
	rjmp	failure
	st		Z, r1	// w0
	ld		r19, Z	// r0
	cpse	r19, r1
	rjmp	failure
	st		Z+, r18	// w1
	sbiw	r26, 1
	brne	march_b_stage2

	// 	up r1,w0,w1
	ldi		r20, 3
	ldi		zl, lo8(INTERNAL_SRAM_START)
	ldi		zh, hi8(INTERNAL_SRAM_START)
	ldi		r26, lo8(INTERNAL_SRAM_SIZE)
	ldi		r27, hi8(INTERNAL_SRAM_SIZE)
march_b_stage3:
	ld		r19, Z	// r1
	cpse	r19, r18
	rjmp	failure
	st		Z, r18	// w0
	st		Z+, r18	// w1
	sbiw	r26, 1
	brne	march_b_stage3

	// down r1,w0,w1,w0
	ldi		r20, 4
	ldi		zl, lo8(INTERNAL_SRAM_END)
	ldi		zh, hi8(INTERNAL_SRAM_END)
	ldi		r26, lo8(INTERNAL_SRAM_SIZE)
	ldi		r27, hi8(INTERNAL_SRAM_SIZE)
march_b_stage4:
	ld		r19, Z	// r1
	cpse	r19, r18
	rjmp	failure
	st		Z, r1	// w0
	st		Z, r18	// w1
	st		Z, r1	// w0
	sbiw	Z, 1
	sbiw	r26, 1
	brne	march_b_stage4

	// down r0,w1,w0
	ldi		r20, 5
	ldi		zl, lo8(INTERNAL_SRAM_END)
	ldi		zh, hi8(INTERNAL_SRAM_END)
	ldi		r26, lo8(INTERNAL_SRAM_SIZE)
	ldi		r27, hi8(INTERNAL_SRAM_SIZE)
march_b_stage5:
	ld		r19, Z	// r0
	cpse	r19, r1
	rjmp	failure
	st		Z, r18	// w1
	st		Z, r1	// w0
	sbiw	Z, 1
	sbiw	r26, 1
	brne	march_b_stage5

	// up/down r0 (additional blank check)
	ldi		r20, 6
	ldi		zl, lo8(INTERNAL_SRAM_START)
	ldi		zh, hi8(INTERNAL_SRAM_START)
	ldi		r26, lo8(INTERNAL_SRAM_SIZE)
	ldi		r27, hi8(INTERNAL_SRAM_SIZE)
march_b_stage6:
	ld		r19, Z	// r0
	cpse	r19, r1
	rjmp	failure
	sbiw	r26, 1
	brne	march_b_stage6
*/