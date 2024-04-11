#include <stdlib.h>
#include <string.h>
#include "i8080.h"

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
	mov, mov, mov, mov, mov, mov, hlt, mov, // 0x70 - 0x77
	mov, mov, mov, mov, mov, mov, mov, mov, // 0x78 - 0x7F
	add, add, add, add, add, add, add, add, // 0x80 - 0x87
	adc, adc, adc, adc, adc, adc, adc, adc, // 0x88 - 0x8F
	sub, sub, sub, sub, sub, sub, sub, sub, // 0x90 - 0x97
	sbb, sbb, sbb, sbb, sbb, sbb, sbb, sbb, // 0x98 - 0x9F
	ana, ana, ana, ana, ana, ana, ana, ana, // 0xA0 - 0xA7
	xra, xra, xra, xra, xra, xra, xra, xra, // 0xA8 - 0xAF
	ora, ora, ora, ora, ora, ora, ora, ora, // 0xB0 - 0xB7
	cmp, cmp, cmp, cmp, cmp, cmp, cmp, cmp, // 0xB8 - 0xBF
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, rst, // 0xC0 - 0xC7
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, rst, // 0xC8 - 0xCF
	NULL, NULL, NULL, out, NULL, NULL, NULL, rst, // 0xD0 - 0xD7
	NULL, NULL, NULL, in, NULL, NULL, NULL, rst, // 0xD8 - 0xDF
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, rst, // 0xE0 - 0xE7
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, rst, // 0xE8 - 0xEF
	NULL, NULL, NULL, di, NULL, NULL, NULL, rst, // 0xF0 - 0xF7
	NULL, NULL, NULL, ei, NULL, NULL, NULL, rst  // 0xF8 - 0xFF
};

static inline int initialize(
	const WORD origin_pc,
	const WORD origin_sp,
	INTEL_8080* i8080
) {
	memset(i8080, 0, sizeof(INTEL_8080));
	i8080->F = 0b00000010;
	i8080->PC = origin_pc;
	i8080->SP = origin_sp;
	i8080->MEM = (BYTE*)calloc(0x10000, sizeof(BYTE));
	if (i8080->MEM == NULL)
		return 1;
	i8080->PORT = (BYTE*)calloc(0x100, sizeof(BYTE));
	if (i8080->PORT == NULL) {
		free(i8080->MEM);
		i8080->MEM = 0;
		return 1;
	}
	return 0;
}

static inline void destroy(
	INTEL_8080* i8080
) {
	free(i8080->MEM);
	free(i8080->PORT);
}

static inline void port_write(
	INTEL_8080* i8080,
	const BYTE port,
	const BYTE data
) {
	i8080->PORT[port] = data;
}

static inline BYTE port_read(
	const INTEL_8080* i8080,
	const BYTE port
) {
	return i8080->PORT[port];
}

static inline int interrupt(
	INTEL_8080* i8080,
	const BYTE vector
) {
	i8080->INT_PENDING = 1;
	i8080->INT_VECTOR = vector;
	return i8080->INT;
}

static inline int emulate(
	INTEL_8080* i8080
) {
	while (1) {
		if (i8080->INT && i8080->INT_PENDING) {
			i8080->INT = 0;
			i8080->INT_PENDING = 0;
			i8080->INT_VECTOR = 0;
			i8080->HALT = 0;
			OPCODE_TABLE[i8080->INT_VECTOR](i8080);
		}
		else if (!i8080->HALT)
			i8080->PC += OPCODE_TABLE[i8080->MEM[i8080->PC]](i8080);
	}
	return 0;
}
