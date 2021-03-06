
# XaVI: A 16-bit Processor for Analog ICs


## Not Another One!

Why do we need _another_ processor?
* **For Analog/Mixed-Signal**: Many analog/mixed-signal chip providers have developed their own proprietary little processor for embedding into their chips. But there is no _standard_, _open_ 16-bit processor: the ARM or MIPS of the mixed-signal world. It needs to be small. Not so (8-bit) small that its code density isn't great but not so (32-bit) large: 16-bit number are _juuuust_ right - just a bit bigger than the ENOB of an ADC or DAC. Digi-people say an ARM Cortex-M is small, and that is true in 22nm. But you don't want non-analog factors determining what process node you use for your analog chips.
* **Open**: _Truly_ open source: without any commercial restrictions. No Copyleft as well as no copyright. Merely the acknowledgement that there if a processor be declared, that it be declared as a XaVI, to propote it as open sourceware. If the processor is _so_ embedded that its very existence is not known to outside parties, then it does not need to be declared at all. [There are some conditions though: more later.]
* **Low Power**: seriously low. Low area = low leakage, making sub-threshold operation (e.g. dynamic voltage scaling down to 0.6V) more feasible. And high code density - tweaked for the target application. High performance: maximal MIPS per second, without actually having a high MIPS target. HUGI is the objective; hurry up, to gate off the power. There will be some particular characteristics that will make XaVI particularly good for ultra-low power.
* **Compiler**: It needs to have a _proven_ C compiler. More precisely, there needs to be a way to write C and get machine code out of it - a subtle difference. Compilers cost money to develop - more money than I earn in a year. Much more. I'm not that altruistic. Some altruistic compiler developer can donate their time to the open-source cause later on (pleeeease). Instead, there needs to be a way of using a free compiler and translating the assembly code from one ISA to another. In this way there are no legal imfringements. Assembly code can be hand-optimized in the target, but having a high-level language accelerates development.

Enter XaVI: XVI for 16 and 'a' for analog.

## A Small Processor Subsystem ##

See CPU_bus_and_mem.gv [filename correct?]

**Harvard or von Neumann?**: We're looking at designing a processor that in many ways wouldn't have looked out of place in the 1980s. The difference now though is this is _IP_ (Semiconductor Intellectual Property) that is _embedded_ into chip. We're not limited by trying to sequence the I/O into a 40-pin DIL package. We are obviously going to choose Harvard, to get simultaneous fetch/decode and execution, but there will probably be unified memory (we can't afford he overhead or lack of flexibility to trade off program and data space). The `pCache` in the diagram below doesn't have to be a cache, it could be a demux to match `dDemux`. But it could be a mini-cache: perhaps 16 words worth for tight loops. In the smallest of systems, `RAM` will be so small that there is no benefit from a cache. Any memory beyond that might be far, far away - in a host processor system, or off-chip serial flash.

**A tightly-coupled co-processor** easily be accommodated if so desired, for application-specific acceleration. There won't be a hardware multiplier within the processor core (yes! I said _small_). But this means that if you want that multiplier there for something specific, you can put more there too. For example, you could put down 4 multiply-accumulates down, configurable for biquad or FIR filters, which can _also_ do a standalone multiply or MAC.

Ultimately, the control/status registers `CSR` are what it is all about. The whole system here is just about providing a _programmable_ state machine to twiddle, sense and shovel small-width words to/from the analog.

Some minimal ROM is likely, for booting, unless this can be achieved via 'far memory' (no _control_ is shown here).


**No pipelining**: K.I.S.S.
* `Fetch` with provide instructions to `Scheduler` with an `ack` back. When `Datapath` does anything that might affect the `PC` (program counter), `Fetch` will be stalled.
* We (as hardware designers) don't worry about memory coherency: just don't write data to program space - except when loading `RAM` (initially or when paging; yes: paging!). We leave all that for software designers to worry about.


## A CPU Architecture Framework ##

