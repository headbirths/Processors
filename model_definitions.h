//////////////////////////////////////

// (c) Neil Howard 2021-

// Common definitions
// !!! Functions should be put elsewhere
/////////////////////////////////////

///////////////////////////////////////////////////////////
// DEFINITIONS
///////////////////////////////////////////////////////////

// !!! Does this need defining?
/// #define UINT    unsigned int  
#define FALSE      0
#define TRUE       1
#define QUIET      0
#define VERBOSE    1
#define LOGGING    2
#define INVALID    -999



///////////////////////////////////////////////////////////
// Registers
///////////////////////////////////////////////////////////

// Program Count, Status Flags, Stack Pointer, Zero Register
#define PC    0
#define SF    1
#define SP    2
#define ZR    3
// And the general-purpose registers R4...R15 (as in MSP430)
// And the extra register R16==XR (extra, clobberable)
#define XR    16
// !!! Renumber these? ZR=0; PC=16; XR=3


///////////////////////////////////////////////////////////
// Opcodes
///////////////////////////////////////////////////////////

// Opcodes are 2, 3 or 4 characters
// Dyadic...
#define MOVE   4
#define ADD    5
#define ADDC   6
#define SUBC   7
#define SUB    8
#define CMP    9
#define DADD   10
#define BIT    11
#define ANDN   12
#define CLR    12
#define OR     13
#define SET    13
#define XOR    14
#define AND    15


// Conditional branch relative...
#define BRNZ    0
#define BRZ     1 
#define BRNC    2
#define BRC     3
#define BRN     4
#define BRGE    5
#define BRL     6
#define BRA     7

// Unary...
#define RORC    0
#define SWPB    1 // Is this just rotate by 8?
#define RORA    2 // What about shifts/rotates left
#define SXT     3
// !!! Will a repeat instruction be added?
// !!! Will a barrel shifter be present?

// !!! How does a LINK register mechanism work?

// !!!Missing (not atomic: JSR, RET, RETI, PUSH, POP
// Need to define how these are realised - in msp_execute()
// Including atomicity.
// Common function to:
// 1. Decode
// 2. Dissassemble
// 3. Execute (translate to XaVI). !!! (XaVI *expanded* format?)

