#pragma once
#ifdef E_I8085
#define CLK_MORE 1
#define CLK_LESS -1
#define JMP_DIFF -3
#define CALL_DIFF -2
#define RET_DIFF 0x200
#define MOV_R_R_DIFF -1
#else
#define CLK_MORE 0
#define CLK_LESS 0
#define JMP_DIFF 0
#define CALL_DIFF 0
#define RET_DIFF 0x100
#ifdef E_NEC8080
#define MOV_R_R_DIFF -1
#else
#define MOV_R_R_DIFF 0
#endif
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
#ifdef E_I8085
	uint8_t U  : 1; // Underflow Indicator (UI/K) Flag Bit
#else
	uint8_t    : 1; // Reserved (1)
#endif
	uint8_t P  : 1; // Parity (P) Flag Bit
	uint8_t    : 1; // Reserved (0)
	uint8_t AC : 1; // Auxiliary Carry (AC) Flag Bit
#ifdef E_I8085
	uint8_t V  : 1; // Overflow Flag Bit
#elif defined E_NEC8080
	uint8_t B  : 1; // Substract (SUB) Flag Bit
#else
	uint8_t    : 1; // Reserved (0)
#endif
	uint8_t Z  : 1; // Zero (Z) Flag Bit
	uint8_t S  : 1; // Sign (S) Flag Bit
} STATUS;