A `Datapath` block contains the fundamental processing resources of the CPU: its registers, ALUs and various muxes to connect everything together to form different data paths. All the mux controls for this can be lumped together to form a VLIW instruction word. And we could use that as the CPU instruction. But that would have horrendously low code density. The purpose of the ISA is to compress the VLIW - into what I call 'Huffman instructions' since, it is essentially Huffman coding that is used for that compression: the most likely VLIW instructions are encoded with the shortest number of Huffman instructions. The compression is application-specific.

Program memory width though is fixed - at 16 bits; we are not going to pack 8-bit instructions into that. We are talking about unpacking 1-word or 2-word Huffman instructions into 1- or more VLIW instructions. The `Scheduler` breaks a Huffman instruction into multiple VLIW instructions and the `Decode` decompresses from Huffman to VLIW. The `Scheduler` allows the CPU to deal with commonly-occurring actions that call upon more resources than the `Datapath` has, so they must be time-multiplexed. It handles the fact that these VLIW instructions must be atomic: non-interruptable. For example: `RETI` (return from interrupt) will be implemented as `POP PC`, `POP FLAGS`. And the `Scheduler` also handles interrupts, inserting instructions: `PUSH FLAGS`, `PUSH PC`, `MOVE #0000, PC`.

An ARM naming convention is adopted for the program and data busses but there are no select/enable signals. Addresses are in bytes and `DWRITE` isa  2-bit vector.

See CPU_fetch_decode_datapath.gv [filename correct?]


**RISC or CISC?**: In practice, no practical processors are pure RISC or pure CISC. Outwardly, XaVI is RISC: the `Datapath`, with VLIW instructions, is RISC. But the Huffman coding can make it more CISC-like.  

**The Datapath**

See CPU_datapath.gv [filename correct?]


**The Register Set**:

The framework provides for 32 registers. Included within these are:
* `PC`: Program Count
* `FLAGS`: including `Z`, `I`, `N`, `C` and `V` flags for zero, interrupt-enable, negative, carry and overflow. Pretty normal really.
* `SP` stack pointer. Any normal register can be used as the stack pointer: this is just an alias. The actual register is chosen by the compiler.
* `ZERO`: only ever returns zero. For purpose, see later.

Optionally, there are:
* Write-only control/data registers - for a `CoPro` or just a more tightly-coupled `CSR` control register.
* Read-only status/data registers - for a `CoPro` or just a more tightly-coupled `CSR` status register.
* Read/write - for the `CoPro`.

**Addressing Modes**:

There are 3 fundamental instruction types supported by `Datapath`.

| Type | Configuration | Addressing mode |
|------|---------------|-----------------|
|A| OP nnnn+Rx*, Ry, Rz||
|B| OP (nnnn+Rx*), Ry, Rz||
|C| OP Ry, (nnnn+Rx*), (nnnn+Rx*)||

where:
* OP is the operation e.g. `MOVE` or `ADD`.
* The first and second operands are fed to 'dyadic' ALUs and the result is sent to the third operand.
* If only 2 operands are given, the first is the source, fed to a 'unary' ALU and the result is sent to the second operand.
* The asterisk indicates optional pre-decrement or post-increment.

This structure subsumes the following 'normal' addressing modes (by setting nnnn to zero and using the `ZERO` register):

| Type | Configuration | Addressing mode |
|------|---------------|-----------------|
|A| OP Rx*, Ry, Rz| Register|
|A| OP nnnn, Ry, Rz| Immediate|
|B| OP (nnnn), Ry, Rz| Absolute|
|B| OP (Rx*), Ry, Rz| Indexed |
|B| OP (nnnn+Rx*), Ry, Rz| Indexed-offset |
|C| OP Ry, (Rx*), (Rx*)|Indirect source/destination|
|C| OP Ry, (nnnn+Rx*), (nnnn+Rx*)|Indexed-offset source/destination|

`OP nnnn+Rx* ...` is not a normal addressing mode, but it is _potentially_ available because it just falls out of the architecture. WHat else just falls out? How can the maximum useful work be done from the `Datapath` hardware laid down? The addition of the path from `ADAD` to `PC`, using `PC` or `ZERO` as the selected source register, allows some operations to also include a relative or absolute jump. For example, `AND (R1*), R2, R3 & JRNZ .again` could be a single instruction.

