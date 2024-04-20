#pragma once
#include <inttypes.h>

#define SET 1
#define RESET 0

#define SWAPORDER(uint16_t) ((uint16_t)(uint16_t<<8)|(uint16_t>>8))

// REGISTER uint8_t INSTRUCTIONS
typedef enum _REG {
	REG_B = 0, // 000b for Register B
	REG_C,	   // 001b for register C
	REG_D,	   // 010b for register D
	REG_E,	   // 011b for register E
	REG_H,	   // 100b for register H
	REG_L,	   // 101b for register L
	REG_M,	   // 110b for memory ref. M
	REG_A	   // 111b for register A
} REG_ID;

// returns real index of register in little endian machine
static inline int le_reg(uint8_t id) {
	if (id >= REG_M) return id;
	return (id % 2 == 1) ? (id - 1) : (id + 1);
}

// REGISTER PAIR INSTRUCTIONS
typedef enum _REG_PAIR {
	REG_PAIR_BC = 0, // 00b for registers B and C
	REG_PAIR_DE,     // 01b for registers D and E
	REG_PAIR_HL,     // 10b for registers H and L
	REG_PAIR_SP      // 11b for flags and register A OR stack pointer
} REG_PAIR_ID;



#pragma pack(push, 1) // pad to 1 uint8_t

/* Flags (Status) Register
|-------------------------------|
| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|-------------------------------|
|   |   |   | A |   |   |   |   |
| S | Z | 0 | C | 0 | P | 1 | C |
|-------------------------------|
*/

typedef struct _status {
	uint8_t C  : 1; // Carry (C) Flag Bit
	uint8_t    : 1; // Reserved (1)
	uint8_t P  : 1; // Parity (P) Flag Bit
	uint8_t    : 1; // Reserved (0)
	uint8_t AC : 1; // Auxiliary Carry (AC) Flag Bit
	uint8_t    : 1; // Reserved (0)
	uint8_t Z  : 1; // Zero (Z) Flag Bit
	uint8_t S  : 1; // Sign (S) Flag Bit
} STATUS;

// Intel 8080 registers set
typedef struct _INTEL_8080 {
	// pointer to 64k memory
	uint8_t* MEM;

	uint8_t* PORT; // 0 - 255 I/O ports

	uint16_t PC; // Program Counter
	uint16_t SP; // Stack Pointer

	union {
		uint8_t REG[8]; // regs: B(0), C(1), D(2), E(3), H(4), L(5), F(6), A(7)
		uint16_t REG_W[4]; // register pairs: BC(0), DE(1), HL(2), AF(3)

		struct {
			union {
				uint16_t BC; // B and C (0 and 1)
				struct {
					uint8_t C;
					uint8_t B;
				};
			};

			union {
				uint16_t DE; // D and E (2 and 3)
				struct {
					uint8_t E;
					uint8_t D;
				};
			};

			union {
				uint16_t HL; // H and L (4 and 5)
				struct {
					uint8_t L; // least significant 8 bits of the address
					uint8_t H; // most significant 8 bits of the address
				};
			};

			union {
				uint16_t PSW; // Program Status uint16_t - A and F (6 and 7)
				struct {
					union {
						uint8_t F; // Flags (Status) Register
						STATUS status;
					};
					uint8_t A; // Accumulator Register
				};
			};

		};
	};

	uint8_t HALT; // is CPU halted
	uint8_t INT; // has CPU enabled interrupts
	uint8_t INT_PENDING; // has CPU pending interrupt
	uint8_t INT_VECTOR; // number of RST routine to execute

	uint32_t CYCLES;
	uint32_t INSTRUCTIONS;
} INTEL_8080;

#pragma pack(pop)

// HELPERS

void write_uint16_t_on_stack(INTEL_8080* i8080, uint16_t uint16_t);
uint16_t read_uint16_t_from_stack(INTEL_8080* i8080);

// INSTRUCTION SET

typedef uint8_t(*INSTRUCTION)(INTEL_8080* i8080);

uint8_t cmc(INTEL_8080* i8080);
uint8_t stc(INTEL_8080* i8080);

