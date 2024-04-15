#include <assert.h>
#include "i8080.h"

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
	assert(l < 8 && r < 8);
	assert(r <= l);
	return (opcode(i8080) & (1 << (l + 1)) - 1) >> r;
}

// returns a current byte argument at address of program counter + 1
static BYTE byte_arg(const INTEL_8080* i8080) {
	return i8080->MEM[i8080->PC + 1];
}

// returns a current word argument at address of program counter + 1
static WORD word_arg(const INTEL_8080* i8080) {
	WORD result = i8080->MEM[i8080->PC + 2] << 8;
	result |= i8080->MEM[i8080->PC + 1];
	return result;
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

// returns flag register based on extended arithmetic/logical operation result
static void set_flags(
	INTEL_8080* i8080,
	const DWORD result_with_carry,
	const BOOL c, // carry affected
	const BOOL z, // zero affected
	const BOOL s, // sign affected
	const BOOL p // parity affected
) {
	if (c) i8080->status.C = result_with_carry >> 8;
	if (z) i8080->status.Z = ((result_with_carry & 0xFF) == 0);
	if (s) i8080->status.S = ((result_with_carry & 0xFF) >= 0x80);
	if (p) i8080->status.P = parity_check(result_with_carry & 0xFF);
}

// writes word at memory address pointed by SP
void write_word_on_stack(INTEL_8080* i8080, WORD word) {
	i8080->MEM[i8080->SP + 1] = word >> 8;
	i8080->MEM[i8080->SP] = (BYTE)word;
}

// read word from memory address pointed by SP
WORD read_word_from_stack(INTEL_8080* i8080) {
	WORD result = i8080->MEM[i8080->SP + 1] << 8;
	result |= i8080->MEM[i8080->SP];
	return result;
}

BYTE cmc(INTEL_8080* i8080) {
	i8080->status.C ^= 1;
	return 1;
}

BYTE stc(INTEL_8080* i8080) {
	i8080->status.C = SET;
	return 1;
}

BYTE inr(INTEL_8080* i8080) {
	BYTE reg = opcode_bits(i8080, 5, 3);
	DWORD carry = 0;
	carry = (reg != REG_M) ? (i8080->REG[le_reg(reg)]++) : (i8080->MEM[i8080->HL]++);
	carry++;
	set_flags(i8080, carry, 0, 1, 1, 1);
	i8080->status.AC = ((carry & 0xF) == 0);
	return 1;
}

BYTE dcr(INTEL_8080* i8080) {
	BYTE reg = opcode_bits(i8080, 5, 3);
	DWORD carry = 0;
	carry = (reg != REG_M) ? (i8080->REG[le_reg(reg)]--) : (i8080->MEM[i8080->HL]--);
	carry--;
	set_flags(i8080, carry, 0, 1, 1, 1);
	i8080->status.AC = ((carry & 0xF) == 0xF);
	return 1;
}

BYTE cma(INTEL_8080* i8080) {
	i8080->A = ~(i8080->A);
	return 1;
}

BYTE daa(INTEL_8080* i8080) {
	BYTE lsb = i8080->A & 0x0F;
	if (lsb > 9 || i8080->status.AC) {
		i8080->A += 0b00000110;
		i8080->status.AC = 1;
	}
	BYTE msb = (i8080->A & 0xF0) >> 4;
	if (msb > 9 || i8080->status.C) {
		i8080->A += 0b01100000;
		i8080->status.C = 1;
	}
	set_flags(i8080, i8080->A, 0, 1, 1, 1);
	return 1;
}

BYTE nop(INTEL_8080* i8080) {
	return 1;
}

BYTE mov(INTEL_8080* i8080) {
	BYTE src = opcode_bits(i8080, 2, 0);
	BYTE dst = opcode_bits(i8080, 5, 3);
	if (src != REG_M && dst != REG_M)
		i8080->REG[le_reg(dst)] = i8080->REG[le_reg(src)];
	else if (src == REG_M)
		i8080->REG[le_reg(dst)] = i8080->MEM[i8080->HL];
	else if (dst == REG_M)
		i8080->MEM[i8080->HL] = i8080->REG[le_reg(src)];
	else
		assert(NULL);
	return 1;
}

BYTE stax(INTEL_8080* i8080) {
	BYTE x = opcode_bits(i8080, 4, 4);
	(x) ? (i8080->MEM[i8080->DE] = i8080->A) :
		(i8080->MEM[i8080->BC] = i8080->A);
	return 1;
}

BYTE ldax(INTEL_8080* i8080) {
	BYTE x = opcode_bits(i8080, 4, 4);
	i8080->A = (x) ? (i8080->MEM[i8080->DE]) : (i8080->MEM[i8080->BC]);
	return 1;
}

BYTE add(INTEL_8080* i8080) {
	DWORD carry = i8080->A;
	BYTE reg = opcode_bits(i8080, 2, 0);
	DWORD num = (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	carry += num;
	set_flags(i8080, carry, 1, 1, 1, 1);
	i8080->status.AC = (((i8080->A & 0x0F) + (num & 0xF)) > 0xF);
	i8080->A = carry;
	return 1;
}

BYTE adc(INTEL_8080* i8080) {
	DWORD carry = i8080->A + i8080->status.C;
	BYTE reg = opcode_bits(i8080, 2, 0);
	DWORD num = (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	carry += num;
	set_flags(i8080, carry, 1, 1, 1, 1);
	i8080->status.AC = (((i8080->A & 0x0F) + (num & 0xF) + i8080->status.C) > 0xF);
	i8080->A = carry;
	return 1;
}

BYTE sub(INTEL_8080* i8080) {
	DWORD carry = i8080->A;
	BYTE reg = opcode_bits(i8080, 2, 0);
	DWORD num = (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	carry -= num;
	set_flags(i8080, carry, 1, 1, 1, 1);
	i8080->status.AC = (((i8080->A & 0x0F) - (num & 0xF)) > 0x0F);
	i8080->A = carry;
	return 1;
}

BYTE sbb(INTEL_8080* i8080) {
	DWORD carry = i8080->A - i8080->status.C;
	BYTE reg = opcode_bits(i8080, 2, 0);
	DWORD num = (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	carry -= num;
	set_flags(i8080, carry, 1, 1, 1, 1);
	i8080->status.AC = (((i8080->A & 0x0F) - (num & 0xF) + i8080->status.C) > 0xF);
	i8080->A = carry;
	return 1;
}

BYTE ana(INTEL_8080* i8080) {
	BYTE reg = opcode_bits(i8080, 2, 0);
	i8080->A &= (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	set_flags(i8080, i8080->A, 1, 1, 1, 1);
	return 1;
}

BYTE xra(INTEL_8080* i8080) {
	BYTE reg = opcode_bits(i8080, 2, 0);
	i8080->A ^= (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	set_flags(i8080, i8080->A, 1, 1, 1, 1);
	return 1;
}

BYTE ora(INTEL_8080* i8080) {
	BYTE reg = opcode_bits(i8080, 2, 0);
	i8080->A |= (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	set_flags(i8080, i8080->A, 1, 1, 1, 1);
	return 1;
}

BYTE cmp(INTEL_8080* i8080) {
	DWORD carry = i8080->A;
	BYTE reg = opcode_bits(i8080, 2, 0);
	DWORD num = (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	carry -= num;
	set_flags(i8080, carry, 1, 1, 1, 1);
	i8080->status.AC = (((i8080->A & 0x0F) - (num & 0xF)) > 0xF);
	return 1;
}

BYTE rlc(INTEL_8080* i8080) {
	i8080->status.C = i8080->A >> 7;
	i8080->A <<= 1;
	i8080->A |= i8080->status.C;
	return 1;
}

BYTE rrc(INTEL_8080* i8080) {
	i8080->status.C = i8080->A;
	i8080->A >>= 1;
	i8080->A |= (i8080->status.C << 7);
	return 1;
}

BYTE ral(INTEL_8080* i8080) {
	BYTE new_carry = i8080->A >> 7;
	i8080->A <<= 1;
	i8080->A |= i8080->status.C;
	i8080->status.C = new_carry;
	return 1;
}

BYTE rar(INTEL_8080* i8080) {
	BYTE new_carry = i8080->A;
	i8080->A >>= 1;
	i8080->A |= (i8080->status.C << 7);
	i8080->status.C = new_carry;
	return 1;
}

BYTE push(INTEL_8080* i8080) {
	i8080->F = (i8080->F & 0xD7) | 0x02;
	i8080->SP -= 2;
	write_word_on_stack(i8080, i8080->REG_W[opcode_bits(i8080, 5, 4)]);
	return 1;
}

BYTE pop(INTEL_8080* i8080) {
	i8080->REG_W[opcode_bits(i8080, 5, 4)] = read_word_from_stack(i8080);
	i8080->SP += 2;
	return 1;
}

BYTE dad(INTEL_8080* i8080) {
	DWORD carry = i8080->HL;
	BYTE pair = opcode_bits(i8080, 5, 4);
	carry += (pair != REG_PAIR_SP) ? (i8080->REG_W[pair]) : (i8080->SP);
	i8080->status.C = carry >> 16;
	i8080->HL = carry;
	return 1;
}

BYTE inx(INTEL_8080* i8080) {
	BYTE pair = opcode_bits(i8080, 5, 4);
	(pair != REG_PAIR_SP) ? (i8080->REG_W[pair]++) : (i8080->SP++);
	return 1;
}

BYTE dcx(INTEL_8080* i8080) {
	BYTE pair = opcode_bits(i8080, 5, 4);
	(pair != REG_PAIR_SP) ? (i8080->REG_W[pair]--) : (i8080->SP--);
	return 1;
}

BYTE xchg(INTEL_8080* i8080) {
	WORD tmp = i8080->DE;
	i8080->DE = i8080->HL;
	i8080->HL = tmp;
	return 1;
}

BYTE xthl(INTEL_8080* i8080) {
	BYTE tmp = i8080->L;
	i8080->L = i8080->MEM[i8080->SP];
	i8080->MEM[i8080->SP] = tmp;
	tmp = i8080->H;
	i8080->H = i8080->MEM[i8080->SP + 1];
	i8080->MEM[i8080->SP + 1] = tmp;
	return 1;
}

BYTE sphl(INTEL_8080* i8080) {
	i8080->SP = i8080->HL;
	return 1;
}

BYTE lxi(INTEL_8080* i8080) {
	BYTE pair = opcode_bits(i8080, 5, 4);
	(pair != REG_PAIR_SP) ? (i8080->REG_W[pair] = word_arg(i8080)) :
		(i8080->SP = word_arg(i8080));
	return 3;
}

BYTE mvi(INTEL_8080* i8080) {
	BYTE reg = opcode_bits(i8080, 5, 3);
	(reg != REG_M) ? (i8080->REG[le_reg(reg)] = byte_arg(i8080)) :
		(i8080->MEM[i8080->HL] = byte_arg(i8080));
	return 2;
}

BYTE adi(INTEL_8080* i8080) {
	DWORD arg = byte_arg(i8080);
	DWORD carry = i8080->A + arg;
	set_flags(i8080, carry, 1, 1, 1, 1);
	i8080->status.AC = (((i8080->A & 0x0F) + (arg & 0xF)) > 0xF);
	i8080->A = carry;
	return 2;
}

BYTE aci(INTEL_8080* i8080) {
	DWORD arg = byte_arg(i8080);
	DWORD carry = i8080->A + arg + i8080->status.C;
	set_flags(i8080, carry, 1, 1, 1, 1);
	i8080->status.AC = (((i8080->A & 0x0F) + (arg & 0xF) + i8080->status.C) > 0xF);
	i8080->A = carry;
	return 2;
}

BYTE sui(INTEL_8080* i8080) {
	DWORD arg = byte_arg(i8080);
	DWORD carry = i8080->A - arg;
	set_flags(i8080, carry, 1, 1, 1, 1);
	i8080->status.AC = (((i8080->A & 0x0F) - (arg & 0xF)) > 0xF);
	i8080->A = carry;
	return 2;
}

BYTE sbi(INTEL_8080* i8080) {
	DWORD arg = byte_arg(i8080);
	DWORD carry = i8080->A - arg - i8080->status.C;
	set_flags(i8080, carry, 1, 1, 1, 1);
	i8080->status.AC = (((i8080->A & 0x0F) - (arg & 0xF) + i8080->status.C) > 0xF);
	i8080->A = carry;
	return 2;
}

BYTE ani(INTEL_8080* i8080) {
	i8080->A &= byte_arg(i8080);
	set_flags(i8080, i8080->A, 1, 1, 1, 1);
	return 2;
}

BYTE xri(INTEL_8080* i8080) {
	i8080->A ^= byte_arg(i8080);
	set_flags(i8080, i8080->A, 1, 1, 1, 1);
	return 2;
}

BYTE ori(INTEL_8080* i8080) {
	i8080->A |= byte_arg(i8080);
	set_flags(i8080, i8080->A, 1, 1, 1, 1);
	return 2;
}

BYTE cpi(INTEL_8080* i8080) {
	DWORD arg = byte_arg(i8080);
	DWORD carry = i8080->A - arg;
	set_flags(i8080, carry, 1, 1, 1, 1);
	i8080->status.AC = (((i8080->A & 0x0F) - (arg & 0xF)) > 0xF);
	return 2;
}

BYTE sta(INTEL_8080* i8080) {
	i8080->MEM[word_arg(i8080)] = i8080->A;
	return 3;
}

BYTE lda(INTEL_8080* i8080) {
	i8080->A = i8080->MEM[word_arg(i8080)];
	return 3;
}

BYTE shld(INTEL_8080* i8080) {
	i8080->MEM[word_arg(i8080)] = i8080->L;
	i8080->MEM[word_arg(i8080) + 1] = i8080->H;
	return 3;
}

BYTE lhld(INTEL_8080* i8080) {
	i8080->L = i8080->MEM[word_arg(i8080)];
	i8080->H = i8080->MEM[word_arg(i8080) + 1];
	return 3;
}

BYTE pchl(INTEL_8080* i8080) {
	i8080->PC = i8080->REG_W[REG_PAIR_HL];
	return 0;
}

BYTE jmp(INTEL_8080* i8080) {
	i8080->PC = word_arg(i8080);
	return 0;
}

BYTE jc(INTEL_8080* i8080) {
	return i8080->status.C ? jmp(i8080) : 3;
}

BYTE jnc(INTEL_8080* i8080) {
	return !i8080->status.C ? jmp(i8080) : 3;
}

BYTE jz(INTEL_8080* i8080) {
	return i8080->status.Z ? jmp(i8080) : 3;
}

BYTE jnz(INTEL_8080* i8080) {
	return !i8080->status.Z ? jmp(i8080) : 3;
}

BYTE jm(INTEL_8080* i8080) {
	return i8080->status.S ? jmp(i8080) : 3;
}

BYTE jp(INTEL_8080* i8080) {
	return !i8080->status.S ? jmp(i8080) : 3;
}

BYTE jpe(INTEL_8080* i8080) {
	return i8080->status.P ? jmp(i8080) : 3;
}

BYTE jpo(INTEL_8080* i8080) {
	return !i8080->status.P ? jmp(i8080) : 3;
}

BYTE call(INTEL_8080* i8080) {
	i8080->SP -= 2;
	write_word_on_stack(i8080, i8080->PC + 3);
	return jmp(i8080);
}

BYTE cc(INTEL_8080* i8080) {
	return i8080->status.C ? call(i8080) : 3;
}

BYTE cnc(INTEL_8080* i8080) {
	return !i8080->status.C ? call(i8080) : 3;
}

BYTE cz(INTEL_8080* i8080) {
	return i8080->status.Z ? call(i8080) : 3;
}

BYTE cnz(INTEL_8080* i8080) {
	return !i8080->status.Z ? call(i8080) : 3;
}

BYTE cm(INTEL_8080* i8080) {
	return i8080->status.S ? call(i8080) : 3;
}

BYTE cp(INTEL_8080* i8080) {
	return !i8080->status.S ? call(i8080) : 3;
}

BYTE cpe(INTEL_8080* i8080) {
	return i8080->status.P ? call(i8080) : 3;
}

BYTE cpo(INTEL_8080* i8080) {
	return !i8080->status.P ? call(i8080) : 3;
}

BYTE ret(INTEL_8080* i8080) {
	i8080->PC = read_word_from_stack(i8080);
	i8080->SP += 2;
	return 0;
}

BYTE rc(INTEL_8080* i8080) {
	return i8080->status.C ? ret(i8080) : 1;
}

BYTE rnc(INTEL_8080* i8080) {
	return !i8080->status.C ? ret(i8080) : 1;
}

BYTE rz(INTEL_8080* i8080) {
	return i8080->status.Z ? ret(i8080) : 1;
}

BYTE rnz(INTEL_8080* i8080) {
	return !i8080->status.Z ? ret(i8080) : 1;
}

BYTE rm(INTEL_8080* i8080) {
	return i8080->status.S ? ret(i8080) : 1;
}

BYTE rp(INTEL_8080* i8080) {
	return !i8080->status.S ? ret(i8080) : 1;
}

BYTE rpe(INTEL_8080* i8080) {
	return i8080->status.P ? ret(i8080) : 1;
}

BYTE rpo(INTEL_8080* i8080) {
	return !i8080->status.P ? ret(i8080) : 1;
}

BYTE rst(INTEL_8080* i8080) {
	i8080->SP -= 2;
	write_word_on_stack(i8080, i8080->PC + 1);
	i8080->PC = opcode(i8080) & 0b00111000;
	return 0;
}

BYTE ei(INTEL_8080* i8080) {
	i8080->INT = SET;
	return 1;
}

BYTE di(INTEL_8080* i8080) {
	i8080->INT = RESET;
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
	i8080->HALT = SET;
	return 1;
}
