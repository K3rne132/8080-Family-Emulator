#include <assert.h>
#include <stddef.h>
#include "i8080.h"

// returns a current opcode at address of program counter
uint8_t opcode(const INTEL_8080* i8080) {
	return i8080->MEM[i8080->PC];
}

/*
returns a bits from an opcode
example: l = 5, r = 3 will results in [543] bits
l and r cannot be greater than 7 and r < l
*/
static uint8_t opcode_bits(
	const INTEL_8080* i8080,
	const uint8_t l, // the most significant bit
	const uint8_t r  // the least significant bit
) {
	assert(l < 8 && r < 8);
	assert(r <= l);
	return (opcode(i8080) & (1 << (l + 1)) - 1) >> r;
}

// returns a current uint8_t argument at address of program counter + 1
uint8_t uint8_t_arg(const INTEL_8080* i8080) {
	return i8080->MEM[i8080->PC + 1];
}

// returns a current uint16_t argument at address of program counter + 1
uint16_t uint16_t_arg(const INTEL_8080* i8080) {
	uint16_t result = i8080->MEM[i8080->PC + 2] << 8;
	result |= i8080->MEM[i8080->PC + 1];
	return result;
}

static uint8_t alu_add(
	INTEL_8080* i8080,
	const uint8_t arg1,
	const uint8_t arg2,
	const uint8_t carry
) {
	uint32_t result = arg1 + arg2 + carry;
	set_ZSP_flags(i8080, result);
	i8080->status.C = result >> 8;
	i8080->status.AC = (((result ^ arg1 ^ arg2) & 0x10) != 0);
	set_V_flag_int8(i8080, arg1, arg2, result);
	set_UI_flag_int8(i8080);
	set_B_flag(i8080, SET);
	return result;
}

static uint8_t alu_sub(
	INTEL_8080* i8080,
	const uint8_t arg1,
	const uint8_t arg2,
	const uint8_t carry
) {
	uint32_t result = alu_add(i8080, arg1, ~arg2, 1 - carry);
	i8080->status.C = 1 - i8080->status.C;
	set_B_flag(i8080, RESET);
	return result;
}

// writes uint16_t at memory address pointed by SP
void write_uint16_t_on_stack(INTEL_8080* i8080, uint16_t uint16_t) {
	i8080->MEM[i8080->SP + 1] = uint16_t >> 8;
	i8080->MEM[i8080->SP] = (uint8_t)uint16_t;
}

// read uint16_t from memory address pointed by SP
uint16_t read_uint16_t_from_stack(INTEL_8080* i8080) {
	uint16_t result = i8080->MEM[i8080->SP + 1] << 8;
	result |= i8080->MEM[i8080->SP];
	return result;
}



uint16_t cmc(INTEL_8080* i8080) {
	i8080->status.C ^= 1;
	return MAKERESULT(1, 4);
}

uint16_t stc(INTEL_8080* i8080) {
	i8080->status.C = SET;
	return MAKERESULT(1, 4);
}

uint16_t inr(INTEL_8080* i8080) {
	uint8_t reg = opcode_bits(i8080, 5, 3);
	uint32_t carry = 0;
	carry = (reg != REG_M) ? (++i8080->REG[le_reg(reg)]) : (++i8080->MEM[i8080->HL]);
	set_ZSP_flags(i8080, carry);
	set_V_flag_int8(i8080, carry - 1, 1, carry);
	set_UI_flag_int8(i8080);
	set_B_flag(i8080, RESET);
	i8080->status.AC = ((carry & 0x0F) == 0);
	return MAKERESULT(1, (reg != REG_M ? 5 + CLK_LESS : 10));
}

uint16_t dcr(INTEL_8080* i8080) {
	uint8_t reg = opcode_bits(i8080, 5, 3);
	uint32_t carry = 0;
	carry = (reg != REG_M) ? (--i8080->REG[le_reg(reg)]) : (--i8080->MEM[i8080->HL]);
	set_ZSP_flags(i8080, carry);
	set_V_flag_int8(i8080, carry + 1, -1, carry);
	set_UI_flag_int8(i8080);
	set_B_flag(i8080, SET);
	i8080->status.AC = !((carry & 0x0F) == 0x0F);
	return MAKERESULT(1, (reg != REG_M ? 5 + CLK_LESS : 10));
}

