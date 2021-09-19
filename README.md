# Processors
A Miscellany of information on processors - others and ideas for my own:
* Graphviz diagrams
* Documentation
* and maybe some code eventually

for example, until I create a separate file for this...

## Z80 Instruction Set

The Z80 comes out surprisingly well on __code density__ compared with even ARM Thumb: see Weaver & McKee "Code Density Concerns for New Architectures" ICCD'09: 
* Presentation: http://web.eece.maine.edu/~vweaver/papers/iccd09/iccd09.pdf
* Paper: http://web.eece.maine.edu/~vweaver/papers/iccd09/iccd09_density.pdf

On the __instruction set__ see here: https://istvannovak.net/2018/01/25/zx-spectrum-ide-part-3-a-brief-overview-of-the-z80a-cpu/
but the tables below capture regularities for if you wanted to design an instruction decoder. (Although it doesn't include the tables for the EXTD, BITS, IX and IY extensions.

Mnemonics table:
* '.' are 'LD' loads
* Capitals: regular
* Lower-case: irregular
* Brackets: extensions

|    | 0    | 1   | 2   | 3   | 4    | 5    | 6    | 7    | 8   | 9   | A   | B      | C    | D      | E   | F    |
|----|------|-----|-----|-----|------|------|------|------|-----|-----|-----|--------|------|--------|-----|------|
| 0  | nop  | .   | .   | INC | INC  | DEC  | .    | rlca | ex  | ADD | .   | DEC    | INC  | DEC    | .   | rrca |
| 10 | djnz | .   | .   | INC | INC  | DEC  | .    | rla  | jr  | ADD | .   | DEC    | INC  | DEC    | .   | rra  |
| 20 | jr   | .   | .   | INC | INC  | DEC  | .    | daa  | jr  | ADD | .   | DEC    | INC  | DEC    | .   | cpl  |
| 30 | jr   | .   | .   | INC | INC  | DEC  | .    | scf  | jr  | ADD | .   | DEC    | INC  | DEC    | .   | ccf  |
| 40 | .    | .   | .   | .   | .    | .    | .    | .    | .   | .   | .   | .      | .    | .      | .   | .    |
| 50 | .    | .   | .   | .   | .    | .    | .    | .    | .   | .   | .   | .      | .    | .      | .   | .    |
| 60 | .    | .   | .   | .   | .    | .    | .    | .    | .   | .   | .   | .      | .    | .      | .   | .    |
| 70 | .    | .   | .   | .   | .    | .    | halt | .    | .   | .   | .   | .      | .    | .      | .   | .    |
| 80 | ADD  | ADD | ADD | ADD | ADD  | ADD  | ADD  | ADD  | ADC | ADC | ADC | ADC    | ADC  | ADC    | ADC | ADC  |
| 90 | SUB  | SUB | SUB | SUB | SUB  | SUB  | SUB  | SUB  | SBC | SBC | SBC | SBC    | SBC  | SBC    | SBC | SBC  |
| A0 | AND  | AND | AND | AND | AND  | AND  | AND  | AND  | XOR | XOR | XOR | XOR    | XOR  | XOR    | XOR | XOR  |
| B0 | OR   | OR  | OR  | OR  | OR   | OR   | OR   | OR   | CP  | CP  | CP  | CP     | CP   | CP     | CP  | CP   |
| C0 | RET  | POP | JP  | jp  | CALL | PUSH | adc  | RST  | RET | ret | JP  | (BITS) | CALL |        | adc | RST  |
| D0 | RET  | POP | JP  | out | CALL | PUSH | sbc  | RST  | RET | exx | JP  | in     | CALL | (IX)   | sbc | RST  |
| E0 | RET  | POP | JP  | ex  | CALL | PUSH | xor  | RST  | RET | jp  | JP  | ex     | CALL | (EXTD) | xor | RST  |
| F0 | RET  | POP | JP  | di  | CALL | PUSH | cp   | RST  | RET |     | JP  | ei     | CALL | (IY)   | cp  | RST  |



Operands table:
* 'Dest.' column identifies the destination register, for dyadic operations.
* Blank: unrelated to register or implied related to A.

|    | 0 | 1  | 2  | 3  | 4    | 5    | 6    | 7 | 8 | 9  | A  | B  | C | D | E    | F | Dest. |
|----|---|----|----|----|------|------|------|---|---|----|----|----|---|---|------|---|-------|
| 0  |   | BC | BC | BC | B    | B    | B    | A |   | BC | BC | BC | C | C | C    | A |       |
| 10 |   | DE | DE | DE | D    | D    | D    | A |   | DE | DE | DE | E | E | E    | A |       |
| 20 |   | HL | HL | HL | H    | H    | H    | A |   | HL | HL | HL | L | L | L    |   |       |
| 30 |   | SP |    | SP | (HL) | (HL) | (HL) |   |   | SP |    | SP | A | A | A    |   |       |
| 40 | B | C  | D  | E  | H    | L    | (HL) | A | B | C  | D  | E  | H | L | (HL) | A | B     |
| 50 | B | C  | D  | E  | H    | L    | (HL) | A | B | C  | D  | E  | H | L | (HL) | A | D     |
| 60 | B | C  | D  | E  | H    | L    | (HL) | A | B | C  | D  | E  | H | L | (HL) | A | H     |
| 70 | B | C  | D  | E  | H    | L    | (HL) | A | B | C  | D  | E  | H | L | (HL) | A | (HL)  |
| 80 | B | C  | D  | E  | H    | L    | (HL) | A | B | C  | D  | E  | H | L | (HL) | A | A     |
| 90 | B | C  | D  | E  | H    | L    | (HL) | A | B | C  | D  | E  | H | L | (HL) | A |       |
| A0 | B | C  | D  | E  | H    | L    | (HL) | A | B | C  | D  | E  | H | L | (HL) | A |       |
| B0 | B | C  | D  | E  | H    | L    | (HL) | A | B | C  | D  | E  | H | L | (HL) | A |       |
| C0 |   | BC |    |    |      | BC   |      |   |   |    |    |    |   |   |      |   |       |
| D0 |   | DE |    |    |      | DE   |      |   |   |    |    |    |   |   |      |   |       |
| E0 |   | HL |    |    |      | HL   |      |   |   |    |    |    |   |   |      |   |       |
| F0 |   | AF |    |    |      | AF   |      |   |   |    |    |    |   |   |      |   |       |

And this is interesting: http://www.righto.com/2014/10/how-z80s-registers-are-implemented-down.html



