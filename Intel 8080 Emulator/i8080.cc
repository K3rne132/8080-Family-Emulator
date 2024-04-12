#include <assert.h>
#include "i8080.h"

#define MAKEWORD(low,high) ((WORD)((BYTE)low))|(((WORD)((BYTE)high))<<8)

// returns a current opcode at address of program counter
static BYTE opcode(const INTEL_8080* i8080) {
	return i8080->MEM[i8080->PC];
}

/*
returns a bits from an opcode
example: l = 5, r = 3 will results in [543] bits
l and r cannot be greater than 7 and r < l
*/
static BYTE opcode_bits(
	const INTEL_8080* i8080,
	const BYTE l, // the most significant bit
	const BYTE r  // the least significant bit
) {
	assert(l < 8 && r < 7);
	assert(r < l);
	return (opcode(i8080) & (1 << (l + 1)) - 1) >> r;
}

/*
returns a bits from an accumulator
example: l = 5, r = 3 will results in [543] bits
l and r cannot be greater than 7 and r < l
*/
static BYTE acumulator_bits(
	const INTEL_8080* i8080,
	const BYTE l, // the most significant bit
	const BYTE r  // the least significant bit
) {
	assert(l < 8 && r < 7);
	assert(r < l);
	return (i8080->A & (1 << (l + 1)) - 1) >> r;
}

// returns a current byte argument at address of program counter + 1
static BYTE byte_arg(const INTEL_8080* i8080) {
	return i8080->MEM[i8080->PC + 1];
}

// returns a current word argument at address of program counter + 1
static WORD word_arg(const INTEL_8080* i8080) {
	return i8080->MEM_W[i8080->PC + 1];
}

// returns parity flag for given value
static BOOL parity_check(const BYTE val) {
	// Source: https://stackoverflow.com/a/48041356
	BYTE x = val;
	x ^= x >> 4;
	x ^= x >> 2;
	x ^= x >> 1;
	return (~x) & 1;
}
// returns carry flag for a + b = c
static BOOL pimpek_carry_check(const BYTE a, const BYTE b, const BYTE c) {
	return ((a & 0x80) || (b & 0x80)) && !(c & 0x80) || ((a & 0x80) && (b & 0x80));
}

// returns auxiliary_carry flag for a + b = c
static BOOL auxiliary_pimpek_carry_check(const BYTE a, const BYTE b, const BYTE c) {
	return ((a & 0x8) || (b & 0x8)) && !(c & 0x8) || ((a & 0x8) && (b & 0x8));
}

BYTE cmc(INTEL_8080* i8080) {
	i8080->status.C ^= 1;
	return 1;
}

BYTE stc(INTEL_8080* i8080) {
	i8080->status.C = 1;
	return 1;
}

BYTE inr(INTEL_8080* i8080) {
	BYTE reg = opcode_bits(i8080, 5, 3);
	BYTE value;
	if (reg != REG_M) {
		value = i8080->reg_b[reg]++;
	}
	else {
		value = i8080->MEM[i8080->HL]++;
	}

	i8080->status.Z = (value == 0);
	i8080->status.S = (value & 0x80);
	i8080->status.P = parity_check(value);
	i8080->status.AC = ((value & 0b11111) == 0b10000);

	return 1;
}

BYTE dcr(INTEL_8080* i8080) {
	BYTE reg = opcode_bits(i8080, 5, 3);
	BYTE value;
	if (reg != REG_M) {
		value = i8080->reg_b[reg]--;
	}
	else {
		value = i8080->MEM[i8080->HL]--;
	}
	
	i8080->status.Z = (value == 0);
	i8080->status.S = (value & 0x80);
	i8080->status.P = parity_check(value);
	i8080->status.AC = 0;

	return 1;
}

BYTE cma(INTEL_8080* i8080) {
	i8080->A = ~(i8080->A);
	return 1;
}