The 16-bit number nnnn may originate from a second instruction word or may be decompressed from the instruction using the `CGEN` constant generator.


## Surely You're Joking? ##

You howl in protest: "You said you're going for a tiny processor but you have specified a 3-operand RISC processor with 32 registers!" That's bigger than a full ARM architecture when what you said you want is teenier than an ARM Thumb" (Thumb being 2-operand, with 16 registers.)

Yes, but this is the _framework_:
* This is the VLIW, not the Huffman instruction: The `Scheduler` and `Decode` blocks define the ISA. Whatever simplifications are in here will just propogate through to the ripping out logic in the `Datapath` _during logic synthesis_ of the Verilog code.
* We want to build _small_ processors but we want to know we can expand and be compatible with larger instructions.


## Licensing ##

Although open-source, a user is not completely free of responsibilities:
1. The Verilog code for the `Datapath.v` must be provided back to the XaVI foundation.
2. The Verilog code for the overall `XaVI.v` top-level must be provided back to the XaVI foundation.
3. Acknowledgement that a XaVI CPU is being used, in documentation of the IC/FPGA design.
4. Failure to comply incurs a fine of $20000, split equally between two UK charities: Epilepsy UK and Epilepsy Action.
A prior total donation of $5000, notified by the charities to the XaVI Foundation, exempts the licencee from a fine for violation of clause 3. 
A prior total donation of $5000 to the charities exempts the licencee from a fine for violation of clauses 1 or 2. Donations are kept confidential if desired. 
The low 'cost' is set to encourage donations to be made.

If you are using the CPU in a way that is not in any way apparent to the user of the IC/FPGA (excluding production test), it is easy to avoid both a donation and a fine. We just lean on your conscience here: it is for a good cause.

Clauses 1 and 2 allow the XaVI Foundation to make improvements to the architecture. Changes to the internals of the `Fetch`, `Schedule` and `Decode` blocks do not need to be disclosed. Clause 3 promotes the XaVI CPU to a wider audience to help create an eco-system around it.


## To Do: ##

* PUSH/POP: is the wrong register X/Y being inc/decremented? List compound instructions.
* ALUs: split into ALU2 and LU1. No barrel shifter. Plug-in ALUs. Long multiplication with compound instructions: 8|16|32, signed|unsigned.
* Long combinatorial paths. Intel/Motorola. Capture (exception: integral conditional jump).
* Debug interface. SPI. Digital Twin. PIC / programmable state machine. Fast: GHz.External SPI - choice of process.



## Abbreviations ##

| Abbreviation | Description |
|--------------|-------------|
|VLIW|Very Long Instruction Word|



## MSP430 Addressing modes ##

The MSP430 instruction set is very regular ('highly orthogonal'), dominated by 3 classes of instructions: unary, dyadic and branches.

Key:
- `ABC` is the opcode: _but note_ these are the _XaVI_ equivalent opcodes!
- `#` is the number of 16-bit words


### Unary operations ###

| Opcode   |
|-----|
| RRC |
| SWP |
| RR  |
| SXT |
| PSH |
| JSR |
| RTI |

| Ad1 |   | # | Form        |
|-----|---|---|-------------|
| 0   | 0 | 1 | ABC Rx      |
| 0   | 1 | 2 | ABC @(n+Rx) |
| 1   | 0 | 1 | ABC @Rx     |
| 1   | 1 | 1 | ABC @Rx+    |

### Dyadic operations ###

| Opcode  |
|---------|
| MOV     |
| ADD     |
| ADC     |
| SBC     |
| SUB     |
| CMP     |
| BIT     |
| ANN/CLR |
| OR/SET  |
| XOR     |
| AND     |

