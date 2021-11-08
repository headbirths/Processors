//////////////////////////////////////

// (c) Neil Howard 2021-

// Model of MSP430:
// Comprises:
// 1. msp430_reset()          Initialisation
// 2. msp430_decode()         Instruction decoder
// 3. msp430_disassemble()    Disassembler
// 4. msp430_execute()        Execution: makes XaVI execution calls
/////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "definitions.h"


//////////////////////////////////////
// Static variables: MSP430 decoded instruction
//////////////////////////////////////

UINT	msp430_idec_length      ; // of instruction, in words - not actually used by execute() function
UINT	msp430_idec_class       ;
UINT	msp430_idec_opcode      ;
UINT	msp430_idec_source      ; // Register select. 17 means the ZERO register; 16 means that a constant is selected instead; 
UINT	msp430_idec_addrmode    ;
UINT	msp430_idec_byte        ;
UINT	msp430_idec_dest        ; // Register select.
UINT	msp430_idec_constant    ;
UINT	msp430_idec_destoffset  ;


//////////////////////////////////////
// msp430_decode
/////////////////////////////////////

// Called from msp430_decode. Common to both unary and dyadic instructions
void msp430_source_decode(){
	switch(msp430_idec_addrmode){ // Determine source
		case 0, 4 : // As=0 register direct
			if      (msp430_idec_source ==  3){ msp430_idec_constant=0; msp430_idec_source=16; } // CG2
			break;
		case 1, 5 : // As=1; indexed
			if      (msp430_idec_source == SP){ msp430_idec_constant=word2; msp430_idec_source=17; msp430_idec_length++; } // Absolute
			else if (msp430_idec_source ==  3){ msp430_idec_constant=1; msp430_idec_source=16; } // CG2
			break;
		case 2, 6 : // As=2; register direct
			if      (msp430_idec_source == SP){ msp430_idec_constant=4; msp430_idec_source=16; } // CG1
			else if (msp430_idec_source ==  3){ msp430_idec_constant=2; msp430_idec_source=16; } // CG2
			else { msp430_idec_length++; msp430_idec_constant=word2; } // This serves as the source offset address
			break;
		case 3, 7 : // As=3; register direct auto post-increment
			if      (msp430_idec_source == PC){ msp430_idec_constant=word2;  msp430_idec_source=16; msp430_idec_length++; } // Immediate
			else if (msp430_idec_source == SP){ msp430_idec_constant=8; msp430_idec_source=16; } // CG1
			else if (msp430_idec_source ==  3){ msp430_idec_constant=-1; msp430_idec_source=16; } // CG2
			else { msp430_idec_length++; msp430_idec_constant=word2; } // This serves as the source offset address
			msp430_idec_autoincr = TRUE; 
			break;
	}
}

void msp430_decode(word1,word2, word3){
	// Default invalid:
	msp430_idec_class       = INVALID;
	msp430_idec_opcode      = INVALID;
	msp430_idec_source      = INVALID;
	msp430_idec_addrmode    = INVALID;
	msp430_idec_byte        = INVALID;
	msp430_idec_dest        = INVALID;
	msp430_idec_constant    = INVALID;
	msp430_idec_destoffset  = INVALID;
	msp430_idec_autoincr    = FALSE;
	msp430_idec_length      = 1; // default

    if (Bits(word1, 15, 12)>=4)      { // Type 2: Dyadic operations
		msp430_idec_class       = DYADIC_CLASS;
		msp430_idec_opcode      = Bits(word1, 15, 12);	
		msp430_idec_source      = Bits(word1, 11,  8);	
		msp430_idec_addrmode    = Bits(word1,  7,  7)*4 + Bits(word1,  5,  4);	
		msp430_idec_byte        = Bits(word1,  6,  6);	
		msp430_idec_dest        = Bits(word1,  3,  0);	

		switch(msp430_idec_opcode){
			case  10 : return 0; break; // No opcode here (DADD not supported)
		}

		msp430_source_decode();
		switch(msp430_idec_addrmode){ // Determine destination (using msp430_idec_dest)
			case 4, 5, 6, 7 : // Ad=1 register indirect
				if      (msp430_idec_length == 1){ msp430_idec_destoffset=word2; }
				else if (msp430_idec_length == 2){ msp430_idec_destoffset=word3; }
				msp430_idec_length++;
				break;
			case 0, 1, 2, 3 : // Ad=0 register direct
				break;
		}
	}
	else if (Bits(word1, 15, 12)>=2) { // Type 0: Jumps
		msp430_idec_class       = JUMP_CLASS;
		msp430_idec_opcode      = Bits(word1, 12, 10);
		msp430_idec_constant    = Sign_Extend(Bits(word1, 9, 0), 10); // !!! 9 or 10 as the length?
	}
	else if (Bits(word1, 15, 10)==0x04) { // Type 1: Unary operations
		msp430_idec_class       = UNARY_CLASS;
		msp430_idec_opcode      = Bits(word1,  9,  7);
		msp430_idec_byte        = Bits(word1,  6,  6);	
		msp430_idec_addrmode    = Bits(word1,  5,  4);	
		msp430_idec_source      = Bits(word1,  3,  0);	// source is also the destination

		msp430_source_decode();
		switch(msp430_idec_opcode){
			case  1, 3, 5 : // SWPB, SXT, CALL
				if (msp430_idec_byte){return 0}; break; // Byte not allowed here
			case  6 :
				if (msp430_idec_byte) {return 0}; // RETI: Byte not allowed here
				if (msp430_idec_source != 0) {return 0}; // RETI: N/A
				if (msp430_idec_addrmode != 0) {return 0};
				break;
			case  7 : return 0; break; // No opcode here
		}
	}
	else {
		return 0 // Invalid: no instruction words consumed
	}
	return (length);
}


