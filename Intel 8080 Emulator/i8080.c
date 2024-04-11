#include <stdlib.h>
#include <string.h>
#include "i8080.h"

#define MAKEWORD(low,high) ((WORD)((BYTE)low))|(((WORD)((BYTE)high))<<8)
#define OPCODE(addr) *((BYTE*)addr)
#define ARG1(addr) *(((BYTE*)addr)+1)
#define ARG2(addr) *(((BYTE*)addr)+2)

INTEL_8080 i8080;
BYTE* memory;

static inline int initialize(WORD origin_pc, WORD origin_sp) {
	memset(&i8080, 0, sizeof(i8080));
	i8080.F = 0b00000010;
	i8080.PC = origin_pc;
	i8080.SP = origin_sp;
	memory = (BYTE*)calloc(0x10000, sizeof(BYTE));
	if (memory == NULL)
		return 1;
	return 0;
}

static inline int emulate() {
	BYTE* current_addr = NULL;
	while (1) {
		current_addr = &memory[i8080.PC];
		if (OPCODE_TABLE[*current_addr] != NULL)
			i8080.PC += OPCODE_TABLE[*current_addr](current_addr);
		else
			i8080.PC++;
	}
	return 0;
}

static inline BYTE cmc(BYTE* code) {
	return 1;
}

static inline BYTE stc(BYTE* code) {
	return 1;
}

static inline BYTE inr(BYTE* code) {
	return 1;
}

static inline BYTE dcr(BYTE* code) {
	return 1;
}

static inline BYTE cma(BYTE* code) {
	return 1;
}

static inline BYTE daa(BYTE* code) {
	return 1;
}

static inline BYTE stax(BYTE* code) {
	return 1;
}

static inline BYTE ldax(BYTE* code) {
	return 1;
}

static inline BYTE mov(BYTE* code) {
	return 1;
}

static inline BYTE add(BYTE* code) {
	return 1;
}

static inline BYTE adc(BYTE* code) {
	return 1;
}

static inline BYTE sub(BYTE* code) {
	return 1;
}

static inline BYTE sbb(BYTE* code) {
	return 1;
}

static inline BYTE ana(BYTE* code) {
	return 1;
}

static inline BYTE xra(BYTE* code) {
	return 1;
}

static inline BYTE ora(BYTE* code) {
	return 1;
}

static inline BYTE cmp(BYTE* code) {
	return 1;
}

static inline BYTE rlc(BYTE* code) {
	return 1;
}

static inline BYTE rrc(BYTE* code) {
	return 1;
}

static inline BYTE ral(BYTE* code) {
	return 1;
}

static inline BYTE rar(BYTE* code) {
	return 1;
}

static inline BYTE push(BYTE* code) {
	return 1;
}

static inline BYTE pop(BYTE* code) {
	return 1;
}

static inline BYTE dad(BYTE* code) {
	return 1;
}

static inline BYTE inx(BYTE* code) {
	return 1;
}

static inline BYTE dcx(BYTE* code) {
	return 1;
}

static inline BYTE xchg(BYTE* code) {
	return 1;
}

static inline BYTE xthl(BYTE* code) {
	return 1;
}

static inline BYTE sphl(BYTE* code) {
	return 1;
}

static inline BYTE mvi(BYTE* code) {
	BYTE reg = (OPCODE(code) & 0b00111000) >> 3;
	(reg != REG_M) ? (i8080.reg_b[reg] = ARG1(code)) : (memory[i8080.HL] = ARG1(code));
	return 2;
}

static inline BYTE adi(BYTE* code) {
	return 2;
}

static inline BYTE aci(BYTE* code) {
	return 2;
}

static inline BYTE sui(BYTE* code) {
	return 2;
}

static inline BYTE sbi(BYTE* code) {
	return 2;
}

static inline BYTE ani(BYTE* code) {
	return 2;
}

static inline BYTE xri(BYTE* code) {
	return 2;
}

static inline BYTE ori(BYTE* code) {
	return 2;
}

static inline BYTE cpi(BYTE* code) {
	return 2;
}

static inline BYTE sta(BYTE* code) {
	return 3;
}

static inline BYTE lda(BYTE* code) {
	return 3;
}

static inline BYTE shld(BYTE* code) {
	return 3;
}

static inline BYTE lhld(BYTE* code) {
	return 3;
}

static inline BYTE jmp(BYTE* code) {
	i8080.PC = MAKEWORD(ARG1(code), ARG2(code));
	return 0;
}

static inline BYTE jc(BYTE* code) {
	return i8080.status.C ? jmp(code) : 3;
}

static inline BYTE jnc(BYTE* code) {
	return 3;
}

static inline BYTE jz(BYTE* code) {
	return 3;
}

static inline BYTE jnz(BYTE* code) {
	return 3;
}

static inline BYTE jm(BYTE* code) {
	return 3;
}

