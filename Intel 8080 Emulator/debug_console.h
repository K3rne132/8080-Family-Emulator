#pragma once
#include <inttypes.h>
#ifdef E_I8085
#include "../Intel 8085 Emulator/i8085.h"
#else
#include "i8080.h"
#endif

#define MAX_FORMAT_WIDTH  1024
#define MAX_FORMAT_HEIGHT 32

#define MAX_PROGRAM_WIDTH 21

#define MAX_STDOUT_WIDTH  80
#define MAX_STDOUT_HEIGHT 14

#define MAX_MEMORY_WIDTH  32

#define MAX_HISTORY_SIZE  8

#define FORMAT_SKIP "                       "



static const char* register_hex_strings[] = {
	"BHEX", "CHEX", "DHEX", "EHEX", "HHEX", "LHEX", "FHEX", "AHEX"
};

static const char* register_dec_strings[] = {
	"BDE", "CDE", "DDE", "EDE", "HDE", "LDE", "FDE", "ADE"
};

static const char* flag_strings[] = {
	"FC", "FU", "FP", "\0", "FA", "FB", "FZ", "FS"
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



typedef struct _DBG_CONSOLE {
	char** screen_format;
	char** screen_text;
	char** standard_output;
	uint16_t instruction_history[MAX_HISTORY_SIZE];
	int queue_index;
	int standard_index;
	uint16_t memory_offset;
	uint16_t* prev_address;
	uint16_t* next_address;
} DBG_CONSOLE;


typedef struct _DRAW_SCR_ARGS {
	DBG_CONSOLE* screen;
	INTEL_8080* i8080;
} DRAW_SCR_ARGS;


int read_screen_format(DBG_CONSOLE* screen, const char* SCREEN_FORMAT_FILE);
int screen_initialize(DBG_CONSOLE* screen);
void screen_destroy(DBG_CONSOLE* screen);
void add_to_history(DBG_CONSOLE* screen, uint16_t pc);
void print_screen(DBG_CONSOLE* screen, INTEL_8080* i8080);
void draw_screen(DRAW_SCR_ARGS* args);
void process_input(DRAW_SCR_ARGS* args);
void put_character(DBG_CONSOLE* screen, char c);
