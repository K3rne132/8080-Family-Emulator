#define _CRT_SECURE_NO_DEPRECATE
#include "debug_console.h"
#include "i8080emulator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <locale.h>
#include <string.h>
#include "system.h"

#define MAKE_WORD(a,b) ((a)|(b<<8))

static const char* OPCODE_NAME[256] = {
	"NOP", "LXI B,%hXh", "STAX B", "INX B", "INR B", "DCR B", "MVI B,%hhXh", "RLC", // 0x00 - 0x07
	"NOP", "DAD B", "LDAX B", "DCX B", "INR C", "DCR C", "MVI C,%hhXh", "RRC", // 0x08 - 0x0F
	"NOP", "LXI D,%hXh", "STAX D", "INX D", "INR D", "DCR D", "MVI D,%hhXh", "RAL", // 0x10 - 0x17
	"NOP", "DAD D", "LDAX D", "DCX D", "INR E", "DCR E", "MVI E,%hhXh", "RAR", // 0x18 - 0x1F
	"NOP", "LXI H,%hXh", "SHLD %hXh", "INX H", "INR H", "DCR H", "MVI H,%hhXh", "DAA", // 0x20 - 0x27
	"NOP", "DAD H", "LHLD %hXh", "DCX H", "INR L", "DCR L", "MVI L,%hhXh", "CMA", // 0x28 - 0x2F
	"NOP", "LXI SP,%hXh", "STA %hXh", "INX SP", "INR M", "DCR M", "MVI M,%hhXh", "STC", // 0x30 - 0x37
	"NOP", "DAD SP", "LDA %hXh", "DCX SP", "INR A", "DCR A", "MVI A,%hhXh", "CMC", // 0x38 - 0x3F
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
	"RNZ", "POP B", "JNZ %hXh", "JMP %hXh", "CNZ %hXh", "PUSH B", "ADI %hhXh", "RST 0", // 0xC0 - 0xC7
	"RZ", "RET", "JZ %hXh", "NOP", "CZ %hXh", "CALL %hXh", "ACI %hhXh", "RST 1", // 0xC8 - 0xCF
	"RNC", "POP D", "JNC %hXh", "OUT %hhXh", "CNC %hXh", "PUSH D", "SUI %hhXh", "RST 2", // 0xD0 - 0xD7
	"RC", "NOP", "JC %hXh", "IN %hhXh", "CC %hXh", "NOP", "SBI %hhXh", "RST 3", // 0xD8 - 0xDF
	"RPO", "POP H", "JPO %hXh", "XTHL", "CPO %hXh", "PUSH H", "ANI %hhXh", "RST 4", // 0xE0 - 0xE7
	"RPE", "PCHL", "JPE %hXh", "XCHG", "CPE %hXh", "NOP", "XRI %hhXh", "RST 5", // 0xE8 - 0xEF
	"RP", "POP PSW", "JP %hXh", "DI", "CP %hXh", "PUSH PSW", "ORI %hhXh", "RST 6", // 0xF0 - 0xF7
	"RM", "SPHL", "JM %hXh", "EI", "CM %hXh", "NOP", "CPI %hhXh", "RST 7" // 0xF8 - 0xFF
};

static const uint8_t OPCODE_LENGTH[256] = {
	1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 0x00 - 0x0F
	1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 0x10 - 0x1F
	1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1, // 0x20 - 0x2F
	1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1, // 0x30 - 0x3F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40 - 0x4F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x50 - 0x5F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60 - 0x6F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x70 - 0x7F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x80 - 0x8F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x90 - 0x9F
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xA0 - 0xAF
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xB0 - 0xBF
	1, 1, 3, 0, 3, 1, 1, 0, 1, 0, 3, 1, 3, 0, 2, 0, // 0xC0 - 0xCF
	1, 1, 3, 2, 3, 1, 2, 0, 1, 1, 3, 2, 3, 1, 2, 0, // 0xD0 - 0xDF
	1, 1, 3, 1, 3, 1, 2, 0, 1, 0, 3, 1, 3, 1, 2, 0, // 0xE0 - 0xEF
	1, 1, 3, 1, 3, 1, 2, 0, 1, 1, 3, 1, 3, 1, 2, 0  // 0xF0 - 0xFF
};

