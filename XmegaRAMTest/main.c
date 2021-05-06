/*
 * XmegaRAMTest.c
 *
 * Author : Kuro68k
 */ 

#include <avr/io.h>
#include <stdio.h>

// example main() showing how to read the test result
int main(void)
{
	PORTA.DIR = 0xFF;
	uint8_t stage = GPIO0;
	uint8_t	bitmask = GPIO1;
	uint16_t address = ((uint16_t)GPIO3 << 8) | GPIO2;
	if (stage != 0)
	{
		printf("RAM error, stage %u, mask 0x%02X, address 0x%04X\n", stage, bitmask, address);
		for(;;);
	}
}
