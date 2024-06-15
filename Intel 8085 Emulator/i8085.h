#pragma once
#ifdef E_I8085
#define CLK_MORE 0
#define CLK_LESS 0
#endif
#include "i8080.h"

enum HARDWARE_INTERRUPT {
	TRAP = 0x24,
	RST_75 = 0x3C,
	RST_65 = 0x34,
	RST_55 = 0x2C
};

#pragma pack(push, 1)

// Set Interrupt Mask bits
typedef struct _SIM_BITS {
	struct {
		uint8_t SOD : 1;
		uint8_t SOE : 1;
		uint8_t : 1;
		uint8_t RST75 : 1;
		uint8_t MSE : 1;
		uint8_t M75 : 1;
		uint8_t M65 : 1;
		uint8_t M55 : 1;
	};
	struct {
		uint8_t : 5;
		uint8_t MASKS : 3;
	};
} SIM_BITS;

// Read Interrupt Mask bits
typedef union _RIM_BITS {
	struct {
		uint8_t SID : 1;
		uint8_t P75 : 1;
		uint8_t P65 : 1;
		uint8_t P55 : 1;
		uint8_t IE : 1;
		uint8_t M75 : 1;
		uint8_t M65 : 1;
		uint8_t M55 : 1;
	};
	struct {
		uint8_t : 1;
		uint8_t PENDING : 3;
		uint8_t : 1;
		uint8_t MASKS : 3;
	};
} RIM_BITS;

// Intel 8085 registers set
typedef struct _INTEL_8085 {
	INTEL_8080 CORE;
	uint8_t SERIAL_IN;
	uint8_t SERIAL_OUT;
	RIM_BITS RIM;
	uint8_t TRAP;
	
} INTEL_8085;

#pragma pack(pop)

uint16_t sim(INTEL_8080* i8080);
uint16_t rim(INTEL_8080* i8080);

uint16_t arhl(INTEL_8080* i8080);
uint16_t dsub(INTEL_8080* i8080);
uint16_t jnui(INTEL_8080* i8080);
uint16_t jui(INTEL_8080* i8080);
uint16_t ldhi(INTEL_8080* i8080);
uint16_t ldsi(INTEL_8080* i8080);
uint16_t lhlx(INTEL_8080* i8080);
uint16_t rdel(INTEL_8080* i8080);
uint16_t rstv(INTEL_8080* i8080);
uint16_t shlx(INTEL_8080* i8080);

#ifdef E_I8085
static const char* OPCODE_NAME[256] = {
	"NOP", "LXI B,%hXh", "STAX B", "INX B", "INR B", "DCR B", "MVI B,%hhXh", "RLC", // 0x00 - 0x07
	"NOP", "DAD B", "LDAX B", "DCX B", "INR C", "DCR C", "MVI C,%hhXh", "RRC", // 0x08 - 0x0F
	"NOP", "LXI D,%hXh", "STAX D", "INX D", "INR D", "DCR D", "MVI D,%hhXh", "RAL", // 0x10 - 0x17
	"NOP", "DAD D", "LDAX D", "DCX D", "INR E", "DCR E", "MVI E,%hhXh", "RAR", // 0x18 - 0x1F
	"RIM", "LXI H,%hXh", "SHLD %hXh", "INX H", "INR H", "DCR H", "MVI H,%hhXh", "DAA", // 0x20 - 0x27
	"NOP", "DAD H", "LHLD %hXh", "DCX H", "INR L", "DCR L", "MVI L,%hhXh", "CMA", // 0x28 - 0x2F
	"SIM", "LXI SP,%hXh", "STA %hXh", "INX SP", "INR M", "DCR M", "MVI M,%hhXh", "STC", // 0x30 - 0x37
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
	rim, lxi,  shld, inx,  inr, dcr,  mvi, daa, // 0x20 - 0x27
	nop, dad,  lhld, dcx,  inr, dcr,  mvi, cma, // 0x28 - 0x2F
	sim, lxi,  sta,  inx,  inr, dcr,  mvi, stc, // 0x30 - 0x37
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
#endif // I8085