// read screen format file from disk
int read_screen_format(
	DBG_CONSOLE* screen,
	const char* screen_format_file
) {
	FILE* file =  fopen(screen_format_file, "r");
	if (file == NULL) {
		fprintf(stderr, "Could not open file %s\n", screen_format_file);
		return -1;
	}
	for (int i = 0; i < MAX_FORMAT_HEIGHT; i++) {
		fgets(screen->screen_format[i], MAX_FORMAT_WIDTH, file);
	}
	fclose(file);
	return 0;
}

// replaces known pattern with given data
int replace_pattern(
	char* line,
	const char* pattern,
	const char* format,
	...
) {
	char* ptr = strstr(line, pattern);
	if (ptr == NULL)
		return -1;
	const size_t len = strlen(pattern) + 1;
	char* replace = (char*)malloc(len);

	va_list values;
	va_start(values, format);
	vsnprintf(replace, len, format, values);
	va_end(values);

	if (replace != NULL) {
		memcpy(ptr, replace, len - 1);
	}

	free(replace);
	return 0;
}

// replaces memory box with program with actual data from procesor memory
int replace_program(
	INTEL_8080* i8080,
	char* line,
	const char* pattern,
	const size_t num,
	uint16_t* address
) {
	const size_t len = strlen(pattern);
	char* text = (char*)malloc(len);
	if (text == NULL)
		return -1;
	snprintf(text, len, OPCODE_NAME[i8080->MEM[address[num]]],
		MAKE_WORD(i8080->MEM[address[num] + 1], i8080->MEM[address[num] + 2]));
	int result = replace_pattern(line, pattern,
		(address[num] != 0) ? ("0x%04X %-15s") : (FORMAT_SKIP), address[num], text);
	free(text);
	return result;
}

// replaces memory box in screen template with actual data from procesor memory
int replace_memory(
	INTEL_8080* i8080,
	char* line,
	const char* pattern,
	const uint16_t num,
	const uint16_t memory_offset
) {
	char* text = (char*)malloc(MAX_MEMORY_WIDTH * sizeof(char));
	if (text == NULL)
		return -1;
	const uint16_t offset = memory_offset + num * 8;
	snprintf(text, strlen(text), "%02X %02X %02X %02X %02X %02X %02X %02X ",
		i8080->MEM[offset + 0], i8080->MEM[offset + 1], i8080->MEM[offset + 2],
		i8080->MEM[offset + 3], i8080->MEM[offset + 4], i8080->MEM[offset + 5],
		i8080->MEM[offset + 6], i8080->MEM[offset + 7]);
	int result = replace_pattern(line, pattern, "0x%04X: %s", offset, text);
	free(text);
	return result;
}

