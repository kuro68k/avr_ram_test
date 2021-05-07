# avr_ram_test
Test MCU internal RAM at startup

MARCH C- intermal SRAM test for AVR. The sample implementation is for XMEGA but easily converted to other devices. The only XMEGA specific parts are the error handling and some code to set the clock speed to 32MHz to reduce the test time.

The code runs in .init1, before main() and before GCC init code has copied any data into RAM, so it can do a full test. Test results are stored in XMEGA GPIO registers, if porting to other devices you will need to stash them somewhere else or handle the failure some other way.

The test takes around 3.1 million cycles.

Licence is GPL v3.
