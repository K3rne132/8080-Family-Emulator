#define _CRT_SECURE_NO_DEPRECATE
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "bdos.h"
#include "system.h"

const INSTRUCTION OPCODE_TABLE[256] = {
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

int i8080_initialize(
	INTEL_8080* i8080,
	const uint16_t origin_pc,
	const uint16_t origin_sp
) {
	memset(i8080, 0, sizeof(INTEL_8080));
	i8080->F = 0b00000010;
	i8080->PC = origin_pc;
	i8080->SP = origin_sp;
	i8080->INSTRUCTIONS = OPCODE_TABLE;
	i8080->MEM = (uint8_t*)calloc(0x10000, sizeof(uint8_t));
	if (i8080->MEM == NULL)
		return 1;
	i8080->PORT = (uint8_t*)calloc(0x100, sizeof(uint8_t));
	if (i8080->PORT == NULL) {
		free(i8080->MEM);
		i8080->MEM = 0;
		return 1;
	}
	return 0;
}

void i8080_destroy(
	INTEL_8080* i8080
) {
	free(i8080->MEM);
	free(i8080->PORT);
	memset(i8080, 0, sizeof(INTEL_8080));
}

void port_write(
	INTEL_8080* i8080,
	const uint8_t port,
	const uint8_t data
) {
	i8080->PORT[port] = data;
}

uint8_t port_read(
	const INTEL_8080* i8080,
	const uint8_t port
) {
	return i8080->PORT[port];
}

int interrupt(
	INTEL_8080* i8080,
	const uint8_t vector // RST <vector>
) {
	assert(vector >= 0 && vector <= 7);
	i8080->INT_PENDING = SET;
	i8080->INT_VECTOR = 0xC7 + 8 * vector;
	return i8080->INT;
}

void write_memory(
	INTEL_8080* i8080,
	const void* data, // pointer to data
	const uint16_t size   // number of uint8_ts to write
) {
	memcpy(i8080->MEM, data, size);
}

int write_file_to_memory(
	INTEL_8080* i8080,
	const char* filename,
	const uint16_t address
) {
	FILE* file = fopen(filename, "rb");
	if (file == NULL) {
		fprintf(stderr, "Could not open file %s\n", filename);
		return 1;
	}
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	if (size + address > 0xFFFF) {
		fclose(file);
		fprintf(stderr, "Filename %s with size %hXh uint8_ts is too large to "
			"be loaded at address %hXh\n", filename, size, address);
		return 2;
	}
	fseek(file, 0, SEEK_SET);
	fread(&i8080->MEM[address], sizeof(uint8_t), size, file);
	fclose(file);
	return 0;
}

int emulate(
	INTEL_8080* i8080,
	uint8_t bdos,
	DBG_CONSOLE* screen
) {
	if (bdos) {
		i8080->MEM[0x0005] = 0xC9; // ret at bdos syscall
		i8080->MEM[0x0000] = 0x76; // hlt at 0x0000
	}
	uint16_t result = 0;
	while (1) {
		if (i8080->INT && i8080->INT_PENDING) {
			i8080->SP -= 2;
			write_uint16_t_on_stack(i8080, i8080->PC);
			i8080->PC = i8080->INT_PENDING << 3;
			i8080->INT = 0;
			i8080->INT_PENDING = 0;
			i8080->HALT = 0;
			add_to_history(screen, i8080->PC);
			result = i8080->INSTRUCTIONS[i8080->INT_VECTOR](i8080);
			i8080->CYCLES += GETINSTRUCTIONCYCLES(result);
			i8080->INT_VECTOR = 0;
		}
		else if (!i8080->HALT) {
			if (i8080->STEPPING)
				i8080->HALT = SET;
			if (i8080->PC == 0x0005 && bdos)
				bdos_syscall(i8080, screen);
			add_to_history(screen, i8080->PC);
			result = i8080->INSTRUCTIONS[i8080->MEM[i8080->PC]](i8080);
			i8080->PC += GETINSTRUCTIONBYTES(result);
			i8080->CYCLES += GETINSTRUCTIONCYCLES(result);
		}
	}
	return 0;
}

int process_args(
	const int argc,
	char** argv,
	int* input,
	int* debug,
	int* bdos,
	char** filename
) {
	assert(input);
	assert(debug);
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--debug") == 0)
			*debug = SET;
		else if (strcmp(argv[i], "--input") == 0)
			*input = SET;
		else if (strcmp(argv[i], "--bdos") == 0)
			*bdos = SET;
		else if (strncmp(argv[i], "--file=", 7) == 0) {
			*filename = argv[i] + 7;
		}
		else if (strcmp(argv[i], "--help") == 0) {
			return 1;
		}
		else {
			fprintf(stderr, "Unknown option: %s\n\n", argv[i]);
			return 1;
		}
	}
	if (!filename)
		return 1;
	return 0;
}

int main(int argc, char** argv) {
	int is_input = 0;
	int is_debug = 0;
	int is_bdos = 0;
	char* filename = NULL;
	if (process_args(argc, argv, &is_input, &is_debug, &is_bdos, &filename)) {
		fprintf(stderr, "Usage: %s <options> --file=<filename>\n"
			"Available options:\n"
			"    --debug         Turns on debug console\n"
			"    --input         Turns on special input handler\n"
			"    --bdos          Turns on simple bdos calls at 0x0005\n"
			"    --file=<file>   Selects file to write in memory at 0x0100\n"
			"    --help=<file>   Displays this help page\n",
			argv[0]);
		return 1;
	}

	INTEL_8080 i8080;
	if (i8080_initialize(&i8080, 0x0100, 0x0000)) {
		fprintf(stderr, "Could not initialize INTEL_8080 structure\n");
		return 1;
	}

	if (write_file_to_memory(&i8080, filename, 0x0100)) {
		fprintf(stderr, "Error while reading file %s\n", filename);
		return 1;
	}

	DBG_CONSOLE screen;
	THREAD drawing_thread = NULL;
	THREAD input_thread = NULL;
	DRAW_SCR_ARGS args = { NULL, &i8080 };

	if (is_debug) {
		if (screen_initialize(&screen)) {
			fprintf(stderr, "Could not initialize DBG_CONSOLE structure\n");
			return 1;
		}
		if (read_screen_format(&screen, "intel8080.cpf")) {
			fprintf(stderr, "Could not read screen format file %s\n", "intel8080.cpf");
			return 1;
		}
		args.screen = &screen;

		drawing_thread = thread_create((void* (*)(void*))draw_screen, &args);
		if (!drawing_thread) {
			fprintf(stderr, "Starting drawing thread failed\n");
			return 1;
		}
	}
	
	if (is_input) {
		input_thread = thread_create((void* (*)(void*))process_input, &args);
		if (!input_thread) {
			fprintf(stderr, "Starting input processing thread failed\n");
			return 1;
		}
	}
	
	if (is_debug)
		emulate(&i8080, is_bdos, &screen);
	else
		emulate(&i8080, is_bdos, NULL);

	if (is_input)
		thread_destroy(input_thread);
	if (is_debug) {
		thread_destroy(drawing_thread);
		screen_destroy(&screen);
	}
	i8080_destroy(&i8080);
}
