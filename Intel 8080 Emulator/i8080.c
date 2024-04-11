#include <assert.h>
#include "i8080.h"

#define MAKEWORD(low,high) ((WORD)((BYTE)low))|(((WORD)((BYTE)high))<<8)

// returns a current opcode at address of program counter
static inline BYTE opcode(const INTEL_8080* i8080) {
	return i8080->MEM[i8080->PC];
}

/*
returns a bits from an opcode
example: l = 5, r = 3 will results in [543] bits
l and r cannot be greater than 7 and r < l
*/ 
static inline BYTE opcode_bits(
	const INTEL_8080* i8080,
	const BYTE l, // the most significant bit
	const BYTE r  // the least significant bit
) {
	assert(l < 8 && r < 7);
	assert(r < l);
	return (opcode(i8080) & (1 << (l + 1)) - 1) >> r;
}

// returns a current byte argument at address of program counter + 1
static inline BYTE byte_arg(const INTEL_8080* i8080) {
	return i8080->MEM[i8080->PC + 1];
}

// returns a current word argument at address of program counter + 1
static inline WORD word_arg(const INTEL_8080* i8080) {
	return i8080->MEM_W[i8080->PC + 1];
}



static BYTE cmc(INTEL_8080* i8080) {
	return 1;
}

static BYTE stc(INTEL_8080* i8080) {
	return 1;
}

static BYTE inr(INTEL_8080* i8080) {
	return 1;
}

static BYTE dcr(INTEL_8080* i8080) {
	return 1;
}

static BYTE cma(INTEL_8080* i8080) {
	return 1;
}

static BYTE daa(INTEL_8080* i8080) {
	return 1;
}

static BYTE stax(INTEL_8080* i8080) {
	return 1;
}

static BYTE ldax(INTEL_8080* i8080) {
	return 1;
}

static BYTE mov(INTEL_8080* i8080) {
	return 1;
}

static BYTE add(INTEL_8080* i8080) {
	return 1;
}

static BYTE adc(INTEL_8080* i8080) {
	return 1;
}

static BYTE sub(INTEL_8080* i8080) {
	return 1;
}

static BYTE sbb(INTEL_8080* i8080) {
	return 1;
}

static BYTE ana(INTEL_8080* i8080) {
	return 1;
}

static BYTE xra(INTEL_8080* i8080) {
	return 1;
}

static BYTE ora(INTEL_8080* i8080) {
	return 1;
}

static BYTE cmp(INTEL_8080* i8080) {
	return 1;
}

static BYTE rlc(INTEL_8080* i8080) {
	return 1;
}

static BYTE rrc(INTEL_8080* i8080) {
	return 1;
}

static BYTE ral(INTEL_8080* i8080) {
	return 1;
}

static BYTE rar(INTEL_8080* i8080) {
	return 1;
}

static BYTE push(INTEL_8080* i8080) {
	return 1;
}

static BYTE pop(INTEL_8080* i8080) {
	return 1;
}

static BYTE dad(INTEL_8080* i8080) {
	return 1;
}

static BYTE inx(INTEL_8080* i8080) {
	return 1;
}

static BYTE dcx(INTEL_8080* i8080) {
	return 1;
}

static BYTE xchg(INTEL_8080* i8080) {
	return 1;
}

static BYTE xthl(INTEL_8080* i8080) {
	return 1;
}

static BYTE sphl(INTEL_8080* i8080) {
	return 1;
}

static BYTE mvi(INTEL_8080* i8080) {
	BYTE reg = opcode_bits(i8080, 5, 3);
	(reg != REG_M) ? (i8080->reg_b[reg] = byte_arg(i8080)) :
		(i8080->MEM[i8080->HL] = byte_arg(i8080));
	return 2;
}

static BYTE adi(INTEL_8080* i8080) {
	return 2;
}

static BYTE aci(INTEL_8080* i8080) {
	return 2;
}

static BYTE sui(INTEL_8080* i8080) {
	return 2;
}

static BYTE sbi(INTEL_8080* i8080) {
	return 2;
}

static BYTE ani(INTEL_8080* i8080) {
	return 2;
}

static BYTE xri(INTEL_8080* i8080) {
	return 2;
}

static BYTE ori(INTEL_8080* i8080) {
	return 2;
}

static BYTE cpi(INTEL_8080* i8080) {
	return 2;
}

static BYTE sta(INTEL_8080* i8080) {
	return 3;
}

static BYTE lda(INTEL_8080* i8080) {
	return 3;
}

static BYTE shld(INTEL_8080* i8080) {
	return 3;
}

static BYTE lhld(INTEL_8080* i8080) {
	return 3;
}

static BYTE jmp(INTEL_8080* i8080) {
	i8080->PC = word_arg(i8080);
	return 0;
}

static BYTE jc(INTEL_8080* i8080) {
	return i8080->status.C ? jmp(i8080) : 3;
}

static BYTE jnc(INTEL_8080* i8080) {
	return 3;
}

static BYTE jz(INTEL_8080* i8080) {
	return 3;
}

static BYTE jnz(INTEL_8080* i8080) {
	return 3;
}

static BYTE jm(INTEL_8080* i8080) {
	return 3;
}

static BYTE jp(INTEL_8080* i8080) {
	return 3;
}

static BYTE jpe(INTEL_8080* i8080) {
	return 3;
}

static BYTE jpo(INTEL_8080* i8080) {
	return 3;
}

static BYTE call(INTEL_8080* i8080) {
	i8080->SP -= 2;
	i8080->MEM[i8080->SP] = i8080->PC + 3;
	return jmp(i8080);
}

static BYTE cc(INTEL_8080* i8080) {
	return i8080->status.C ? call(i8080) : 3;
}

static BYTE cnc(INTEL_8080* i8080) {
	return 3;
}

static BYTE cz(INTEL_8080* i8080) {
	return 3;
}

static BYTE cnz(INTEL_8080* i8080) {
	return 3;
}

static BYTE cm(INTEL_8080* i8080) {
	return 3;
}

static BYTE cp(INTEL_8080* i8080) {
	return 3;
}

static BYTE cpe(INTEL_8080* i8080) {
	return 3;
}

static BYTE cpo(INTEL_8080* i8080) {
	return 3;
}

static BYTE ret(INTEL_8080* i8080) {
	i8080->PC = i8080->MEM[i8080->SP];
	i8080->SP += 2;
	return 0;
}

static BYTE rc(INTEL_8080* i8080) {
	return i8080->status.C ? ret(i8080) : 1;
}

static BYTE rnc(INTEL_8080* i8080) {
	return 1;
}

static BYTE rz(INTEL_8080* i8080) {
	return 1;
}

static BYTE rnz(INTEL_8080* i8080) {
	return 1;
}

static BYTE rm(INTEL_8080* i8080) {
	return 1;
}

static BYTE rp(INTEL_8080* i8080) {
	return 1;
}

static BYTE rpe(INTEL_8080* i8080) {
	return 1;
}

static BYTE rpo(INTEL_8080* i8080) {
	return 1;
}

static BYTE rst(INTEL_8080* i8080) {
	return 1;
}

static BYTE ei(INTEL_8080* i8080) {
	return 1;
}

static BYTE di(INTEL_8080* i8080) {
	return 1;
}

static BYTE in(INTEL_8080* i8080) {
	return 2;
}

static BYTE out(INTEL_8080* i8080) {
	return 2;
}

static BYTE hlt(INTEL_8080* i8080) {
	return 1;
}