static inline BYTE jp(BYTE* code) {
	return 3;
}

static inline BYTE jpe(BYTE* code) {
	return 3;
}

static inline BYTE jpo(BYTE* code) {
	return 3;
}

static inline BYTE call(BYTE* code) {
	i8080.SP -= 2;
	memory[i8080.SP] = i8080.PC + 3;
	return jmp(code);
}

static inline BYTE cc(BYTE* code) {
	return i8080.status.C ? call(code) : 3;
}

static inline BYTE cnc(BYTE* code) {
	return 3;
}

static inline BYTE cz(BYTE* code) {
	return 3;
}

static inline BYTE cnz(BYTE* code) {
	return 3;
}

static inline BYTE cm(BYTE* code) {
	return 3;
}

static inline BYTE cp(BYTE* code) {
	return 3;
}

static inline BYTE cpe(BYTE* code) {
	return 3;
}

static inline BYTE cpo(BYTE* code) {
	return 3;
}

static inline BYTE ret(BYTE* code) {
	i8080.PC = memory[i8080.SP];
	i8080.SP += 2;
	return 0;
}

static inline BYTE rc(BYTE* code) {
	return i8080.status.C ? ret(code) : 1;
}

static inline BYTE rnc(BYTE* code) {
	return 1;
}

static inline BYTE rz(BYTE* code) {
	return 1;
}

static inline BYTE rnz(BYTE* code) {
	return 1;
}

static inline BYTE rm(BYTE* code) {
	return 1;
}

static inline BYTE rp(BYTE* code) {
	return 1;
}

static inline BYTE rpe(BYTE* code) {
	return 1;
}

static inline BYTE rpo(BYTE* code) {
	return 1;
}

static inline BYTE rst(BYTE* code) {
	return 1;
}

static inline BYTE ei(BYTE* code) {
	return 1;
}

static inline BYTE di(BYTE* code) {
	return 1;
}

static inline BYTE in(BYTE* code) {
	return 2;
}

static inline BYTE out(BYTE* code) {
	return 2;
}

static inline BYTE hlt(BYTE* code) {
	return 1;
}

const static INSTRUCTION OPCODE_TABLE[256] = {
	NULL, NULL, NULL, NULL, inr, dcr, mvi, NULL, // 0x00 - 0x07
	NULL, NULL, NULL, NULL, inr, dcr, mvi, NULL, // 0x08 - 0x0F
	NULL, NULL, NULL, NULL, inr, dcr, mvi, NULL, // 0x10 - 0x17
	NULL, NULL, NULL, NULL, inr, dcr, mvi, NULL, // 0x18 - 0x1F
	NULL, NULL, NULL, NULL, inr, dcr, mvi, NULL, // 0x20 - 0x27
	NULL, NULL, NULL, NULL, inr, dcr, mvi, NULL, // 0x28 - 0x2F
	NULL, NULL, NULL, NULL, inr, dcr, mvi, NULL, // 0x30 - 0x37
	NULL, NULL, NULL, NULL, inr, dcr, mvi, NULL, // 0x38 - 0x3F
	mov, mov, mov, mov, mov, mov, mov, mov, // 0x40 - 0x47
	mov, mov, mov, mov, mov, mov, mov, mov, // 0x48 - 0x4F
	mov, mov, mov, mov, mov, mov, mov, mov, // 0x50 - 0x57
	mov, mov, mov, mov, mov, mov, mov, mov, // 0x58 - 0x5F
	mov, mov, mov, mov, mov, mov, mov, mov, // 0x60 - 0x67
	mov, mov, mov, mov, mov, mov, mov, mov, // 0x68 - 0x6F
	mov, mov, mov, mov, mov, mov, mov, mov, // 0x70 - 0x77
	mov, mov, mov, mov, mov, mov, mov, mov, // 0x78 - 0x7F
	add, add, add, add, add, add, add, add, // 0x80 - 0x87
	adc, adc, adc, adc, adc, adc, adc, adc, // 0x88 - 0x8F
	sub, sub, sub, sub, sub, sub, sub, sub, // 0x90 - 0x97
	sbb, sbb, sbb, sbb, sbb, sbb, sbb, sbb, // 0x98 - 0x9F
	ana, ana, ana, ana, ana, ana, ana, ana, // 0xA0 - 0xA7
	xra, xra, xra, xra, xra, xra, xra, xra, // 0xA8 - 0xAF
	ora, ora, ora, ora, ora, ora, ora, ora, // 0xB0 - 0xB7
	cmp, cmp, cmp, cmp, cmp, cmp, cmp, cmp, // 0xB8 - 0xBF
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xC0 - 0xC7
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xC8 - 0xCF
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xD0 - 0xD7
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xD8 - 0xDF
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xE0 - 0xE7
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xE8 - 0xEF
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xF0 - 0xF7
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL  // 0xF8 - 0xFF
};
