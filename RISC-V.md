
## RISC-V Smaller ISA

http://www.embedded-cpu-models.org/risc-v_models/rv32ec/
https://www.embecosm.com/2018/05/07/surveying-the-free-and-open-source-risc-v-ecosystem/

'Embedded' instruction set: 16 registers.

https://groups.google.com/a/groups.riscv.org/g/isa-dev/c/j2okI7akT74/m/BQdUwjMRAgAJ
At ASTC (www.astc-design.com), we have an implementation of RV32EC as a synthesizable IP core intended for small embedded applications, such as smart sensors and IoT. Feature set:
* RISCV RV32E base instruction set, compliant with RISCV User-Level ISA Version 2.1.
* RVC standard 16-bit compressed instructions for common RV32 instructions, for reduced code size.
* Optional "M" standard extension for integer multiplication and division instructions.
* Provision for application-specific instruction set extensions, eg for DSP operations.
* Simple machine-mode privileged architecture with direct physical addressing of memory, compliant with RISCV Privileged Architecture Version 1.9.1.
* Machine-mode timer and timer comparison.
* 20 extended interrupts, plus timer and software interrupts.
* All interrupts and exceptions vectored for fast interrupt response.
* Wait-for-interrupt supporting clock gating for low-power idle state.
* 2-stage pipeline comprising fetch and execute stages. Most instructions complete in one clock cycle.
* Tightly-coupled memory interfaces for ASIC ROM and SRAM memories.
* AHB-Lite interface for extended memory and memory-mapped I/O.
* Firmware and virtual prototype development supported by ASTC’s VLAB system-level design tools.

See this thread on this list: 
https://groups.google.com/a/groups.riscv.org/forum/#!topic/isa-dev/iK3enKGb5bw,
and this spreadsheet which contains the complete suggested Xcondensed ISA, opcode formats. With CC licence. 
https://docs.google.com/spreadsheets/d/1rray4sbhGarasDS6acnWyAlOjLvDqBXX3s1LrBLtFs8/edit#gid=1499769591

This is quite interesting. The idea of distinct encodings for the same ISA is an interesting concept. I can imagine some interesting use cases. You might be interested in a tool I have written. We can machine generate disassembler, interpreter and documentation from spreadsheet like input. I am working on an assembler at the moment. With the addition of the assembler we could get the RISC-V gcc or clang backend to use a different assembler and output for the RISC-V ISA binary encoding variant. The tool uses the RISC-V opcodes format and supports 16, 32, 48 and 64-bit instruction sizes.
- https://github.com/michaeljclark/riscv-meta/blob/master/doc/pdf/riscv-instructions.pdf
- https://github.com/michaeljclark/riscv-meta/blob/master/doc/pdf/riscv-types.pdf
- 


## RISC-V Pseudo-Instructions


32 registers - but cannot reduce.
r0 is the 'zero' register.

| Pseudo | Equivalent | Notes |
|------|---------------|--|
| LOAD rx, immediate|  'myriad'! |  |
| MOVE rx, ry | ADD rx, r0, ry |  |
| NOT rx, ry | XOR rx, #-1, ry |  |
| NEG rx, ry | SUB rx, r0, ry |  |
| SEXT.w rx, ry | ADD.w rx, 0, ry | Why? |
| SEQZ rx, ry |  | Set if equal |
| SNEZ rx, ry |  | Set if not equal |
| SLTZ rx, ry |  | Set if <0 |
| SGTZ rx, ry |  | Set if >0; why not use CMP? |
| J offset | JAL x0, offset | ? |
| JAL offset | JAL x1, offset | x1=? |
| JR rx | JALR x0, 0(rx) | jump register |
| JALR rx | JALR x1, 0(rx) | jump and link register |
| RET | JALR r0, 0(rx) |  |
|  |  |  |
|  |  |  |
|  |  |  |