| Ad2 |   |   | # | Form                  |
|-----|---|---|---|-----------------------|
| 0   | 0 | 0 | 1 | ABC Rx , Ry           |
| 0   | 0 | 1 | 2 | ABC Rx , @(n+Ry)      |
| 0   | 1 | 0 | 1 | ABC Rx , @Ry          |
| 0   | 1 | 1 | 1 | ABC Rx , @Ry+         |
| 1   | 0 | 0 | 2 | ABC @(n+Rx) , Ry      |
| 1   | 0 | 1 | 3 | ABC @(n+Rx) , @(n+Ry) |
| 1   | 1 | 0 | 2 | ABC @(n+Rx) , @Ry     |
| 1   | 1 | 1 | 2 | ABC @(n+Rx) , @Ry+    |

### Branches ###

| Opcode |
|-----|
| BNZ |
| BRZ |
| BNC |
| BRC |
| BRN |
| BRP |
| BRL |
| BRA |


## Advance Information for Compiler Developers ##

XaVI is an Open Source (ISA and Verilog RTL) 16-bit CPU targetted for ultra-low power applications in analog/mixed-signal ICs (XVI for 16 and 'a' for analog). Functionally, it is comparable to the Texas Instruments MSP430 CPU which has code density and power consumption that is class-leading among 8- and 16-bit CPUs. Over 40 years after the introduction of 8-bit microprocessors, the only truly Open Source *tiny* CPU is the Intel 8051 which has significantly worse power consumption and code density than the MSP430. 

XaVI is architected to be able to exploit some low-power techniques that, it is hoped, can reduce area, dynamic power and leakage by about 30%. This will be beneficial when applying XaVI to MSP430 microcontroller type applications. But it is hoped these benefits will help push XaVI into places previously considered too small for processors to exist; for example, as 'programmable state machines' in analog ICs. Top-end implementations might have 128Kbyte program and 128Kbyte data memory. But at the bottom end, it may have only 64 words of (writable) program memory and absolutely no data memory (reliant solely on its internal registers). 

Note: there is the trade-off between minimizing dynamic (switching, CV^2) power and static (leakage) power. The target is for a balanced reduction, approprate for 'always-on' control hardware.

It is possible to optimize CPU power consumption at many different levels, e.g.:
1. Compiler usage (e.g. https://research-information.bris.ac.uk/en/studentTheses/inductive-logic-programming-for-compiler-tuning)
2. Compiler/hardware optimization (e.g. https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-129.pdf).
3. Hardware architecture.
4. Gate-level (e.g. use of HVT cells to minimize leakage).
5. Transistor-level (e.g. back-biasing).

My interest is in minimization of area and power through level 3 'Hardware architecture' techniques. The target reductions by approx 30% are compared with conventional design implementations that already employ state-of-the-art low power techniques (e.g. clock and operand gating, adaptive/dynamic voltage/frequency scaling).

With such a focus on low power, the architecture might hopefully be good candidate for other techniques, particularly at level 2. For example: XaVI 'external' instructions are 16 bits long. '0xFxxx' instructions specify a 12-bit prefix to the following instruction. Thus, internal 'uncompressed' instructions are 28 bits long (not the same as internal 'decoded instructions' that control the datapath and are longer still). The Huffman coding of 28-bit data into 16-bit words could be application-specific, based on analysis of example code for the target application.

The Open Source XaVI CPU hardware is a by-product of the investigation into low-power techniques. For the purposes of quantifying the area and power benefits, a compiler is not actually necessary. But for the purposes of having an Open Source 'tiny' CPU, it is!

One possibility to provide a route from C without the significant costs of compiler development is to compile C code to MSP430 and then translate assembly code across to XaVI. Obviously this would sacrifice code density and power savings.

In February 2022, there will be:
* A functional C model of XaVI (i.e. not cycle-accurate), for the purposes of developing a C compiler.
* Documentation for mapping from MSP430 assembler to 28-bit XaVI instructions, for the purposes of developing an assembly code translator.

It is thought that the development of a C compiler for XaVI might be a suitable student project (based on LLVM would be easiest). An assembly code translator (and assembler/linker) would be an easier project.


