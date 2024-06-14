#define _CRT_SECURE_NO_DEPRECATE
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "i8085emulator.h"
#include "i8080emulator.h"
#include "bdos.h"
#include "system.h"

int i8085_initialize(
	INTEL_8085* i8085,
	const uint16_t origin_pc,
	const uint16_t origin_sp
) {
	i8080_initialize((INTEL_8080*)i8085, origin_pc, origin_sp);
}

void i8085_destroy(
	INTEL_8085* i8085
) {
	i8080_destroy((INTEL_8080*)i8085);
}

int hardware_interrupt(
	INTEL_8085* i8085,
	HARDWARE_INTERRUPT interrupt
) {
	i8085->CORE.INT_PENDING = SET;
	i8085->CORE.INT_VECTOR = interrupt;
	return i8085->CORE.INT_ENABLE;
}

int intr(
	INTEL_8085* i8085,
	uint16_t address
) {
	i8085->CORE.INT_PENDING = SET;
	i8085->CORE.INT_VECTOR = address;
	return i8085->CORE.INT_ENABLE;
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

	INTEL_8085 i8085;
	if (i8085_initialize(&i8085, 0x0100, 0x0000)) {
		fprintf(stderr, "Could not initialize INTEL_8080 structure\n");
		return 1;
	}

	if (write_file_to_memory((INTEL_8080*)&i8085, filename, 0x0100)) {
		fprintf(stderr, "Error while reading file %s\n", filename);
		return 1;
	}

	DBG_CONSOLE screen;
	THREAD drawing_thread = NULL;
	THREAD input_thread = NULL;
	DRAW_SCR_ARGS args = { NULL, (INTEL_8080*)&i8085 };

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
		emulate((INTEL_8080*)&i8085, is_bdos, &screen, CLK_INF);
	else
		emulate((INTEL_8080*)&i8085, is_bdos, NULL, CLK_INF);

	if (is_input)
		thread_destroy(input_thread);
	if (is_debug) {
		thread_destroy(drawing_thread);
		screen_destroy(&screen);
	}
	i8085_destroy(&i8085);
}
