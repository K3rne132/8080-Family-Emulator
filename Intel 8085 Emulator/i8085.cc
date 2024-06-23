#include "i8085.h"

uint16_t sim(INTEL_8080* i8080) {
	SIM_BITS sb = *(SIM_BITS*)(&i8080->A);
	INTEL_8085* i8085 = (INTEL_8085*)i8080;
	if (sb.SOE)
		i8085->SERIAL_OUT = sb.SOD;
	if (sb.MSE) {
		i8085->RIM.MASKS = sb.MASKS;
	}
	return MAKERESULT(1, 4);
}

uint16_t rim(INTEL_8080* i8080) {
	RIM_BITS rb = ((INTEL_8085*)i8080)->RIM;
	rb.IE = i8080->INT_ENABLE;
	rb.SID = ((INTEL_8085*)i8080)->SERIAL_IN;
	i8080->A = *(uint8_t*)(&rb);
	return MAKERESULT(1, 4);
}

uint16_t arhl(INTEL_8080* i8080) {
	i8080->status.C = i8080->L & 1;
	i8080->L >>= 1;
	i8080->L |= (i8080->H & 1);
	i8080->H >>= 1;
	i8080->H |= ((i8080->H & 0x40) << 1);
	i8080->status.V = RESET;
	return MAKERESULT(1, 7);
}

uint16_t dsub(INTEL_8080* i8080) {
	uint16_t result = i8080->HL - i8080->BC;
	set_ZSP_flags(i8080, result);
	i8080->status.C = result >> 8;
	i8080->status.AC = (((result ^ i8080->HL ^ ~i8080->BC) & 0x10) != 0);
	set_V_flag_int16(i8080, i8080->HL, ~i8080->BC, result);
	set_UI_flag_int8(i8080);
	i8080->HL = result;
	return MAKERESULT(1, 10);
}

uint16_t jnui(INTEL_8080* i8080) {
	return !(INTEL_8085*)i8080->status.U ? jmp(i8080) : MAKERESULT(3, 7);
}

uint16_t jui(INTEL_8080* i8080) {
	return (INTEL_8085*)i8080->status.U ? jmp(i8080) : MAKERESULT(3, 7);
}

uint16_t ldhi(INTEL_8080* i8080) {
	i8080->DE = i8080->HL + uint8_t_arg(i8080);
	return MAKERESULT(2, 10);
}

uint16_t ldsi(INTEL_8080* i8080) {
	i8080->DE = i8080->SP + uint8_t_arg(i8080);
	return MAKERESULT(2, 10);
}

uint16_t lhlx(INTEL_8080* i8080) {
	i8080->L = i8080->MEM[i8080->DE];
	i8080->H = i8080->MEM[i8080->DE + 1];
	return MAKERESULT(2, 10);
}

uint16_t rdel(INTEL_8080* i8080) {
	uint8_t new_carry = i8080->D >> 7;
	uint16_t old_de = i8080->DE;
	i8080->D <<= 1;
	i8080->D |= ((i8080->E >> 7) & 1);
	i8080->E <<= 1;
	i8080->E |= i8080->status.C;
	i8080->status.C = new_carry;
	set_V_flag_int16(i8080, old_de, old_de, i8080->DE);
	return MAKERESULT(1, 10);
}

uint16_t rstv(INTEL_8080* i8080) {
	if (!(INTEL_8085*)i8080->status.V)
		return MAKERESULT(1, 6);
	i8080->SP -= 2;
	write_uint16_t_on_stack(i8080, i8080->PC + 1);
	i8080->PC = opcode(i8080) & 0b00111000;
	return MAKERESULT(0, 12);
}

uint16_t shlx(INTEL_8080* i8080) {
	i8080->MEM[i8080->DE] = i8080->L;
	i8080->MEM[i8080->DE + 1] = i8080->H;
	return MAKERESULT(1, 10);
}
