#pragma once
#include <inttypes.h>

typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint8_t  BOOL;

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
|-------------------------------|
| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|-------------------------------|
|   |   |   | A |   |   |   |   |
| S | Z | 0 | C | 0 | P | 1 | C |
|-------------------------------|
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
	// pointer to 64k memory
	union {
		BYTE* MEM;
		WORD* MEM_W;
	};

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

typedef BYTE(*INSTRUCTION)(INTEL_8080* i8080);
const static INSTRUCTION OPCODE_TABLE[256];

BYTE cmc(INTEL_8080* i8080);
BYTE stc(INTEL_8080* i8080);
BYTE inr(INTEL_8080* i8080);
BYTE dcr(INTEL_8080* i8080);
BYTE cma(INTEL_8080* i8080);
BYTE daa(INTEL_8080* i8080);
BYTE stax(INTEL_8080* i8080);
BYTE ldax(INTEL_8080* i8080);
BYTE mov(INTEL_8080* i8080);
BYTE add(INTEL_8080* i8080);
BYTE adc(INTEL_8080* i8080);
BYTE sub(INTEL_8080* i8080);
BYTE sbb(INTEL_8080* i8080);
BYTE ana(INTEL_8080* i8080);
BYTE xra(INTEL_8080* i8080);
BYTE ora(INTEL_8080* i8080);
BYTE cmp(INTEL_8080* i8080);
BYTE rlc(INTEL_8080* i8080);
BYTE rrc(INTEL_8080* i8080);
BYTE ral(INTEL_8080* i8080);
BYTE rar(INTEL_8080* i8080);
BYTE push(INTEL_8080* i8080);
BYTE pop(INTEL_8080* i8080);
BYTE dad(INTEL_8080* i8080);
BYTE inx(INTEL_8080* i8080);
BYTE dcx(INTEL_8080* i8080);
BYTE xchg(INTEL_8080* i8080);
BYTE xthl(INTEL_8080* i8080);
BYTE sphl(INTEL_8080* i8080);
BYTE mvi(INTEL_8080* i8080);
BYTE adi(INTEL_8080* i8080);
BYTE aci(INTEL_8080* i8080);
BYTE sui(INTEL_8080* i8080);
BYTE sbi(INTEL_8080* i8080);
BYTE ani(INTEL_8080* i8080);
BYTE xri(INTEL_8080* i8080);
BYTE ori(INTEL_8080* i8080);
BYTE cpi(INTEL_8080* i8080);
BYTE sta(INTEL_8080* i8080);
BYTE lda(INTEL_8080* i8080);
BYTE shld(INTEL_8080* i8080);
BYTE lhld(INTEL_8080* i8080);
BYTE jmp(INTEL_8080* i8080);
BYTE jc(INTEL_8080* i8080);
BYTE jnc(INTEL_8080* i8080);
BYTE jz(INTEL_8080* i8080);
BYTE jnz(INTEL_8080* i8080);
BYTE jm(INTEL_8080* i8080);
BYTE jp(INTEL_8080* i8080);
BYTE jpe(INTEL_8080* i8080);
BYTE jpo(INTEL_8080* i8080);
BYTE call(INTEL_8080* i8080);
BYTE cc(INTEL_8080* i8080);
BYTE cnc(INTEL_8080* i8080);
BYTE cz(INTEL_8080* i8080);
BYTE cnz(INTEL_8080* i8080);
BYTE cm(INTEL_8080* i8080);
BYTE cp(INTEL_8080* i8080);
BYTE cpe(INTEL_8080* i8080);
BYTE cpo(INTEL_8080* i8080);
BYTE ret(INTEL_8080* i8080);
BYTE rc(INTEL_8080* i8080);
BYTE rnc(INTEL_8080* i8080);
BYTE rz(INTEL_8080* i8080);
BYTE rnz(INTEL_8080* i8080);
BYTE rm(INTEL_8080* i8080);
BYTE rp(INTEL_8080* i8080);
BYTE rpe(INTEL_8080* i8080);
BYTE rpo(INTEL_8080* i8080);
BYTE rst(INTEL_8080* i8080);
BYTE ei(INTEL_8080* i8080);
BYTE di(INTEL_8080* i8080);
BYTE in(INTEL_8080* i8080);
BYTE out(INTEL_8080* i8080);
BYTE hlt(INTEL_8080* i8080);
