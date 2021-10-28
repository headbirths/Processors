
** System **

Covers:
* ROM/RAM interfacing 
* Control/status registers
* The external interface port (which also serves as the debug port).

** External SPI EEPROM **

Example low cost (cheaper than Micron):
* https://www.macronix.com/Lists/Datasheet/Attachments/7478/MX25R2035F,%20Wide%20Range,%202Mb,%20v1.6.pdf

4-wire SPI rather than onewire: many pins, but cheapest silicon.

EEPROM could be co-packaged.


** CSR: Control/status registers **

Latch implementation. Flood-fill reset.


** ROM/RAM **

Expected to be asynchronous memories, for maximum speed and simplicity.

Can build RAM from latches (see CSR) - same low voltage as rest of system. [Does this make sense?]



** Interface Port **

4-wire SPI: many pins, but:
* cheapest SPI EEPROMS
* unidirectional signals, mean they can be fast.

Requirements:
* Be controllable over SPI from a microcontroller.
* Able to read/write through to control/status registers directly (as if a CPU was not present).
* Able to connect a SPI EEPROM in order for the CPU to (i) execute code, (ii) read trim/config parameters.
* Probe down onto SPI pins to externally program the EEPROM (or have tri-stated FPGA).
* Run the interface as fast as possible.

Typically, we would expect the interface to be a SPI slave, not a master. But it needs to be a master for driving SPI EEPROM.

Do we accommodate dual mode, or handle the microcontroller as a slave? Try the latter...

Pins:
* SCK: clock, mastered by XaVIsys.
* MOSI: data mastered by XaVIsys.
* SCK: data from slave
* CSN/HOLD: select, controlled by XaVIsys. Open-collector output, sensed.

External device can signal readiness by asserting CSN.

From power-up, XaVIsys assumes SPI EEPROM and issues reads. Possibly: reads first n locations to fill up n-word cache then executes code from there on.

If there is a HOLD from power up (i.e. CSN pin is low even though XaVIsys is not pulling it low) then it is assumed that there is a MCU interfacing.

Under <some circumstance>, SCK and MOSI will be tri-stated to allow access from MCU/FPGA/probe to the EEPROM.

We generally want push-pull rather than open-collector pins in order to be fast.

Communication to MCU:
* MCU generally asserts HOLD. Releases to signal 'OK to poll me'.
* XaVIsys will poll 'what do you want from me?'
* Default MCU response: 'nothing' (NOP).
* XaVIsys can grab a whole stream of instructions, with NOPs interspersed as required.

Read/write CSR example:
* MCU sets up request information: (14-bit address + read/write flag)
* MCU releases HOLD
  
  
Negotiating bus frequency: MCU can program internal register.


  
** Debug **
  
* XaVIsys has a CPU HALT flag (writable from MCU).
* XVI has a read-only input register Rm.
* There is a comparator detecting Rx=Rm. Rx can be set to be R8 in operational mode or R0 (PC) in debug mode.
* Rm will be a CSR (or: what could be done with this in operational mode? - general approach - make whatever hardware is laid down as useful as possible)
* Comparator will trip (set) the HALT flag.
* When the CPU is halted, the Interface takes over, sending a 'what do you want from me?' transcation - indicating that the CPU has hit the breakpoint.
* There is not 'step' operation: just set the
* Compromise for debug: not able to us the Match feature for code being debugged.


** Next Steps **
  
Need to investigate capabilities of various MCUs - for being a slave; how fast etc.
  
  
  

