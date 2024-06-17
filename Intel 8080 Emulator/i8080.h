#pragma once
#ifdef E_I8080
#define CLK_MORE 0
#define CLK_LESS 0
#define JMP_DIFF 0
#define CALL_DIFF 0
#define RET_DIFF 0x100
#elif defined E_I8085
#define CLK_MORE 1
#define CLK_LESS -1
#define JMP_DIFF -3
#define CALL_DIFF -2
#define RET_DIFF 0x200
#endif
#include <inttypes.h>

#define SET 1
#define RESET 0

#define SWAPORDER(word) ((uint16_t)(word<<8)|(word>>8))
#define MAKERESULT(bytes, cycles) ((uint16_t)(((cycles)<<8)|(bytes)))
#define GETINSTRUCTIONBYTES(result) ((uint8_t)(result&0x00FF))
#define GETINSTRUCTIONCYCLES(result) ((uint8_t)((result&0xFF00)>>8))


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


struct _INTEL_8080;

typedef uint16_t(*INSTRUCTION)(_INTEL_8080* i8080);



#pragma pack(push, 1)

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
#ifndef E_I8085
	uint8_t    : 1; // Reserved (1)
#else
	uint8_t U  : 1; // Underflow Indicator (UI) Flag Bit
#endif
	uint8_t P  : 1; // Parity (P) Flag Bit
	uint8_t    : 1; // Reserved (0)
	uint8_t AC : 1; // Auxiliary Carry (AC) Flag Bit
#ifndef E_I8085
	uint8_t    : 1; // Reserved (0)
#else
	uint8_t V  : 1; // Overflow Flag Bit
#endif
	uint8_t Z  : 1; // Zero (Z) Flag Bit
	uint8_t S  : 1; // Sign (S) Flag Bit
} STATUS;

// Intel 8080 registers set
typedef struct _INTEL_8080 {
	
	uint8_t* MEM; // pointer to 64k memory

	uint8_t* PORT; // 0 - 255 I/O ports

	uint64_t CYCLES; // number of cycles
	const INSTRUCTION* INSTRUCTIONS; // pointer to opcode table
	uint8_t  STEPPING; // is instruction stepping

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

	uint16_t INT_VECTOR; // address of routine to execute
	uint8_t HALT; // is CPU halted
	uint8_t INT_ENABLE; // has CPU enabled interrupts
	uint8_t INT_PENDING; // has CPU pending interrupt

} INTEL_8080;

#pragma pack(pop)

// HELPERS

uint8_t uint8_t_arg(const INTEL_8080* i8080);
uint16_t uint16_t_arg(const INTEL_8080* i8080);
uint8_t opcode(const INTEL_8080* i8080);
void write_uint16_t_on_stack(INTEL_8080* i8080, uint16_t uint16_t);
uint16_t read_uint16_t_from_stack(INTEL_8080* i8080);

// INSTRUCTION SET

uint16_t cmc(INTEL_8080* i8080);
uint16_t stc(INTEL_8080* i8080);

uint16_t inr(INTEL_8080* i8080);
uint16_t dcr(INTEL_8080* i8080);
uint16_t cma(INTEL_8080* i8080);
uint16_t daa(INTEL_8080* i8080);

uint16_t nop(INTEL_8080* i8080);

uint16_t mov(INTEL_8080* i8080);
uint16_t stax(INTEL_8080* i8080);
uint16_t ldax(INTEL_8080* i8080);

uint16_t add(INTEL_8080* i8080);
uint16_t adc(INTEL_8080* i8080);
uint16_t sub(INTEL_8080* i8080);
uint16_t sbb(INTEL_8080* i8080);
uint16_t ana(INTEL_8080* i8080);
uint16_t xra(INTEL_8080* i8080);
uint16_t ora(INTEL_8080* i8080);
uint16_t cmp(INTEL_8080* i8080);

uint16_t rlc(INTEL_8080* i8080);
uint16_t rrc(INTEL_8080* i8080);
uint16_t ral(INTEL_8080* i8080);
uint16_t rar(INTEL_8080* i8080);

uint16_t push(INTEL_8080* i8080);
uint16_t pop(INTEL_8080* i8080);
uint16_t dad(INTEL_8080* i8080);
uint16_t inx(INTEL_8080* i8080);
uint16_t dcx(INTEL_8080* i8080);
uint16_t xchg(INTEL_8080* i8080);
uint16_t xthl(INTEL_8080* i8080);
uint16_t sphl(INTEL_8080* i8080);