uint16_t cma(INTEL_8080* i8080) {
	i8080->A = ~(i8080->A);
	return MAKERESULT(1, 4);
}

uint16_t daa(INTEL_8080* i8080) {
	uint8_t old_c = i8080->status.C;
	uint8_t to_add = 0;
	uint8_t lsb = i8080->A & 0x0F;
	uint8_t msb = (i8080->A & 0xF0) >> 4;
	if (lsb > 9 || i8080->status.AC)
		to_add += 0x06;
	if (msb > 9 || i8080->status.C || (msb >= 9 && lsb > 9)) {
		to_add += 0x60;
		old_c = 1;
	}
	uint8_t old_b = RESET;
#ifdef E_NEC8080
	old_b = i8080->status.B;
#endif // E_NEC8080
	uint8_t result = alu_add(i8080, i8080->A, to_add, 0);
	set_V_flag_int8(i8080, i8080->A, to_add, result);
	set_UI_flag_int8(i8080);
	set_B_flag(i8080, old_b); // restore SUB bit for DAA
	i8080->A = result;
	i8080->status.C = old_c;
	return MAKERESULT(1, 4);
}

uint16_t nop(INTEL_8080* i8080) {
	return MAKERESULT(1, 4);
}

uint16_t mov(INTEL_8080* i8080) {
	uint8_t src = opcode_bits(i8080, 2, 0);
	uint8_t dst = opcode_bits(i8080, 5, 3);
	if (src != REG_M && dst != REG_M)
		i8080->REG[le_reg(dst)] = i8080->REG[le_reg(src)];
	else if (src == REG_M)
		i8080->REG[le_reg(dst)] = i8080->MEM[i8080->HL];
	else if (dst == REG_M)
		i8080->MEM[i8080->HL] = i8080->REG[le_reg(src)];
	else
		assert(NULL);
	return MAKERESULT(1, ((src == REG_M || dst == REG_M) ? 7 : 5 + MOV_R_R_DIFF));
}

uint16_t stax(INTEL_8080* i8080) {
	uint8_t x = opcode_bits(i8080, 4, 4);
	(x) ? (i8080->MEM[i8080->DE] = i8080->A) :
		(i8080->MEM[i8080->BC] = i8080->A);
	return MAKERESULT(1, 7);
}

uint16_t ldax(INTEL_8080* i8080) {
	uint8_t x = opcode_bits(i8080, 4, 4);
	i8080->A = (x) ? (i8080->MEM[i8080->DE]) : (i8080->MEM[i8080->BC]);
	return MAKERESULT(1, 7);
}

