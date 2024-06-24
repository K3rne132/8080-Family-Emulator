#pragma once
#include "i8080.h"
#include "debug_console.h"

typedef enum _CLK {
	CLK_INF = UINT32_MAX,
	CLK_8MHZ = 8000,
	CLK_4MHZ = 4000,
	CLK_2MHZ = 2000,
	CLK_1MHZ = 1000,
	CLK_100KHZ = 100,
	CLK_1KHZ = 1,
} CLK;

void int_handler(int sig);

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
	DBG_CONSOLE* screen,
	CLK clock
);

int process_args(
	const int argc,
	char** argv,
	int* input,
	int* debug,
	int* bdos,
	char** filename
);

void print_help(
	const char* program
);

void reset(
	INTEL_8080* i8080,
	DBG_CONSOLE* screen
);
