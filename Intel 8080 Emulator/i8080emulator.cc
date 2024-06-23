#define _CRT_SECURE_NO_DEPRECATE
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>

#include "i8080emulator.h"
#include "bdos.h"
#include "system.h"

int i8080_initialize(
	INTEL_8080* i8080,
	const uint16_t origin_pc,
	const uint16_t origin_sp
) {
	memset(i8080, 0, sizeof(INTEL_8080));
	i8080->F = 0b00000010;
	i8080->PC = origin_pc;
	i8080->SP = origin_sp;
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
	i8080->INT_VECTOR = vector * 8;
	return i8080->INT_ENABLE;
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
	DBG_CONSOLE* screen,
	CLK clock
) {
	if (bdos) {
		i8080->MEM[0x0005] = 0xC9; // ret at bdos syscall
		i8080->MEM[0x0000] = 0x76; // hlt at 0x0000
	}
	uint16_t result = 0;
	uint32_t tmp_cycles = 0;
	uint32_t cycles = 0;
	uint32_t periods_to_sleep = 0;
	while (_RUNNING) {
		if (i8080->INT_ENABLE && i8080->INT_PENDING) {
			reset(i8080, screen);
			i8080->INT_ENABLE = RESET;
			i8080->INT_PENDING = RESET;
		}
#ifdef E_I8085
		else if (((INTEL_8085*)i8080)->TRAP) {
			reset(i8080, screen);
			((INTEL_8085*)i8080)->TRAP = RESET;
		}
		else if (i8080->INT_ENABLE && ((INTEL_8085*)i8080)->RIM.PENDING) {
			reset(i8080, screen);
			i8080->INT_ENABLE = RESET;
			((INTEL_8085*)i8080)->RIM.PENDING = RESET;
		}
#endif
		else if (!i8080->HALT) {
			if (i8080->STEPPING)
				i8080->HALT = SET;
			if (i8080->PC == 0x0005 && bdos)
				bdos_syscall(i8080, screen);
			add_to_history(screen, i8080->PC);
			result = EMUL_DATA[i8080->MEM[i8080->PC]].OPCODE(i8080);
			i8080->PC += GETINSTRUCTIONBYTES(result);
			tmp_cycles = GETINSTRUCTIONCYCLES(result);
			i8080->CYCLES += tmp_cycles;
			cycles = (cycles + tmp_cycles) % 100;
			if (cycles < tmp_cycles)
				periods_to_sleep++;
		}
		if (periods_to_sleep == clock) {
			thread_sleep(10);
			periods_to_sleep = 0;
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
	if (!(*filename))
		return 1;
	return 0;
}

void print_help(
	const char* program
) {
	fprintf(stderr, "Usage: %s <options> --file=<filename>\n"
		"Available options:\n"
		"    --debug         Turns on debug console\n"
		"    --input         Turns on special input handler\n"
		"    --bdos          Turns on simple bdos calls at 0x0005\n"
		"    --file=<file>   Selects file to write in memory at 0x0100\n"
		"    --help=<file>   Displays this help page\n",
		program);
}

void reset(
	INTEL_8080* i8080,
	DBG_CONSOLE* screen
) {
	i8080->SP -= 2;
	write_uint16_t_on_stack(i8080, i8080->PC); // PUSH PC
	i8080->PC = i8080->INT_VECTOR; // JMP <vector>
	add_to_history(screen, i8080->PC);
	i8080->CYCLES += (uint64_t)11 + CLK_MORE;
	i8080->HALT = RESET;
}

#ifndef E_I8085
int main(int argc, char** argv) {
	signal(SIGINT, int_handler);
	int is_input = 0;
	int is_debug = 0;
	int is_bdos = 0;
	char* filename = NULL;
	if (process_args(argc, argv, &is_input, &is_debug, &is_bdos, &filename)) {
		print_help(argv[0]);
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
#ifdef E_AM9080
	const char* screen_file = "am9080.cpf";
#else
	const char* screen_file = "intel8080.cpf";
#endif


	if (is_debug) {
		if (screen_initialize(&screen)) {
			fprintf(stderr, "Could not initialize DBG_CONSOLE structure\n");
			return 1;
		}
		if (read_screen_format(&screen, screen_file)) {
			fprintf(stderr, "Could not read screen format file %s\n", screen_file);
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
		emulate(&i8080, is_bdos, &screen, CLK_INF);
	else
		emulate(&i8080, is_bdos, NULL, CLK_INF);

	if (is_input)
		thread_destroy(input_thread);
	if (is_debug) {
		thread_destroy(drawing_thread);
		screen_destroy(&screen);
	}
	i8080_destroy(&i8080);
}
#endif
