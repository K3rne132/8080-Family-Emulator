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
	snprintf(text, len, EMUL_DATA[i8080->MEM[address[num]]].OPCODE_NAME,
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
	snprintf(text, MAX_MEMORY_WIDTH, "%02X %02X %02X %02X %02X %02X %02X %02X ",
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
	screen_go_home();
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
		uint8_t opcode_len = EMUL_DATA[i8080->MEM[fake_pc]].OPCODE_LENGTH;
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
		replace_pattern(text, "FI", " %1d", i8080->INT_ENABLE);
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
	initialize_screen();
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
	cleanup_screen();
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