uint8_t inr(INTEL_8080* i8080);
uint8_t dcr(INTEL_8080* i8080);
uint8_t cma(INTEL_8080* i8080);
uint8_t daa(INTEL_8080* i8080);

uint8_t nop(INTEL_8080* i8080);

uint8_t mov(INTEL_8080* i8080);
uint8_t stax(INTEL_8080* i8080);
uint8_t ldax(INTEL_8080* i8080);

uint8_t add(INTEL_8080* i8080);
uint8_t adc(INTEL_8080* i8080);
uint8_t sub(INTEL_8080* i8080);
uint8_t sbb(INTEL_8080* i8080);
uint8_t ana(INTEL_8080* i8080);
uint8_t xra(INTEL_8080* i8080);
uint8_t ora(INTEL_8080* i8080);
uint8_t cmp(INTEL_8080* i8080);

uint8_t rlc(INTEL_8080* i8080);
uint8_t rrc(INTEL_8080* i8080);
uint8_t ral(INTEL_8080* i8080);
uint8_t rar(INTEL_8080* i8080);

uint8_t push(INTEL_8080* i8080);
uint8_t pop(INTEL_8080* i8080);
uint8_t dad(INTEL_8080* i8080);
uint8_t inx(INTEL_8080* i8080);
uint8_t dcx(INTEL_8080* i8080);
uint8_t xchg(INTEL_8080* i8080);
uint8_t xthl(INTEL_8080* i8080);
uint8_t sphl(INTEL_8080* i8080);

uint8_t lxi(INTEL_8080* i8080);
uint8_t mvi(INTEL_8080* i8080);
uint8_t adi(INTEL_8080* i8080);
uint8_t aci(INTEL_8080* i8080);
uint8_t sui(INTEL_8080* i8080);
uint8_t sbi(INTEL_8080* i8080);
uint8_t ani(INTEL_8080* i8080);
uint8_t xri(INTEL_8080* i8080);
uint8_t ori(INTEL_8080* i8080);
uint8_t cpi(INTEL_8080* i8080);

uint8_t sta(INTEL_8080* i8080);
uint8_t lda(INTEL_8080* i8080);
uint8_t shld(INTEL_8080* i8080);
uint8_t lhld(INTEL_8080* i8080);

uint8_t pchl(INTEL_8080* i8080);
uint8_t jmp(INTEL_8080* i8080);
uint8_t jc(INTEL_8080* i8080);
uint8_t jnc(INTEL_8080* i8080);
uint8_t jz(INTEL_8080* i8080);
uint8_t jnz(INTEL_8080* i8080);
uint8_t jm(INTEL_8080* i8080);
uint8_t jp(INTEL_8080* i8080);
uint8_t jpe(INTEL_8080* i8080);
uint8_t jpo(INTEL_8080* i8080);

uint8_t call(INTEL_8080* i8080);
uint8_t cc(INTEL_8080* i8080);
uint8_t cnc(INTEL_8080* i8080);
uint8_t cz(INTEL_8080* i8080);
uint8_t cnz(INTEL_8080* i8080);
uint8_t cm(INTEL_8080* i8080);
uint8_t cp(INTEL_8080* i8080);
uint8_t cpe(INTEL_8080* i8080);
uint8_t cpo(INTEL_8080* i8080);

uint8_t ret(INTEL_8080* i8080);
uint8_t rc(INTEL_8080* i8080);
uint8_t rnc(INTEL_8080* i8080);
uint8_t rz(INTEL_8080* i8080);
uint8_t rnz(INTEL_8080* i8080);
uint8_t rm(INTEL_8080* i8080);
uint8_t rp(INTEL_8080* i8080);
uint8_t rpe(INTEL_8080* i8080);
uint8_t rpo(INTEL_8080* i8080);

uint8_t rst(INTEL_8080* i8080);
uint8_t ei(INTEL_8080* i8080);
uint8_t di(INTEL_8080* i8080);
uint8_t in(INTEL_8080* i8080);
uint8_t out(INTEL_8080* i8080);
uint8_t hlt(INTEL_8080* i8080);