uint16_t lxi(INTEL_8080* i8080);
uint16_t mvi(INTEL_8080* i8080);
uint16_t adi(INTEL_8080* i8080);
uint16_t aci(INTEL_8080* i8080);
uint16_t sui(INTEL_8080* i8080);
uint16_t sbi(INTEL_8080* i8080);
uint16_t ani(INTEL_8080* i8080);
uint16_t xri(INTEL_8080* i8080);
uint16_t ori(INTEL_8080* i8080);
uint16_t cpi(INTEL_8080* i8080);

uint16_t sta(INTEL_8080* i8080);
uint16_t lda(INTEL_8080* i8080);
uint16_t shld(INTEL_8080* i8080);
uint16_t lhld(INTEL_8080* i8080);

uint16_t pchl(INTEL_8080* i8080);
uint16_t jmp(INTEL_8080* i8080);
uint16_t jc(INTEL_8080* i8080);
uint16_t jnc(INTEL_8080* i8080);
uint16_t jz(INTEL_8080* i8080);
uint16_t jnz(INTEL_8080* i8080);
uint16_t jm(INTEL_8080* i8080);
uint16_t jp(INTEL_8080* i8080);
uint16_t jpe(INTEL_8080* i8080);
uint16_t jpo(INTEL_8080* i8080);

uint16_t call(INTEL_8080* i8080);
uint16_t cc(INTEL_8080* i8080);
uint16_t cnc(INTEL_8080* i8080);
uint16_t cz(INTEL_8080* i8080);
uint16_t cnz(INTEL_8080* i8080);
uint16_t cm(INTEL_8080* i8080);
uint16_t cp(INTEL_8080* i8080);
uint16_t cpe(INTEL_8080* i8080);
uint16_t cpo(INTEL_8080* i8080);

uint16_t ret(INTEL_8080* i8080);
uint16_t rc(INTEL_8080* i8080);
uint16_t rnc(INTEL_8080* i8080);
uint16_t rz(INTEL_8080* i8080);
uint16_t rnz(INTEL_8080* i8080);
uint16_t rm(INTEL_8080* i8080);
uint16_t rp(INTEL_8080* i8080);
uint16_t rpe(INTEL_8080* i8080);
uint16_t rpo(INTEL_8080* i8080);

uint16_t rst(INTEL_8080* i8080);
uint16_t ei(INTEL_8080* i8080);
uint16_t di(INTEL_8080* i8080);
uint16_t in(INTEL_8080* i8080);
uint16_t out(INTEL_8080* i8080);
uint16_t hlt(INTEL_8080* i8080);