BYTE daa(INTEL_8080* i8080) {
	BYTE lacu = acumulator_bits(i8080, 3, 0);
	BYTE aprev = i8080->A & 0xf;
	BYTE addend = (6 * (lacu > 9 || i8080->status.AC));
	i8080->A += addend;
	i8080->status.AC = auxiliary_pimpek_carry_check(aprev, addend, i8080->A);
	aprev = i8080->A;
	BYTE hacu = acumulator_bits(i8080, 7, 4);
	addend = (0x60 * (hacu > 9 || i8080->status.C));
	i8080->A += addend;
	i8080->status.C = pimpek_carry_check(aprev, addend, i8080->A);
	return 1;
}

BYTE stax(INTEL_8080* i8080) {
	BYTE rp = acumulator_bits(i8080, 4, 4);
	(rp) ? (i8080->MEM[REG_PAIR_BC] = i8080->A) : 
		(i8080->MEM[REG_PAIR_DE] = i8080->A);
	return 1;
}

BYTE ldax(INTEL_8080* i8080) {
	BYTE rp = acumulator_bits(i8080, 4, 4);
	(rp) ? (i8080->A = i8080->MEM[REG_PAIR_BC]) :
		(i8080->A = i8080->MEM[REG_PAIR_DE]);
	return 1;
}

BYTE mov(INTEL_8080* i8080) {
	BYTE src = acumulator_bits(i8080, 2, 0);
	BYTE dst = acumulator_bits(i8080, 5, 3);
	if (src == REG_M && dst == REG_M) {
		// Illegal operation
	}
	else if (src != REG_M && dst != REG_M) {
		i8080->reg_b[dst] = i8080->reg_b[src];
	}
	else if (src == REG_M) {
		i8080->reg_b[dst] = i8080->MEM[i8080->HL];
	}
	else if (dst == REG_M) {
		i8080->MEM[i8080->HL] = i8080->reg_b[src];
	}

	return 1;
}

BYTE add(INTEL_8080* i8080) {
	BYTE reg = acumulator_bits(i8080, 2, 0);
	BYTE aprev = i8080->A;
	BYTE addend;
	(reg != REG_M) ? (addend = i8080->reg_b[reg]) :
		(addend = i8080->MEM[i8080->HL]);
	i8080->A += addend;

	i8080->status.C = pimpek_carry_check(aprev, addend, i8080->A);
	i8080->status.Z = (i8080->A == 0);
	i8080->status.S = (i8080->A & 0x80);
	i8080->status.P = parity_check(i8080->A);
	i8080->status.AC = auxiliary_pimpek_carry_check(aprev, addend, i8080->A);
	return 1;
}

BYTE adc(INTEL_8080* i8080) {
	BYTE reg = acumulator_bits(i8080, 2, 0);
	BYTE aprev = i8080->A;
	BYTE addend;
	(reg != REG_M) ? (addend = i8080->reg_b[reg] + i8080->status.C) :
		(addend = i8080->MEM[i8080->HL] + i8080->status.C);
	i8080->A += addend;

	i8080->status.C = pimpek_carry_check(aprev, addend, i8080->A);
	i8080->status.Z = (i8080->A == 0);
	i8080->status.S = (i8080->A & 0x80);
	i8080->status.P = parity_check(i8080->A);
	i8080->status.AC = auxiliary_pimpek_carry_check(aprev, addend, i8080->A);
	return 1;
}

BYTE sub(INTEL_8080* i8080) {
	BYTE reg = acumulator_bits(i8080, 2, 0);
	BYTE aprev = i8080->A;
	BYTE substrahend;
	(reg != REG_M) ? (substrahend = i8080->reg_b[reg]) :
		(substrahend = i8080->MEM[i8080->HL]);
	i8080->A += (~substrahend + 1);

	i8080->status.C = !pimpek_carry_check(aprev, substrahend, i8080->A);
	i8080->status.Z = (i8080->A == 0);
	i8080->status.S = (i8080->A & 0x80);
	i8080->status.P = parity_check(i8080->A);
	i8080->status.AC = auxiliary_pimpek_carry_check(aprev, substrahend, i8080->A);
	return 1;
}