// prints console screen in console
void print_screen(
	DBG_CONSOLE* screen,
	INTEL_8080* i8080
) {
	printf("\033[H");
	for (int i = 0; i < MAX_FORMAT_HEIGHT; i++)
		memcpy(screen->screen_text[i], screen->screen_format[i], MAX_FORMAT_WIDTH);

	for (int i = 0; i < MAX_HISTORY_SIZE; i++) {
		const int index = screen->queue_index + MAX_HISTORY_SIZE - i;
		screen->prev_address[i] = screen->instruction_history[index % MAX_HISTORY_SIZE];
	}
	memset(screen->next_address, 0, (MAX_HISTORY_SIZE + 1) * sizeof(uint16_t));
	uint16_t fake_pc = i8080->PC;
	for (int j = 0; j <= MAX_HISTORY_SIZE; j++) {
		screen->next_address[j] = fake_pc;
		uint8_t opcode_len = OPCODE_LENGTH[i8080->MEM[fake_pc]];
		if (opcode_len == 0 && j != MAX_HISTORY_SIZE) {
			screen->next_address[j + 1] = fake_pc + 1;
			break;
		}
		else {
			fake_pc += opcode_len;
		}
	}

	for (int i = 0; i < MAX_FORMAT_HEIGHT; i++) {
		char* text = screen->screen_text[i];
		for (int j = 0; j < 8; j++) {
			replace_pattern(text, register_hex_strings[j], "0x%02X", i8080->REG[j]);
			replace_pattern(text, register_dec_strings[j], "%03d", i8080->REG[j]);
			if (flag_strings[j] != "\0") {
				replace_pattern(text, flag_strings[j], " %1d", (i8080->F >> j) & 1);
			}
		}
		replace_pattern(text, "SPHEEX", "0x%04X", i8080->SP);
		replace_pattern(text, "PCHEEX", "0x%04X", i8080->PC);
		replace_pattern(text, "FH", " %1d", i8080->HALT);
		replace_pattern(text, "FI", " %1d", i8080->INT);
	}

	for (int i = 0; i < MAX_FORMAT_HEIGHT; i++) {
		for (int j = 0; j < 4; j++) {
			char* ptr = strstr(screen->screen_text[i], template_strings[j]);
			if (ptr != NULL) {
				int offset = ptr - screen->screen_text[i];
				char* template_format = (char*)malloc(TEMPLATE_MAX_WIDTH[j] * sizeof(char));
				const size_t template_len = strlen(template_strings[j]);
				memset(template_format, ' ', TEMPLATE_MAX_WIDTH[j]);
				template_format[TEMPLATE_MAX_WIDTH[j] - 1] = 0;
				memcpy(template_format, template_strings[j], template_len);

				int num = atoi(&screen->screen_text[i][offset + template_len]);
				memcpy(template_format + template_len,
					&screen->screen_text[i][offset + template_len],
					NUMBERS_MAX_WIDTH[j]);
				switch (j) {
				case 0: //PROGRAM+
					replace_program(i8080, screen->screen_text[i],
						template_format, num, screen->next_address);
					break;
				case 1: //PROGRAM-
					replace_program(i8080, screen->screen_text[i], 
						template_format, num, screen->prev_address);
					break;
				case 2: //STDOUT
					replace_pattern(screen->screen_text[i],
						template_format, "%-79s ", screen->standard_output[num]);
					break;
				case 3: //MEM
					replace_memory(i8080, screen->screen_text[i],
						template_format, num, screen->memory_offset);
					break;
				}
				free(template_format);
			}
		}
	}
	for (int i = 0; i < MAX_FORMAT_HEIGHT; ++i) {
		printf(screen->screen_text[i]);
	}
}

// initialize necesary variables for screen
int screen_initialize(
	DBG_CONSOLE* screen
) {
	setlocale(LC_ALL, "pl_PL.UTF-8");
	memset(screen, 0, sizeof(DBG_CONSOLE));

	screen->screen_format = (char**)calloc(MAX_FORMAT_HEIGHT, sizeof(char*));
	screen->screen_text = (char**)calloc(MAX_FORMAT_HEIGHT, sizeof(char*));
	screen->standard_output = (char**)calloc(MAX_STDOUT_HEIGHT, sizeof(char*));
	if (!screen->screen_format || !screen->screen_text || !screen->standard_output) {
		screen_destroy(screen);
		return -101;
	}

	for (int i = 0; i < MAX_FORMAT_HEIGHT; i++) {
		screen->screen_format[i] = (char*)calloc(MAX_FORMAT_WIDTH, sizeof(char));
		screen->screen_text[i] = (char*)calloc(MAX_FORMAT_WIDTH, sizeof(char));
		if (!screen->screen_format[i] || !screen->screen_text[i]) {
			screen_destroy(screen);
			return -102;
		}
	}

	for (int i = 0; i < MAX_STDOUT_HEIGHT; i++) {
		screen->standard_output[i] = (char*)calloc(MAX_STDOUT_WIDTH, sizeof(char));
		if (!screen->standard_output[i]) {
			screen_destroy(screen);
			return -103;
		}
		memset(screen->standard_output[i], ' ', MAX_STDOUT_WIDTH);
	}

	screen->prev_address = (uint16_t*)calloc((MAX_HISTORY_SIZE), sizeof(uint16_t));
	screen->next_address = (uint16_t*)calloc((MAX_HISTORY_SIZE), sizeof(uint16_t));
	if (!screen->prev_address || !screen->next_address) {
		screen_destroy(screen);
		return -104;
	}
	return 0;
}