// Intel 8080 registers set
typedef struct _INTEL_8080 {
	
	uint8_t* MEM; // pointer to 64k memory

	uint8_t* PORT; // 0 - 255 I/O ports

	uint64_t CYCLES; // number of cycles
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

// returns parity flag for given value
static uint8_t parity_check(const uint8_t val) {
	// Source: https://stackoverflow.com/a/48041356
	uint8_t x = val;
	x ^= x >> 4;
	x ^= x >> 2;
	x ^= x >> 1;
	return (~x) & 1;
}

// returns flag register based on extended arithmetic/logical operation result
static void set_ZSP_flags(
	INTEL_8080* i8080,
	const uint32_t result
) {
	i8080->status.Z = ((result & 0xFF) == 0);
	i8080->status.S = ((result & 0xFF) >= 0x80);
	i8080->status.P = parity_check(result & 0xFF);
}

static void set_UI_flag_int8(
	INTEL_8080* i8080
) {
#ifdef E_I8085
	i8080->status.U = i8080->status.V ^ i8080->status.V;
#endif
}

static void set_V_flag_int16(
	INTEL_8080* i8080,
	const int16_t arg1, // change to signed
	const int16_t arg2, // change to signed
	const int16_t result
) {
#ifdef E_I8085
	i8080->status.V = ((arg1 >= 0 && arg2 >= 0 && result < 0) ||
		(arg1 < 0 && arg2 < 0 && result >= 0));
#endif
}

static void set_V_flag_int8(
	INTEL_8080* i8080,
	const int8_t arg1, // change to signed
	const int8_t arg2, // change to signed
	const int8_t result
) {
#ifdef E_I8085
	i8080->status.V = ((arg1 >= 0 && arg2 >= 0 && result < 0) ||
		(arg1 < 0 && arg2 < 0 && result >= 0));
#endif
}

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

typedef struct _EMUL_STRUCT {
	const char*       OPCODE_NAME;
	const uint8_t     OPCODE_LENGTH;
	const INSTRUCTION OPCODE;
} EMUL_STRUCT;

#ifndef E_I8085
const EMUL_STRUCT EMUL_DATA[256] = {
	{"NOP", 1, nop}, {"LXI B,%hXh", 3, lxi}, {"STAX B", 1, stax}, {"INX B", 1, inx},
	{"INR B", 1, inr}, {"DCR B", 1, dcr}, {"MVI B,%hhXh", 2, mvi}, {"RLC", 1, rlc},
	{"NOP", 1, nop}, {"DAD B", 1, dad}, {"LDAX B", 1, ldax}, {"DCX B", 1, dcx},
	{"INR C", 1, inr}, {"DCR C", 1, dcr}, {"MVI C,%hhXh", 2, mvi}, {"RRC", 1, rrc},
	{"NOP", 1, nop}, {"LXI D,%hXh", 3, lxi}, {"STAX D", 1, stax}, {"INX D", 1, inx},
	{"INR D", 1, inr}, {"DCR D", 1, dcr}, {"MVI D,%hhXh", 2, mvi}, {"RAL", 1, ral},
	{"NOP", 1, nop}, {"DAD D", 1, dad}, {"LDAX D", 1, ldax}, {"DCX D", 1, dcx},
	{"INR E", 1, inr}, {"DCR E", 1, dcr}, {"MVI E,%hhXh", 2, mvi}, {"RAR", 1, rar},
	{"NOP", 1, nop}, {"LXI H,%hXh", 3, lxi}, {"SHLD %hXh", 3, shld}, {"INX H", 1, inx},
	{"INR H", 1, inr}, {"DCR H", 1, dcr}, {"MVI H,%hhXh", 2, mvi}, {"DAA", 1, daa},
	{"NOP", 1, nop}, {"DAD H", 1, dad}, {"LHLD %hXh", 3, lhld}, {"DCX H", 1, dcx},
	{"INR L", 1, inr}, {"DCR L", 1, dcr}, {"MVI L,%hhXh", 2, mvi}, {"CMA", 1, cma},
	{"NOP", 1, nop}, {"LXI SP,%hXh", 3, lxi}, {"STA %hXh", 3, sta}, {"INX SP", 1, inx},
	{"INR M", 1, inr}, {"DCR M", 1, dcr}, {"MVI M,%hhXh", 2, mvi}, {"STC", 1, stc},
	{"NOP", 1, nop}, {"DAD SP", 1, dad}, {"LDA %hXh", 3, lda}, {"DCX SP", 1, dcx},
	{"INR A", 1, inr}, {"DCR A", 1, dcr}, {"MVI A,%hhXh", 2, mvi}, {"CMC", 1, cmc},
	{"MOV B,B", 1, mov}, {"MOV B,C", 1, mov}, {"MOV B,D", 1, mov}, {"MOV B,E", 1, mov},
	{"MOV B,H", 1, mov}, {"MOV B,L", 1, mov}, {"MOV B,M", 1, mov}, {"MOV B,A", 1, mov},
	{"MOV C,B", 1, mov}, {"MOV C,C", 1, mov}, {"MOV C,D", 1, mov}, {"MOV C,E", 1, mov},
	{"MOV C,H", 1, mov}, {"MOV C,L", 1, mov}, {"MOV C,M", 1, mov}, {"MOV C,A", 1, mov},
	{"MOV D,B", 1, mov}, {"MOV D,C", 1, mov}, {"MOV D,D", 1, mov}, {"MOV D,E", 1, mov},
	{"MOV D,H", 1, mov}, {"MOV D,L", 1, mov}, {"MOV D,M", 1, mov}, {"MOV D,A", 1, mov},
	{"MOV E,B", 1, mov}, {"MOV E,C", 1, mov}, {"MOV E,D", 1, mov}, {"MOV E,E", 1, mov},
	{"MOV E,H", 1, mov}, {"MOV E,L", 1, mov}, {"MOV E,M", 1, mov}, {"MOV E,A", 1, mov},
	{"MOV H,B", 1, mov}, {"MOV H,C", 1, mov}, {"MOV H,D", 1, mov}, {"MOV H,E", 1, mov},
	{"MOV H,H", 1, mov}, {"MOV H,L", 1, mov}, {"MOV H,M", 1, mov}, {"MOV H,A", 1, mov},
	{"MOV L,B", 1, mov}, {"MOV L,C", 1, mov}, {"MOV L,D", 1, mov}, {"MOV L,E", 1, mov},
	{"MOV L,H", 1, mov}, {"MOV L,L", 1, mov}, {"MOV L,M", 1, mov}, {"MOV L,A", 1, mov},
	{"MOV M,B", 1, mov}, {"MOV M,C", 1, mov}, {"MOV M,D", 1, mov}, {"MOV M,E", 1, mov},
	{"MOV M,H", 1, mov}, {"MOV M,L", 1, mov}, {"HLT", 1, hlt}, {"MOV M,A", 1, mov},
	{"MOV A,B", 1, mov}, {"MOV A,C", 1, mov}, {"MOV A,D", 1, mov}, {"MOV A,E", 1, mov},
	{"MOV A,H", 1, mov}, {"MOV A,L", 1, mov}, {"MOV A,M", 1, mov}, {"MOV A,A", 1, mov},
	{"ADD B", 1, add}, {"ADD C", 1, add}, {"ADD D", 1, add}, {"ADD E", 1, add},
	{"ADD H", 1, add}, {"ADD L", 1, add}, {"ADD M", 1, add}, {"ADD A", 1, add},
	{"ADC B", 1, adc}, {"ADC C", 1, adc}, {"ADC D", 1, adc}, {"ADC E", 1, adc},
	{"ADC H", 1, adc}, {"ADC L", 1, adc}, {"ADC M", 1, adc}, {"ADC A", 1, adc},
	{"SUB B", 1, sub}, {"SUB C", 1, sub}, {"SUB D", 1, sub}, {"SUB E", 1, sub},
	{"SUB H", 1, sub}, {"SUB L", 1, sub}, {"SUB M", 1, sub}, {"SUB A", 1, sub},
	{"SBB B", 1, sbb}, {"SBB C", 1, sbb}, {"SBB D", 1, sbb}, {"SBB E", 1, sbb},
	{"SBB H", 1, sbb}, {"SBB L", 1, sbb}, {"SBB M", 1, sbb}, {"SBB A", 1, sbb},
	{"ANA B", 1, ana}, {"ANA C", 1, ana}, {"ANA D", 1, ana}, {"ANA E", 1, ana},
	{"ANA H", 1, ana}, {"ANA L", 1, ana}, {"ANA M", 1, ana}, {"ANA A", 1, ana},
	{"XRA B", 1, xra}, {"XRA C", 1, xra}, {"XRA D", 1, xra}, {"XRA E", 1, xra},
	{"XRA H", 1, xra}, {"XRA L", 1, xra}, {"XRA M", 1, xra}, {"XRA A", 1, xra},
	{"ORA B", 1, ora}, {"ORA C", 1, ora}, {"ORA D", 1, ora}, {"ORA E", 1, ora},
	{"ORA H", 1, ora}, {"ORA L", 1, ora}, {"ORA M", 1, ora}, {"ORA A", 1, ora},
	{"CMP B", 1, cmp}, {"CMP C", 1, cmp}, {"CMP D", 1, cmp}, {"CMP E", 1, cmp},
	{"CMP H", 1, cmp}, {"CMP L", 1, cmp}, {"CMP M", 1, cmp}, {"CMP A", 1, cmp},
	{"RNZ", 1, rnz}, {"POP B", 1, pop}, {"JNZ %hXh", 3, jnz}, {"JMP %hXh", 3, jmp},
	{"CNZ %hXh", 3, cnz}, {"PUSH B", 1, push}, {"ADI %hhXh", 2, adi}, {"RST 0", 1, rst},
	{"RZ", 1, rz}, {"RET", 1, ret}, {"JZ %hXh", 3, jz}, {"JMP %hXh", 3, jmp},
	{"CZ %hXh", 3, cz}, {"CALL %hXh", 3, call}, {"ACI %hhXh", 2, aci}, {"RST 1", 1, rst},
	{"RNC", 1, rnc}, {"POP D", 1, pop}, {"JNC %hXh", 3, jnc}, {"OUT %hhXh", 2, out},
	{"CNC %hXh", 3, cnc}, {"PUSH D", 1, push}, {"SUI %hhXh", 2, sui}, {"RST 2", 1, rst},
	{"RC", 1, rc}, {"RET", 1, ret}, {"JC %hXh", 3, jc}, {"IN %hhXh", 2, in},
	{"CC %hXh", 3, cc}, {"CALL %hXh", 3, call}, {"SBI %hhXh", 2, sbi}, {"RST 3", 1, rst},
	{"RPO", 1, rpo}, {"POP H", 1, pop}, {"JPO %hXh", 3, jpo}, {"XTHL", 1, xthl},
	{"CPO %hXh", 3, cpo}, {"PUSH H", 1, push}, {"ANI %hhXh", 2, ani}, {"RST 4", 1, rst},
	{"RPE", 1, rpe}, {"PCHL", 1, pchl}, {"JPE %hXh", 3, jpe}, {"XCHG", 1, xchg},
	{"CPE %hXh", 3, cpe}, {"CALL %hXh", 3, call}, {"XRI %hhXh", 2, xri}, {"RST 5", 1, rst},
	{"RP", 1, rp}, {"POP PSW", 1, pop}, {"JP %hXh", 3, jp}, {"DI", 1, di},
	{"CP %hXh", 3, cp}, {"PUSH PSW", 1, push}, {"ORI %hhXh", 2, ori}, {"RST 6", 1, rst},
	{"RM", 1, rm}, {"SPHL", 1, sphl}, {"JM %hXh", 3, jm}, {"EI", 1, ei},
	{"CM %hXh", 3, cm}, {"CALL %hXh", 3, call}, {"CPI %hhXh", 2, cpi}, {"RST 7", 1, rst}
};
#endif