BYTE sbb(INTEL_8080* i8080) {
	BYTE reg = acumulator_bits(i8080, 2, 0);
	BYTE aprev = i8080->A;
	BYTE substrahend;
	(reg != REG_M) ? (substrahend = i8080->reg_b[reg]) :
		(substrahend = i8080->MEM[i8080->HL]);
	substrahend += i8080->status.C;
	i8080->A += (~substrahend + 1);

	i8080->status.C = !pimpek_carry_check(aprev, substrahend, i8080->A);
	i8080->status.Z = (i8080->A == 0);
	i8080->status.S = (i8080->A & 0x80);
	i8080->status.P = parity_check(i8080->A);
	i8080->status.AC = auxiliary_pimpek_carry_check(aprev, substrahend, i8080->A);
	return 1;
}

BYTE ana(INTEL_8080* i8080) {
	BYTE reg = acumulator_bits(i8080, 2, 0);
	(reg != REG_M) ? (i8080->A &= i8080->reg_b[reg]) :
		(i8080->A &= i8080->MEM[i8080->HL]);

	i8080->status.C = 0;
	i8080->status.Z = (i8080->A == 0);
	i8080->status.S = (i8080->A & 0x80);
	i8080->status.P = parity_check(i8080->A);

	return 1;
}

BYTE xra(INTEL_8080* i8080) {
	BYTE reg = acumulator_bits(i8080, 2, 0);
	(reg != REG_M) ? (i8080->A ^= i8080->reg_b[reg]) :
		(i8080->A ^= i8080->MEM[i8080->HL]);

	i8080->status.C = 0;
	i8080->status.Z = (i8080->A == 0);
	i8080->status.S = (i8080->A & 0x80);
	i8080->status.P = parity_check(i8080->A);
	i8080->status.AC = 0;

	return 1;
}

BYTE ora(INTEL_8080* i8080) {
	BYTE reg = acumulator_bits(i8080, 2, 0);
	(reg != REG_M) ? (i8080->A |= i8080->reg_b[reg]) :
		(i8080->A |= i8080->MEM[i8080->HL]);

	i8080->status.C = 0;
	i8080->status.Z = (i8080->A == 0);
	i8080->status.S = (i8080->A & 0x80);
	i8080->status.P = parity_check(i8080->A);

	return 1;
}

BYTE cmp(INTEL_8080* i8080) {
	return 1;
}

BYTE rlc(INTEL_8080* i8080) {
	return 1;
}

BYTE rrc(INTEL_8080* i8080) {
	return 1;
}

BYTE ral(INTEL_8080* i8080) {
	return 1;
}

BYTE rar(INTEL_8080* i8080) {
	return 1;
}

BYTE push(INTEL_8080* i8080) {
	return 1;
}

BYTE pop(INTEL_8080* i8080) {
	return 1;
}

BYTE dad(INTEL_8080* i8080) {
	return 1;
}

BYTE inx(INTEL_8080* i8080) {
	return 1;
}

BYTE dcx(INTEL_8080* i8080) {
	return 1;
}

BYTE xchg(INTEL_8080* i8080) {
	return 1;
}

BYTE xthl(INTEL_8080* i8080) {
	return 1;
}

BYTE sphl(INTEL_8080* i8080) {
	return 1;
}

BYTE mvi(INTEL_8080* i8080) {
	BYTE reg = opcode_bits(i8080, 5, 3);
	(reg != REG_M) ? (i8080->reg_b[reg] = byte_arg(i8080)) :
		(i8080->MEM[i8080->HL] = byte_arg(i8080));
	return 2;
}

BYTE adi(INTEL_8080* i8080) {
	return 2;
}

BYTE aci(INTEL_8080* i8080) {
	return 2;
}