// deallocate used memory blocks
void screen_destroy(DBG_CONSOLE* screen) {
	for (int i = 0; i < MAX_FORMAT_HEIGHT; i++) {
		free(screen->screen_format[i]);
		free(screen->screen_text[i]);
	}
	for (int i = 0; i < MAX_STDOUT_HEIGHT; i++) {
		free(screen->standard_output[i]);
	}

	free(screen->screen_format);
	free(screen->screen_text);
	free(screen->standard_output);

	free(screen->prev_address);
	free(screen->next_address);
	memset(screen, 0, sizeof(DBG_CONSOLE));
}

// add instruction to history
void add_to_history(DBG_CONSOLE* screen, uint16_t pc) {
	if (!screen)
		return;
	screen->instruction_history[screen->queue_index] = pc;
	screen->queue_index = (++screen->queue_index % MAX_HISTORY_SIZE);
}

void draw_screen(DRAW_SCR_ARGS* args) {
	DBG_CONSOLE* screen = args->screen;
	INTEL_8080* i8080 = args->i8080;
	while (1) {
		print_screen(screen, i8080);
	}
}

void process_input(DRAW_SCR_ARGS* args) {
	DBG_CONSOLE* screen = args->screen;
	INTEL_8080* i8080 = args->i8080;
	initialize_keys();
	while (1) {
		switch (getch()) {
		case 'b': i8080->HALT = SET; break; // breakpoint
		case 's': i8080->STEPPING = SET; i8080->HALT = RESET; break; // step
		case 'r': i8080->STEPPING = RESET; i8080->HALT = RESET; break; // run
		case '0': interrupt(i8080, 0); break; // RST 0
		case '1': interrupt(i8080, 1); break; // RST 1
		case '2': interrupt(i8080, 2); break; // RST 2
		case '3': interrupt(i8080, 3); break; // RST 3
		case '4': interrupt(i8080, 4); break; // RST 4
		case '5': interrupt(i8080, 5); break; // RST 5
		case '6': interrupt(i8080, 6); break; // RST 6
		case '7': interrupt(i8080, 7); break; // RST 7
		}
	}
	cleanup_keys();
}

void put_character(DBG_CONSOLE* screen, char c) {
	if (!screen) {
		putchar(c);
		return;
	}

	int si = screen->standard_index;
	int row = si / MAX_STDOUT_WIDTH;
	if (si / MAX_STDOUT_WIDTH >= MAX_STDOUT_HEIGHT) {
		for (int i = 0; i < MAX_STDOUT_HEIGHT; i++)
			memset(screen->standard_output[i], ' ', MAX_STDOUT_WIDTH);
		screen->standard_index = 0;
		si = screen->standard_index;
		row = si / MAX_STDOUT_WIDTH;
	}

	if (c == '\n')
		screen->standard_index = ((row) + 1) * MAX_STDOUT_WIDTH;
	else if (c == '\7') { // BELL
		putchar(c);
	}
	else if (c >= ' ') {
		screen->standard_output[row][si % MAX_STDOUT_WIDTH] = c;
		screen->standard_index++;
	}
}