#ifdef E_I8080
static const char* OPCODE_NAME[256] = {
	"NOP", "LXI B,%hXh", "STAX B", "INX B", "INR B", "DCR B", "MVI B,%hhXh", "RLC", // 0x00 - 0x07
	"NOP", "DAD B", "LDAX B", "DCX B", "INR C", "DCR C", "MVI C,%hhXh", "RRC", // 0x08 - 0x0F
	"NOP", "LXI D,%hXh", "STAX D", "INX D", "INR D", "DCR D", "MVI D,%hhXh", "RAL", // 0x10 - 0x17
	"NOP", "DAD D", "LDAX D", "DCX D", "INR E", "DCR E", "MVI E,%hhXh", "RAR", // 0x18 - 0x1F
	"NOP", "LXI H,%hXh", "SHLD %hXh", "INX H", "INR H", "DCR H", "MVI H,%hhXh", "DAA", // 0x20 - 0x27
	"NOP", "DAD H", "LHLD %hXh", "DCX H", "INR L", "DCR L", "MVI L,%hhXh", "CMA", // 0x28 - 0x2F
	"NOP", "LXI SP,%hXh", "STA %hXh", "INX SP", "INR M", "DCR M", "MVI M,%hhXh", "STC", // 0x30 - 0x37
	"NOP", "DAD SP", "LDA %hXh", "DCX SP", "INR A", "DCR A", "MVI A,%hhXh", "CMC", // 0x38 - 0x3F
	"MOV B,B", "MOV B,C", "MOV B,D", "MOV B,E", "MOV B,H", "MOV B,L", "MOV B,M", "MOV B,A", // 0x40 - 0x47
	"MOV C,B", "MOV C,C", "MOV C,D", "MOV C,E", "MOV C,H", "MOV C,L", "MOV C,M", "MOV C,A", // 0x48 - 0x4F
	"MOV D,B", "MOV D,C", "MOV D,D", "MOV D,E", "MOV D,H", "MOV D,L", "MOV D,M", "MOV D,A", // 0x50 - 0x57
	"MOV E,B", "MOV E,C", "MOV E,D", "MOV E,E", "MOV E,H", "MOV E,L", "MOV E,M", "MOV E,A", // 0x58 - 0x5F
	"MOV H,B", "MOV H,C", "MOV H,D", "MOV H,E", "MOV H,H", "MOV H,L", "MOV H,M", "MOV H,A", // 0x60 - 0x67
	"MOV L,B", "MOV L,C", "MOV L,D", "MOV L,E", "MOV L,H", "MOV L,L", "MOV L,M", "MOV L,A", // 0x68 - 0x6F
	"MOV M,B", "MOV M,C", "MOV M,D", "MOV M,E", "MOV M,H", "MOV M,L", "HLT", "MOV M,A", // 0x70 - 0x77
	"MOV A,B", "MOV A,C", "MOV A,D", "MOV A,E", "MOV A,H", "MOV A,L", "MOV A,M", "MOV A,A", // 0x78 - 0x7F
	"ADD B", "ADD C", "ADD D", "ADD E", "ADD H", "ADD L", "ADD M", "ADD A", // 0x80 - 0x87
	"ADC B", "ADC C", "ADC D", "ADC E", "ADC H", "ADC L", "ADC M", "ADC A", // 0x88 - 0x8F
	"SUB B", "SUB C", "SUB D", "SUB E", "SUB H", "SUB L", "SUB M", "SUB A", // 0x90 - 0x97
	"SBB B", "SBB C", "SBB D", "SBB E", "SBB H", "SBB L", "SBB M", "SBB A", // 0x98 - 0x9F
	"ANA B", "ANA C", "ANA D", "ANA E", "ANA H", "ANA L", "ANA M", "ANA A", // 0xA0 - 0xA7
	"XRA B", "XRA C", "XRA D", "XRA E", "XRA H", "XRA L", "XRA M", "XRA A", // 0xA8 - 0xAF
	"ORA B", "ORA C", "ORA D", "ORA E", "ORA H", "ORA L", "ORA M", "ORA A", // 0xB0 - 0xB7
	"CMP B", "CMP C", "CMP D", "CMP E", "CMP H", "CMP L", "CMP M", "CMP A", // 0xB8 - 0xBF
	"RNZ", "POP B", "JNZ %hXh", "JMP %hXh", "CNZ %hXh", "PUSH B", "ADI %hhXh", "RST 0", // 0xC0 - 0xC7
	"RZ", "RET", "JZ %hXh", "NOP", "CZ %hXh", "CALL %hXh", "ACI %hhXh", "RST 1", // 0xC8 - 0xCF
	"RNC", "POP D", "JNC %hXh", "OUT %hhXh", "CNC %hXh", "PUSH D", "SUI %hhXh", "RST 2", // 0xD0 - 0xD7
	"RC", "NOP", "JC %hXh", "IN %hhXh", "CC %hXh", "NOP", "SBI %hhXh", "RST 3", // 0xD8 - 0xDF
	"RPO", "POP H", "JPO %hXh", "XTHL", "CPO %hXh", "PUSH H", "ANI %hhXh", "RST 4", // 0xE0 - 0xE7
	"RPE", "PCHL", "JPE %hXh", "XCHG", "CPE %hXh", "NOP", "XRI %hhXh", "RST 5", // 0xE8 - 0xEF
	"RP", "POP PSW", "JP %hXh", "DI", "CP %hXh", "PUSH PSW", "ORI %hhXh", "RST 6", // 0xF0 - 0xF7
	"RM", "SPHL", "JM %hXh", "EI", "CM %hXh", "NOP", "CPI %hhXh", "RST 7" // 0xF8 - 0xFF
};

