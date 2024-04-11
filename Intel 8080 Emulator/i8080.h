#pragma once
#include <inttypes.h>

typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef BYTE(*INSTRUCTION)(BYTE* code);

const static INSTRUCTION OPCODE_TABLE[256];

// REGISTER BYTE INSTRUCTIONS
typedef enum _REG {
	REG_B = 0, // 000b for Register B
	REG_C,	   // 001b for register C
	REG_D,	   // 010b for register D
	REG_E,	   // 011b for register E
	REG_H,	   // 100b for register H
	REG_L,	   // 101b for register L
	REG_M,	   // 110b for memory ref. M
	REG_A	   // 111b for register A
} REG;

// REGISTER PAIR INSTRUCTIONS
typedef enum _REG_PAIR {
	REG_PAIR_BC = 0, // 00b for registers B and C
	REG_PAIR_DE,     // 01b for registers D and E
	REG_PAIR_HL,     // 10b for registers H and L
	REG_PAIR_SP      // 11b for flags and register A OR stack pointer
} REG_PAIR;



#pragma pack(push, 1) // pad to 1 byte

/* Flags (Status) Register
---------------------------------
| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
---------------------------------
|   |   |   | A |   |   |   |   |
| S | Z | 0 | C | 0 | P | 1 | C |
---------------------------------
*/

typedef struct _status {
	BYTE C  : 1; // Carry (C) Flag Bit
	BYTE    : 1; // Reserved (1)
	BYTE P  : 1; // Parity (P) Flag Bit
	BYTE    : 1; // Reserved (0)
	BYTE AC : 1; // Auxiliary Carry (AC) Flag Bit
	BYTE    : 1; // Reserved (0)
	BYTE Z  : 1; // Zero (Z) Flag Bit
	BYTE S  : 1; // Sign (S) Flag Bit
} STATUS;

// Intel 8080 registers set
typedef struct _INTEL_8080 {
	WORD PC; // Program Counter
	WORD SP; // Stack Pointer

	union {
		WORD reg_w[4]; // register pairs: BC(0), DE(1), HL(2), AF(3)
		BYTE reg_b[8]; // regs: B(0), C(1), D(2), E(3), H(4), L(5), F(6), A(7)

		struct {
			union {
				WORD BC; // B and C (0 and 1)
				struct {
					BYTE B;
					BYTE C;
				};
			};

			union {
				WORD DE; // D and E (2 and 3)
				struct {
					BYTE D;
					BYTE E;
				};
			};

			union {
				WORD HL; // H and L (4 and 5)
				struct {
					uint8_t H; // most significant 8 bits of the address
					uint8_t L; // least significant 8 bits of the address
				};
			};

			union {
				WORD PSW; // Program Status Word - A and F (6 and 7)
				struct {
					union {
						BYTE F; // Flags (Status) Register
						STATUS status;
					};
					BYTE A; // Accumulator Register
				};
			};

		};
	};
} INTEL_8080;

#pragma pack(pop)
