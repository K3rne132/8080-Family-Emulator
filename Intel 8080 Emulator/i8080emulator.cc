#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "i8080.h"
#include "memdump.h"

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

static const char* OPCODE_NAME[256] = {
	"NOP", "LXI B,%#06x", "STAX B", "INX B", "INR B", "DCR B", "MVI B,%#04x", "RLC", // 0x00 - 0x07
	"NOP", "DAD B", "LDAX B", "DCX B", "INR C", "DCR C", "MVI C,%#04x", "RRC", // 0x08 - 0x0F
	"NOP", "LXI D,%#06x", "STAX D", "INX D", "INR D", "DCR D", "MVI D,%#04x", "RAL", // 0x10 - 0x17
	"NOP", "DAD D", "LDAX D", "DCX D", "INR E", "DCR E", "MVI E,%#04x", "RAR", // 0x18 - 0x1F
	"NOP", "LXI H,%#06x", "SHLD %#06x", "INX H", "INR H", "DCR H", "MVI H,%#04x", "DAA", // 0x20 - 0x27
	"NOP", "DAD H", "LHLD %#06x", "DCX H", "INR L", "DCR L", "MVI L,%#04x", "CMA", // 0x28 - 0x2F
	"NOP", "LXI SP,%#06x", "STA %#06x", "INX SP", "INR M", "DCR M", "MVI M,%#04x", "STC", // 0x30 - 0x37
	"NOP", "DAD SP", "LDA %#06x", "DCX SP", "INR A", "DCR A", "MVI A,%#04x", "CMC", // 0x38 - 0x3F
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
	"RNZ", "POP B", "JNZ %#06x", "JMP %#06x", "CNZ %#06x", "PUSH B", "ADI %#04x", "RST 0", // 0xC0 - 0xC7
	"RZ", "RET", "JZ %#06x", "NOP", "CZ %#06x", "CALL %#06x", "ACI %#04x", "RST 1", // 0xC8 - 0xCF
	"RNC", "POP D", "JNC %#06x", "OUT %#04x", "CNC %#06x", "PUSH D", "SUI %#04x", "RST 2", // 0xD0 - 0xD7
	"RC", "NOP", "JC %#06x", "IN %#04x", "CC %#06x", "NOP", "SBI %#04x", "RST 3", // 0xD8 - 0xDF
	"RPO", "POP H", "JPO %#06x", "XTHL", "CPO %#06x", "PUSH H", "ANI %#04x", "RST 4", // 0xE0 - 0xE7
	"RPE", "PCHL", "JPE %#06x", "XCHG", "CPE %#06x", "NOP", "XRI %#04x", "RST 5", // 0xE8 - 0xEF
	"RP", "POP PSW", "JP %#06x", "DI", "CP %#06x", "PUSH PSW", "ORI %#04x", "RST 6", // 0xF0 - 0xF7
	"RM", "SPHL", "JM %#06x", "EI", "CM %#06x", "NOP", "CPI %#04x", "RST 7" // 0xF8 - 0xFF
};

static WORD word_arg(const INTEL_8080* i8080) {
	WORD result = i8080->MEM[i8080->PC + 1] << 8;
	result |= i8080->MEM[i8080->PC + 2];
	return result;
}

static inline int initialize(
	INTEL_8080* i8080,
	const WORD origin_pc,
	const WORD origin_sp
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

static inline void write_memory(
	INTEL_8080* i8080,
	const void* data, // pointer to data
	const WORD size   // number of bytes to write
) {
	memcpy(i8080->MEM, data, size);
}

static inline int write_file_to_memory(
	INTEL_8080* i8080,
	const char* filename,
	const WORD address
) {
	FILE* file = NULL;
	fopen_s(&file, filename, "rb");
	if (file == NULL) {
		fprintf(stderr, "Could not open file %s\n", filename);
		return 1;
	}
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	if (size + address > 0xFFFF) {
		fclose(file);
		fprintf(stderr, "Filename %s with size %#06x bytes is too large to "
			"be loaded at address %#06x\n", filename, size, address);
		return 1;
	}
	fseek(file, 0, SEEK_SET);
	fread(&i8080->MEM[address], sizeof(BYTE), size, file);
	fclose(file);
	return 0;
}

static inline void read_memory(
	const INTEL_8080* i8080,
	const WORD address, // pointer to address
	const WORD size   // number of bytes to read
) {
	memdump(&i8080->MEM[address], size);
}

static inline void bdos_syscall(INTEL_8080* i8080) {
	switch (i8080->C) {
	case 2: putchar(i8080->E); break; // C_WRITE
	case 9: // C_WRITESTR
		WORD index = i8080->DE;
		while (putchar(i8080->MEM[index++]) != '$');
		break;
	}
}

static void instruction_print(
	INTEL_8080* i8080,
	BYTE instruction
) {
	printf(OPCODE_NAME[instruction], word_arg(i8080));
	printf("        PC = %#06x\n", i8080->PC);
}

static inline int emulate(
	INTEL_8080* i8080
) {
	while (1) {
		if (i8080->INT && i8080->INT_PENDING) {
			instruction_print(i8080, i8080->INT_VECTOR);
			OPCODE_TABLE[i8080->INT_VECTOR](i8080);
			i8080->INT = 0;
			i8080->INT_PENDING = 0;
			i8080->INT_VECTOR = 0;
			i8080->HALT = 0;
		}
		else if (!i8080->HALT) {
			if (i8080->PC == 0x0005)
				bdos_syscall(i8080);
			//instruction_print(i8080, i8080->MEM[i8080->PC]);
			i8080->PC += OPCODE_TABLE[i8080->MEM[i8080->PC]](i8080);
		}
	}
	return 0;
}

int main(int argc, char** argv) {
	INTEL_8080 i8080;
	if (initialize(&i8080, 0x0100, 0x0000)) {
		fprintf(stderr, "Could not initialize INTEL_8080 structure\n");
		return 1;
	}
	i8080.MEM[0x0005] = 0xC9; // ret at bdos syscall
	i8080.MEM[0x0000] = 0x76; // hlt at 0x0000
	write_file_to_memory(&i8080, "8080EXM.COM", 0x0100);
	emulate(&i8080);
	destroy(&i8080);
}