static const uint8_t OPCODE_LENGTH[256] = {
	1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 0x00 - 0x0F
	1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 0x10 - 0x1F
	1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1, // 0x20 - 0x2F
	1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1, // 0x30 - 0x3F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x50 - 0x5F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x70 - 0x7F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x80 - 0x8F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x90 - 0x9F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xA0 - 0xAF
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xB0 - 0xBF
	1, 1, 3, 0, 3, 1, 1, 0, 1, 0, 3, 1, 3, 0, 2, 0, // 0xC0 - 0xCF
	1, 1, 3, 2, 3, 1, 2, 0, 1, 1, 3, 2, 3, 1, 2, 0, // 0xD0 - 0xDF
	1, 1, 3, 1, 3, 1, 2, 0, 1, 0, 3, 1, 3, 1, 2, 0, // 0xE0 - 0xEF
	1, 1, 3, 1, 3, 1, 2, 0, 1, 1, 3, 1, 3, 1, 2, 0  // 0xF0 - 0xFF
};

static const INSTRUCTION OPCODE_TABLE[256] = {
	nop, lxi,  stax, inx,  inr, dcr,  mvi, rlc, // 0x00 - 0x07
	nop, dad,  ldax, dcx,  inr, dcr,  mvi, rrc, // 0x08 - 0x0F
	nop, lxi,  stax, inx,  inr, dcr,  mvi, ral, // 0x10 - 0x17
	nop, dad,  ldax, dcx,  inr, dcr,  mvi, rar, // 0x18 - 0x1F
	nop, lxi,  shld, inx,  inr, dcr,  mvi, daa, // 0x20 - 0x27
	nop, dad,  lhld, dcx,  inr, dcr,  mvi, cma, // 0x28 - 0x2F
	nop, lxi,  sta,  inx,  inr, dcr,  mvi, stc, // 0x30 - 0x37
	nop, dad,  lda,  dcx,  inr, dcr,  mvi, cmc, // 0x38 - 0x3F
	mov, mov,  mov,  mov,  mov, mov,  mov, mov, // 0x40 - 0x47
	mov, mov,  mov,  mov,  mov, mov,  mov, mov, // 0x48 - 0x4F
	mov, mov,  mov,  mov,  mov, mov,  mov, mov, // 0x50 - 0x57
	mov, mov,  mov,  mov,  mov, mov,  mov, mov, // 0x58 - 0x5F
	mov, mov,  mov,  mov,  mov, mov,  mov, mov, // 0x60 - 0x67
	mov, mov,  mov,  mov,  mov, mov,  mov, mov, // 0x68 - 0x6F
	mov, mov,  mov,  mov,  mov, mov,  hlt, mov, // 0x70 - 0x77
	mov, mov,  mov,  mov,  mov, mov,  mov, mov, // 0x78 - 0x7F
	add, add,  add,  add,  add, add,  add, add, // 0x80 - 0x87
	adc, adc,  adc,  adc,  adc, adc,  adc, adc, // 0x88 - 0x8F
	sub, sub,  sub,  sub,  sub, sub,  sub, sub, // 0x90 - 0x97
	sbb, sbb,  sbb,  sbb,  sbb, sbb,  sbb, sbb, // 0x98 - 0x9F
	ana, ana,  ana,  ana,  ana, ana,  ana, ana, // 0xA0 - 0xA7
	xra, xra,  xra,  xra,  xra, xra,  xra, xra, // 0xA8 - 0xAF
	ora, ora,  ora,  ora,  ora, ora,  ora, ora, // 0xB0 - 0xB7
	cmp, cmp,  cmp,  cmp,  cmp, cmp,  cmp, cmp, // 0xB8 - 0xBF
	rnz, pop,  jnz,  jmp,  cnz, push, adi, rst, // 0xC0 - 0xC7
	rz,  ret,  jz,   nop,  cz,  call, aci, rst, // 0xC8 - 0xCF
	rnc, pop,  jnc,  out,  cnc, push, sui, rst, // 0xD0 - 0xD7
	rc,  nop,  jc,   in,   cc,  nop,  sbi, rst, // 0xD8 - 0xDF
	rpo, pop,  jpo,  xthl, cpo, push, ani, rst, // 0xE0 - 0xE7
	rpe, pchl, jpe,  xchg, cpe, nop,  xri, rst, // 0xE8 - 0xEF
	rp,  pop,  jp,   di,   cp,  push, ori, rst, // 0xF0 - 0xF7
	rm,  sphl, jm,   ei,   cm,  nop,  cpi, rst  // 0xF8 - 0xFF
};
#endif // I8080