BYTE sui(INTEL_8080* i8080) {
	return 2;
}

BYTE sbi(INTEL_8080* i8080) {
	return 2;
}

BYTE ani(INTEL_8080* i8080) {
	return 2;
}

BYTE xri(INTEL_8080* i8080) {
	return 2;
}

BYTE ori(INTEL_8080* i8080) {
	return 2;
}

BYTE cpi(INTEL_8080* i8080) {
	return 2;
}

BYTE sta(INTEL_8080* i8080) {
	return 3;
}

BYTE lda(INTEL_8080* i8080) {
	return 3;
}

BYTE shld(INTEL_8080* i8080) {
	return 3;
}

BYTE lhld(INTEL_8080* i8080) {
	return 3;
}

BYTE jmp(INTEL_8080* i8080) {
	i8080->PC = word_arg(i8080);
	return 0;
}

BYTE jc(INTEL_8080* i8080) {
	return i8080->status.C ? jmp(i8080) : 3;
}

BYTE jnc(INTEL_8080* i8080) {
	return 3;
}

BYTE jz(INTEL_8080* i8080) {
	return 3;
}

BYTE jnz(INTEL_8080* i8080) {
	return 3;
}

BYTE jm(INTEL_8080* i8080) {
	return 3;
}

BYTE jp(INTEL_8080* i8080) {
	return 3;
}

BYTE jpe(INTEL_8080* i8080) {
	return 3;
}

BYTE jpo(INTEL_8080* i8080) {
	return 3;
}

BYTE call(INTEL_8080* i8080) {
	i8080->SP -= 2;
	i8080->MEM[i8080->SP] = i8080->PC + 3;
	return jmp(i8080);
}

BYTE cc(INTEL_8080* i8080) {
	return i8080->status.C ? call(i8080) : 3;
}

BYTE cnc(INTEL_8080* i8080) {
	return 3;
}

BYTE cz(INTEL_8080* i8080) {
	return 3;
}

BYTE cnz(INTEL_8080* i8080) {
	return 3;
}

BYTE cm(INTEL_8080* i8080) {
	return 3;
}

BYTE cp(INTEL_8080* i8080) {
	return 3;
}

BYTE cpe(INTEL_8080* i8080) {
	return 3;
}

BYTE cpo(INTEL_8080* i8080) {
	return 3;
}

BYTE ret(INTEL_8080* i8080) {
	i8080->PC = i8080->MEM[i8080->SP];
	i8080->SP += 2;
	return 0;
}

BYTE rc(INTEL_8080* i8080) {
	return i8080->status.C ? ret(i8080) : 1;
}

BYTE rnc(INTEL_8080* i8080) {
	return 1;
}

BYTE rz(INTEL_8080* i8080) {
	return 1;
}

BYTE rnz(INTEL_8080* i8080) {
	return 1;
}

BYTE rm(INTEL_8080* i8080) {
	return 1;
}

BYTE rp(INTEL_8080* i8080) {
	return 1;
}

BYTE rpe(INTEL_8080* i8080) {
	return 1;
}

BYTE rpo(INTEL_8080* i8080) {
	return 1;
}

BYTE rst(INTEL_8080* i8080) {
	i8080->SP -= 2;
	i8080->MEM[i8080->SP] = i8080->PC + 1;
	i8080->PC = opcode(i8080) & 0b00111000;
	return 0;
}

BYTE ei(INTEL_8080* i8080) {
	i8080->INT = 1;
	return 1;
}

BYTE di(INTEL_8080* i8080) {
	i8080->INT = 0;
	return 1;
}

BYTE in(INTEL_8080* i8080) {
	i8080->A = i8080->PORT[byte_arg(i8080)];
	return 2;
}

BYTE out(INTEL_8080* i8080) {
	i8080->PORT[byte_arg(i8080)] = i8080->A;
	return 2;
}

BYTE hlt(INTEL_8080* i8080) {
	i8080->HALT = 1;
	return 1;
}