uint16_t add(INTEL_8080* i8080) {
	uint8_t reg = opcode_bits(i8080, 2, 0);
	uint32_t num = (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	i8080->A = alu_add(i8080, i8080->A, num, 0);
	return MAKERESULT(1, (reg != REG_M ? 4 : 7));
}

uint16_t adc(INTEL_8080* i8080) {
	uint8_t reg = opcode_bits(i8080, 2, 0);
	uint32_t num = (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	i8080->A = alu_add(i8080, i8080->A, num, i8080->status.C);
	return MAKERESULT(1, (reg != REG_M ? 4 : 7));
}

uint16_t sub(INTEL_8080* i8080) {
	uint8_t reg = opcode_bits(i8080, 2, 0);
	uint32_t num = (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	i8080->A = alu_sub(i8080, i8080->A, num, 0);
	return MAKERESULT(1, (reg != REG_M ? 4 : 7));
}

uint16_t sbb(INTEL_8080* i8080) {
	uint8_t reg = opcode_bits(i8080, 2, 0);
	uint32_t num = (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	i8080->A = alu_sub(i8080, i8080->A, num, i8080->status.C);
	return MAKERESULT(1, (reg != REG_M ? 4 : 7));
}

uint16_t ana(INTEL_8080* i8080) {
	uint8_t reg = opcode_bits(i8080, 2, 0);
	uint8_t num = (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	i8080->status.C = RESET;
#ifdef E_AM9080
	i8080->status.AC = RESET;
#else
	i8080->status.AC = (((i8080->A | num) & 0x08) != 0);
#endif
	i8080->A &= num;
	set_ZSP_flags(i8080, i8080->A);
	set_V_flag_int8(i8080, 0, 0, 0);
	set_UI_flag_int8(i8080);
	set_B_flag(i8080, RESET);
	return MAKERESULT(1, (reg != REG_M ? 4 : 7));
}

uint16_t xra(INTEL_8080* i8080) {
	uint8_t reg = opcode_bits(i8080, 2, 0);
	i8080->A ^= (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	set_ZSP_flags(i8080, i8080->A);
	set_V_flag_int8(i8080, 0, 0, 0);
	set_UI_flag_int8(i8080);
	set_B_flag(i8080, RESET);
	i8080->status.C = RESET;
	i8080->status.AC = RESET;
	return MAKERESULT(1, (reg != REG_M ? 4 : 7));
}

uint16_t ora(INTEL_8080* i8080) {
	uint8_t reg = opcode_bits(i8080, 2, 0);
	i8080->A |= (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	set_ZSP_flags(i8080, i8080->A);
	set_V_flag_int8(i8080, 0, 0, 0);
	set_UI_flag_int8(i8080);
	set_B_flag(i8080, RESET);
	i8080->status.C = RESET;
	i8080->status.AC = RESET;
	return MAKERESULT(1, (reg != REG_M ? 4 : 7));
}

uint16_t cmp(INTEL_8080* i8080) {
	uint8_t reg = opcode_bits(i8080, 2, 0);
	uint32_t num = (reg != REG_M) ? (i8080->REG[le_reg(reg)]) : (i8080->MEM[i8080->HL]);
	alu_sub(i8080, i8080->A, num, 0);
	return MAKERESULT(1, (reg != REG_M ? 4 : 7));
}

uint16_t rlc(INTEL_8080* i8080) {
	i8080->status.C = i8080->A >> 7;
	uint8_t old_a = i8080->A;
	i8080->A <<= 1;
	i8080->A |= i8080->status.C;
	set_V_flag_int8(i8080, old_a, old_a, i8080->A);
	set_B_flag(i8080, RESET);
	return MAKERESULT(1, 4);
}

uint16_t rrc(INTEL_8080* i8080) {
	i8080->status.C = i8080->A;
	i8080->A >>= 1;
	i8080->A |= (i8080->status.C << 7);
	set_V_flag_int8(i8080, 0, 0, 0);
	set_B_flag(i8080, RESET);
	return MAKERESULT(1, 4);
}

uint16_t ral(INTEL_8080* i8080) {
	uint8_t new_carry = i8080->A >> 7;
	uint8_t old_a = i8080->A;
	i8080->A <<= 1;
	i8080->A |= i8080->status.C;
	i8080->status.C = new_carry;
	set_V_flag_int8(i8080, old_a, old_a, i8080->A);
	set_B_flag(i8080, RESET);
	return MAKERESULT(1, 4);
}

uint16_t rar(INTEL_8080* i8080) {
	uint8_t new_carry = i8080->A;
	i8080->A >>= 1;
	i8080->A |= (i8080->status.C << 7);
	i8080->status.C = new_carry;
	set_V_flag_int8(i8080, 0, 0, 0);
	set_B_flag(i8080, RESET);
	return MAKERESULT(1, 4);
}

uint16_t push(INTEL_8080* i8080) {
	i8080->F = (i8080->F & 0xD7) | 0x02;
	i8080->SP -= 2;
	write_uint16_t_on_stack(i8080, i8080->REG_W[opcode_bits(i8080, 5, 4)]);
	return MAKERESULT(1, 11);
}

uint16_t pop(INTEL_8080* i8080) {
	i8080->REG_W[opcode_bits(i8080, 5, 4)] = read_uint16_t_from_stack(i8080);
	i8080->SP += 2;
	return MAKERESULT(1, 10);
}

uint16_t dad(INTEL_8080* i8080) {
	uint32_t carry = i8080->HL;
	uint8_t pair = opcode_bits(i8080, 5, 4);
	uint16_t to_add = (pair != REG_PAIR_SP) ? (i8080->REG_W[pair]) : (i8080->SP);
	carry += to_add;
	i8080->status.C = carry >> 16;
	i8080->HL = carry;
	set_V_flag_int16(i8080, i8080->HL, to_add, i8080->HL);
	set_B_flag(i8080, SET);
	return MAKERESULT(1, 10);
}

uint16_t inx(INTEL_8080* i8080) {
	uint8_t pair = opcode_bits(i8080, 5, 4);
	uint16_t* to_inx = (pair != REG_PAIR_SP) ? (&i8080->REG_W[pair]) : (&i8080->SP);
	(*to_inx)++;
#ifdef E_I8085
	i8080->status.U = (*to_inx == 0x8000);
#endif
	set_B_flag(i8080, SET);
	return MAKERESULT(1, 5 + CLK_MORE);
}

uint16_t dcx(INTEL_8080* i8080) {
	uint8_t pair = opcode_bits(i8080, 5, 4);
	uint16_t* to_dcx = (pair != REG_PAIR_SP) ? (&i8080->REG_W[pair]) : (&i8080->SP);
	(*to_dcx)--;
#ifdef E_I8085
	i8080->status.U = (*to_dcx == 0x7FFF);
#endif
	set_B_flag(i8080, RESET);
	return MAKERESULT(1, 5 + CLK_MORE);
}

uint16_t xchg(INTEL_8080* i8080) {
	uint16_t tmp = i8080->DE;
	i8080->DE = i8080->HL;
	i8080->HL = tmp;
	return MAKERESULT(1, 5);
}

uint16_t xthl(INTEL_8080* i8080) {
	uint8_t tmp = i8080->L;
	i8080->L = i8080->MEM[i8080->SP];
	i8080->MEM[i8080->SP] = tmp;
	tmp = i8080->H;
	i8080->H = i8080->MEM[i8080->SP + 1];
	i8080->MEM[i8080->SP + 1] = tmp;
	return MAKERESULT(1, 18);
}

uint16_t sphl(INTEL_8080* i8080) {
	i8080->SP = i8080->HL;
	return MAKERESULT(1, 5);
}

uint16_t lxi(INTEL_8080* i8080) {
	uint8_t pair = opcode_bits(i8080, 5, 4);
	(pair != REG_PAIR_SP) ? (i8080->REG_W[pair] = uint16_t_arg(i8080)) :
		(i8080->SP = uint16_t_arg(i8080));
	return MAKERESULT(3, 10);
}

uint16_t mvi(INTEL_8080* i8080) {
	uint8_t reg = opcode_bits(i8080, 5, 3);
	(reg != REG_M) ? (i8080->REG[le_reg(reg)] = uint8_t_arg(i8080)) :
		(i8080->MEM[i8080->HL] = uint8_t_arg(i8080));
	return MAKERESULT(2, (reg != REG_M ? 7 : 10));
}

uint16_t adi(INTEL_8080* i8080) {
	i8080->A = alu_add(i8080, i8080->A, uint8_t_arg(i8080), 0);
	return MAKERESULT(2, 7);
}

uint16_t aci(INTEL_8080* i8080) {
	i8080->A = alu_add(i8080, i8080->A, uint8_t_arg(i8080), i8080->status.C);
	return MAKERESULT(2, 7);
}

uint16_t sui(INTEL_8080* i8080) {
	i8080->A = alu_sub(i8080, i8080->A, uint8_t_arg(i8080), 0);
	return MAKERESULT(2, 7);
}

uint16_t sbi(INTEL_8080* i8080) {
	i8080->A = alu_sub(i8080, i8080->A, uint8_t_arg(i8080), i8080->status.C);
	return MAKERESULT(2, 7);
}

uint16_t ani(INTEL_8080* i8080) {
	i8080->status.C = RESET;
#ifdef E_AM9080
	i8080->status.AC = RESET;
#else
	i8080->status.AC = (((i8080->A | uint8_t_arg(i8080)) & 0x08) != 0);
#endif
	i8080->A &= uint8_t_arg(i8080);
	set_ZSP_flags(i8080, i8080->A);
	set_V_flag_int8(i8080, 0, 0, 0);
	set_UI_flag_int8(i8080);
	set_B_flag(i8080, RESET);
	return MAKERESULT(2, 7);
}

uint16_t xri(INTEL_8080* i8080) {
	i8080->A ^= uint8_t_arg(i8080);
	set_ZSP_flags(i8080, i8080->A);
	set_V_flag_int8(i8080, 0, 0, 0);
	set_UI_flag_int8(i8080);
	set_B_flag(i8080, RESET);
	i8080->status.C = RESET;
	i8080->status.AC = RESET;
	return MAKERESULT(2, 7);
}

uint16_t ori(INTEL_8080* i8080) {
	i8080->A |= uint8_t_arg(i8080);
	set_ZSP_flags(i8080, i8080->A);
	set_V_flag_int8(i8080, 0, 0, 0);
	set_UI_flag_int8(i8080);
	set_B_flag(i8080, RESET);
	i8080->status.C = RESET;
	i8080->status.AC = RESET;
	return MAKERESULT(2, 7);
}

uint16_t cpi(INTEL_8080* i8080) {
	alu_sub(i8080, i8080->A, uint8_t_arg(i8080), 0);
	return MAKERESULT(2, 7);
}

uint16_t sta(INTEL_8080* i8080) {
	i8080->MEM[uint16_t_arg(i8080)] = i8080->A;
	return MAKERESULT(3, 13);
}

uint16_t lda(INTEL_8080* i8080) {
	i8080->A = i8080->MEM[uint16_t_arg(i8080)];
	return MAKERESULT(3, 13);
}

uint16_t shld(INTEL_8080* i8080) {
	i8080->MEM[uint16_t_arg(i8080)] = i8080->L;
	i8080->MEM[uint16_t_arg(i8080) + 1] = i8080->H;
	return MAKERESULT(3, 16);
}

uint16_t lhld(INTEL_8080* i8080) {
	i8080->L = i8080->MEM[uint16_t_arg(i8080)];
	i8080->H = i8080->MEM[uint16_t_arg(i8080) + 1];
	return MAKERESULT(3, 16);
}

uint16_t pchl(INTEL_8080* i8080) {
	i8080->PC = i8080->REG_W[REG_PAIR_HL];
	return MAKERESULT(0, 5);
}

uint16_t jmp(INTEL_8080* i8080) {
	i8080->PC = uint16_t_arg(i8080);
	return MAKERESULT(0, 10);
}

uint16_t jc(INTEL_8080* i8080) {
	return i8080->status.C ? jmp(i8080) : MAKERESULT(3, 10 + JMP_DIFF);
}

uint16_t jnc(INTEL_8080* i8080) {
	return !i8080->status.C ? jmp(i8080) : MAKERESULT(3, 10 + JMP_DIFF);
}

uint16_t jz(INTEL_8080* i8080) {
	return i8080->status.Z ? jmp(i8080) : MAKERESULT(3, 10 + JMP_DIFF);
}

uint16_t jnz(INTEL_8080* i8080) {
	return !i8080->status.Z ? jmp(i8080) : MAKERESULT(3, 10 + JMP_DIFF);
}

uint16_t jm(INTEL_8080* i8080) {
	return i8080->status.S ? jmp(i8080) : MAKERESULT(3, 10 + JMP_DIFF);
}

uint16_t jp(INTEL_8080* i8080) {
	return !i8080->status.S ? jmp(i8080) : MAKERESULT(3, 10 + JMP_DIFF);
}

uint16_t jpe(INTEL_8080* i8080) {
	return i8080->status.P ? jmp(i8080) : MAKERESULT(3, 10 + JMP_DIFF);
}

uint16_t jpo(INTEL_8080* i8080) {
	return !i8080->status.P ? jmp(i8080) : MAKERESULT(3, 10 + JMP_DIFF);
}

uint16_t call(INTEL_8080* i8080) {
	i8080->SP -= 2;
	write_uint16_t_on_stack(i8080, i8080->PC + 3);
	jmp(i8080);
	return MAKERESULT(0, 17 + CLK_MORE);
}

uint16_t cc(INTEL_8080* i8080) {
	return i8080->status.C ? call(i8080) : MAKERESULT(3, 11 + CALL_DIFF);
}

uint16_t cnc(INTEL_8080* i8080) {
	return !i8080->status.C ? call(i8080) : MAKERESULT(3, 11 + CALL_DIFF);
}

uint16_t cz(INTEL_8080* i8080) {
	return i8080->status.Z ? call(i8080) : MAKERESULT(3, 11 + CALL_DIFF);
}

uint16_t cnz(INTEL_8080* i8080) {
	return !i8080->status.Z ? call(i8080) : MAKERESULT(3, 11 + CALL_DIFF);
}

uint16_t cm(INTEL_8080* i8080) {
	return i8080->status.S ? call(i8080) : MAKERESULT(3, 11 + CALL_DIFF);
}

uint16_t cp(INTEL_8080* i8080) {
	return !i8080->status.S ? call(i8080) : MAKERESULT(3, 11 + CALL_DIFF);
}

uint16_t cpe(INTEL_8080* i8080) {
	return i8080->status.P ? call(i8080) : MAKERESULT(3, 11 + CALL_DIFF);
}

uint16_t cpo(INTEL_8080* i8080) {
	return !i8080->status.P ? call(i8080) : MAKERESULT(3, 11 + CALL_DIFF);
}

uint16_t ret(INTEL_8080* i8080) {
	i8080->PC = read_uint16_t_from_stack(i8080);
	i8080->SP += 2;
	return MAKERESULT(0, 10);
}

uint16_t rc(INTEL_8080* i8080) {
	return (i8080->status.C ? ret(i8080) : MAKERESULT(1, 4)) + RET_DIFF;
}

uint16_t rnc(INTEL_8080* i8080) {
	return (!i8080->status.C ? ret(i8080) : MAKERESULT(1, 4)) + RET_DIFF;
}

uint16_t rz(INTEL_8080* i8080) {
	return (i8080->status.Z ? ret(i8080) : MAKERESULT(1, 4)) + RET_DIFF;
}

uint16_t rnz(INTEL_8080* i8080) {
	return (!i8080->status.Z ? ret(i8080) : MAKERESULT(1, 4)) + RET_DIFF;
}

uint16_t rm(INTEL_8080* i8080) {
	return (i8080->status.S ? ret(i8080) : MAKERESULT(1, 4)) + RET_DIFF;
}

uint16_t rp(INTEL_8080* i8080) {
	return (!i8080->status.S ? ret(i8080) : MAKERESULT(1, 4)) + RET_DIFF;
}

uint16_t rpe(INTEL_8080* i8080) {
	return (i8080->status.P ? ret(i8080) : MAKERESULT(1, 4)) + RET_DIFF;
}

uint16_t rpo(INTEL_8080* i8080) {
	return (!i8080->status.P ? ret(i8080) : MAKERESULT(1, 4)) + RET_DIFF;
}

uint16_t rst(INTEL_8080* i8080) {
	i8080->SP -= 2;
	write_uint16_t_on_stack(i8080, i8080->PC + 1);
	i8080->PC = opcode(i8080) & 0b00111000;
	return MAKERESULT(0, 11 + CLK_MORE);
}

uint16_t ei(INTEL_8080* i8080) {
	i8080->INT_ENABLE = SET;
	return MAKERESULT(1, 4);
}

uint16_t di(INTEL_8080* i8080) {
	i8080->INT_ENABLE = RESET;
	return MAKERESULT(1, 4);
}

uint16_t in(INTEL_8080* i8080) {
	i8080->A = i8080->PORT[uint8_t_arg(i8080)];
	return MAKERESULT(2, 10);
}

uint16_t out(INTEL_8080* i8080) {
	i8080->PORT[uint8_t_arg(i8080)] = i8080->A;
	return MAKERESULT(2, 10);
}

uint16_t hlt(INTEL_8080* i8080) {
	i8080->HALT = SET;
	return MAKERESULT(1, 7);
}