//////////////////////////////////////
// msp430_disassemble
/////////////////////////////////////

// !!! Could be prettified e.g. printing 'PC' instead of 'R0'

void msp430_disassemble(addr){
	// Uses msp430_idec_* static variables.
	// Assumes that the decoded instruction is valid.

	printf("0x%4h : ", addr);
	
	char size; // indicate B or W
	
	if (msp430_idec_byte) { size='B'; } else { size='W'; }
	if (msp430_idec_class == DYADIC_CLASS){
		switch(msp430_idec_opcode){
			case  4: printf("MOV.%c  ", size); break;
			case  5: printf("ADD.%c  ", size); break;
			case  6: printf("ADDC.%c ", size); break;
			case  7: printf("SUBC.%c ", size); break;
			case  8: printf("SUB.%c  ", size); break;
			case  9: printf("CMP.%c  ", size); break;
			case 10: printf("DADD.%c ", size); break;
			case 11: printf("BIT.%c  ", size); break;
			case 12: printf("BIC.%c  ", size); break;
			case 13: printf("BIS.%c  ", size); break;
			case 14: printf("XOR.%c  ", size); break;
			case 15: printf("AND.%c  ", size); break;
		}		
		switch(msp430_idec_addrmode){ // Report source
			case 0, 4 : // As=0 register direct
				if      (msp430_idec_source ==  3){ printf("#%d, ", msp430_idec_constant) } // CG2
				else { printf("R%d, ", msp430_idec_source) }
				break;
			case 1, 5 : // As=1; indexed
				if (msp430_idec_source == SP){ printf("&0x%4h,  ", msp430_idec_constant) } // Absolute
				else                  { printf("%d(R%d), ", msp430_idec_constant, msp430_idec_source) }
				break;
			case 2, 6 : // As=2; register indirect
				if (msp430_idec_constant != INVALID){ printf("#%d,  ", msp430_idec_constant); }
				else                         { printf("(R%d),  ", msp430_idec_source); }
				break;
			case 3, 7 : // As=3; register indirect auto post-increment
				if (msp430_idec_constant != INVALID){ printf("#%d,  ", msp430_idec_constant); }
				else                         { printf("(R%d+), ", msp430_idec_source); }
				break;
		}
		switch(msp430_idec_addrmode){ // Report destination
			case 0, 1, 2, 3 : // Ad=0 register direct
				printf("R%d, ", msp430_idec_dest) }
				break;
			case 4, 5, 6, 7 : // As=1; absolute
				printf("0x%4h, ", msp430_idec_constant) } // !!! Doesn't handle if word2 or word3
				break;
		}
	}
	if (msp430_idec_class == JUMP_CLASS){
		switch(msp430_idec_opcode){
			case  0: printf("JNZ %d", msp430_idec_constant); break;
			case  1: printf("JZ  %d", msp430_idec_constant); break;
			case  2: printf("JNC %d", msp430_idec_constant); break;
			case  3: printf("JC  %d", msp430_idec_constant); break;
			case  4: printf("JN  %d", msp430_idec_constant); break;
			case  5: printf("JGE %d", msp430_idec_constant); break;
			case  6: printf("JL  %d", msp430_idec_constant); break;
			case  7: printf("JMP %d", msp430_idec_constant); break;
	}
	if (msp430_idec_class == UNARY_CLASS){
		switch(msp430_idec_opcode){
			case  0: printf("RRC.%c ", size); break;
			case  1: printf("SWPB  ",  size); break;
			case  2: printf("RRA.%c ", size); break;
			case  3: printf("SXT   ",  size); break;
			case  4: printf("PUSH  ",  size); break;
			case  5: printf("CALL  ",  size); break;
			case  6: printf("RETI"); break;
	}

	printf("\n");
}

//////////////////////////////////////
// msp430_reset
/////////////////////////////////////

// !!! Assumes static 'pc' used as program counter

void msp430_reset(){
	pc = 0;
	xavi_reset();
}

//////////////////////////////////////
// msp430_fetch
/////////////////////////////////////

// !!! Assumes static 'pc' used as program counter
// !!! Assumes static 'mem[65536]' used as instruction memory

// !!! Doesn't handle strange cases of if indexing around PC for example (consequence of 'immediate mode' being '0(PC)+'; von Neumann)
void msp430_fetch(){
	UINT length; // no. of instruction words used
	length = msp430_decode( mem[pc], mem[pc+1], mem[pc+2]);
	if (length==0){ printf("Invalid Instruction 0x%4h at 0x%4h\n", mem[pc], pc); msp430_reset(); }
	else { msp430_disassemble(pc); pc += length }
}

//////////////////////////////////////
// msp430_execute
/////////////////////////////////////

// !!! Not yet: will just make calls to xavi_execute()
// !!! Decode should flag things that XaVI will struggle with e.g. using '(PC+)'

