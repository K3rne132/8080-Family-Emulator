#pragma once
#define _CRT_SECURE_NO_DEPRECATE

#include "i8080.h"
#include "debug_console.h"

int i8080_initialize(
	INTEL_8080* i8080,
	const uint16_t origin_pc,
	const uint16_t origin_sp
);

void i8080_destroy(
	INTEL_8080* i8080
);

void port_write(
	INTEL_8080* i8080,
	const uint8_t port,
	const uint8_t data
);

uint8_t port_read(
	const INTEL_8080* i8080,
	const uint8_t port
);

int interrupt(
	INTEL_8080* i8080,
	const uint8_t vector // RST <vector>
);

void write_memory(
	INTEL_8080* i8080,
	const void* data, // pointer to data
	const uint16_t size   // number of uint8_ts to write
);

int write_file_to_memory(
	INTEL_8080* i8080,
	const char* filename,
	const uint16_t address
);

int emulate(
	INTEL_8080* i8080,
	uint8_t bdos,
	DBG_CONSOLE* screen
);

int process_args(
	const int argc,
	char** argv,
	int* input,
	int* debug,
	int* bdos,
	char** filename
);
