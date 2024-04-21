#pragma once
#include <inttypes.h>
#include "i8080.h"

#define MAX_FORMAT_WIDTH  1024
#define MAX_FORMAT_HEIGHT 32

#define MAX_PROGRAM_WIDTH 21

#define MAX_STDOUT_WIDTH  80
#define MAX_STDOUT_HEIGHT 14

#define MAX_MEMORY_WIDTH  32

#define MAX_HISTORY_SIZE  8

#define FORMAT_SKIP    "                       "



static const char* register_hex_strings[] = {
	"BHEX", "CHEX", "DHEX", "EHEX", "HHEX", "LHEX", "FHEX", "AHEX"
};

static const char* register_dec_strings[] = {
	"BDE", "CDE", "DDE", "EDE", "HDE", "LDE", "FDE", "ADE"
};

static const char* flag_strings[] = {
	"FC", "\0", "FP", "\0", "FA", "\0", "FZ", "FS"
};

static const char* template_strings[] = {
	"PROGRAM+", "PROGRAM-", "STDOUT", "MEM"
};

static const uint32_t TEMPLATE_MAX_WIDTH[] = {
	MAX_PROGRAM_WIDTH, MAX_PROGRAM_WIDTH, MAX_STDOUT_WIDTH, MAX_MEMORY_WIDTH
};

static const uint32_t NUMBERS_MAX_WIDTH[] = {
	1, 1, 2, 4
};



typedef struct _SCREEN {
	char** screen_format;
	char** screen_text;
	char** standard_output;
	uint16_t instruction_history[MAX_HISTORY_SIZE];
	int queue_index;
	int standard_index;
	uint16_t memory_offset;
	uint16_t* prev_address;
	uint16_t* next_address;
} SCREEN;


typedef struct _DRAW_SCR_ARGS {
	SCREEN* screen;
	INTEL_8080* i8080;
} DRAW_SCR_ARGS;


int read_screen_format(SCREEN* screen, const char* SCREEN_FORMAT_FILE);
int screen_initialize(SCREEN* screen);
void screen_destroy(SCREEN* screen);
void add_to_history(SCREEN* screen, uint16_t pc);
void print_screen(SCREEN* screen, INTEL_8080* i8080);
void draw_screen(DRAW_SCR_ARGS args);
