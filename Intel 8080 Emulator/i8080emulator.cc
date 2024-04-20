#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <locale.h>

#include "i8080.h"
#include "memdump.h"
#include <iostream>
#include <string>
#include <queue>
#include <cstdarg>


static const char* SCREEN_FORMAT_FILE = "intel8080.cpf";
char** screen_format;
char** screen_text;
char** standard_output;

static const unsigned int MAX_FORMAT_WIDTH = 1024;
static const unsigned int MAX_FORMAT_HEIGHT = 32;
static const unsigned int MAX_PROGRAM_WIDTH = 21;
static const unsigned int MAX_STDOUT_WIDTH = 80;
static const unsigned int MAX_STDOUT_HEIGHT = 14;
static const unsigned int MAX_MEMORY_WIDTH = 32;

static const char* register_hex_strings[] = { "BHEX", "CHEX", "DHEX", "EHEX", "HHEX", "LHEX", "FHEX", "AHEX" };
static const char* register_dec_strings[] = { "BDE", "CDE", "DDE", "EDE", "HDE", "LDE", "FDE", "ADE" };
static const char* flag_strings[] = { "FC", "\0", "FP", "\0", "FA", "\0", "FZ", "FS" };
static const char* template_strings[] = { "PROGRAM+", "PROGRAM-", "STDOUT", "MEM" };
static const unsigned int TEMPLATE_MAX_WIDTH[] = { MAX_PROGRAM_WIDTH, MAX_PROGRAM_WIDTH, MAX_STDOUT_WIDTH, MAX_MEMORY_WIDTH };
static const unsigned int NUMBERS_MAX_WIDTH[] = { 1, 1, 2, 4 };

static const unsigned int MAX_HISTORY_SIZE = 8;
std::queue<uint16_t> instruction_history;
int standard_index = 0;
uint16_t memory_offset = 0;

uint16_t* prev_address;
uint16_t* next_address;

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

static const uint8_t OPCODE_LENGTH[256] = {
	1,	3,	1,	1,	1,	1,	2,	1,	// 0x00 - 0x07
	1,	1,	1,	1,	1,	1,	2,	1,	// 0x08 - 0x0F
	1,	3,	1,	1,	1,	1,	2,	1,	// 0x10 - 0x17
	1,	1,	1,	1,	1,	1,	2,	1,	// 0x18 - 0x1F
	1,	3,	3,	1,	1,	1,	2,	1,	// 0x20 - 0x27
	1,	1,	3,	1,	1,	1,	2,	1,	// 0x28 - 0x2F
	1,	3,	3,	1,	1,	1,	2,	1,	// 0x30 - 0x37
	1,	1,	3,	1,	1,	1,	2,	1,	// 0x38 - 0x3F
	1,	1,	1,	1,	1,	1,	1,	1,	// 0x40 - 0x47
	1,	1,	1,	1,	1,	1,	1,	1,	// 0x48 - 0x4F
	1,	1,	1,	1,	1,	1,	1,	1,	// 0x50 - 0x57
	1,	1,	1,	1,	1,	1,	1,	1,	// 0x58 - 0x5F
	1,	1,	1,	1,	1,	1,	1,	1,	// 0x60 - 0x67
	1,	1,	1,	1,	1,	1,	1,	1,	// 0x68 - 0x6F
	1,	1,	1,	1,	1,	1,	1,	1,	// 0x70 - 0x77
	1,	1,	1,	1,	1,	1,	1,	1,	// 0x78 - 0x7F
	1,	1,	1,	1,	1,	1,	1,	1,	// 0x80 - 0x87
	1,	1,	1,	1,	1,	1,	1,	1,	// 0x88 - 0x8F
	1,	1,	1,	1,	1,	1,	1,	1,	// 0x90 - 0x97
	1,	1,	1,	1,	1,	1,	1,	1,	// 0x98 - 0x9F
	1,	1,	1,	1,	1,	1,	1,	1,	// 0xA0 - 0xA7
	1,	1,	1,	1,	1,	1,	1,	1,	// 0xA8 - 0xAF
	1,	1,	1,	1,	1,	1,	1,	1,	// 0xB0 - 0xB7
	1,	1,	1,	1,	1,	1,	1,	1,	// 0xB8 - 0xBF
	1,	1,	3,	0,	3,	1,	1,	0,	// 0xC0 - 0xC7
	1,	0,	3,	1,	3,	0,	2,	0,	// 0xC8 - 0xCF
	1,	1,	3,	2,	3,	1,	2,	0,	// 0xD0 - 0xD7
	1,	1,	3,	2,	3,	1,	2,	0,	// 0xD8 - 0xDF
	1,	1,	3,	1,	3,	1,	2,	0,	// 0xE0 - 0xE7
	1,	0,	3,	1,	3,	1,	2,	0,	// 0xE8 - 0xEF
	1,	1,	3,	1,	3,	1,	2,	0,	// 0xF0 - 0xF7
	1,	1,	3,	1,	3,	1,	2,	0   // 0xF8 - 0xFF
};


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
	const BYTE vector // RST <vector>
) {
	assert(vector >= 0 && vector <= 7);
	i8080->INT_PENDING = SET;
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
		fprintf(stderr, "Filename %s with size %hXh bytes is too large to "
			"be loaded at address %hXh\n", filename, size, address);
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
/*static int find_text(const char* text, const char* pattern) {
	for (int offset = 0; offset < MAX_FORMAT_WIDTH - strlen(pattern); ++offset) {
		bool found = true;
		for (int i = 0; i < strlen(pattern); ++i) {
			if (text[offset + i] != pattern[i]) {
					found = false;
					break;
			}
		}
		if (found) {
			return offset;
		}
	}
	return -1;
}*/

static void read_screen_format(
	INTEL_8080* i8080, 
	char** screen_format
) {
	FILE* fp;
	errno_t err = fopen_s(&fp, SCREEN_FORMAT_FILE, "r");
	if (err == 0) {
		for (int i = 0; i < MAX_FORMAT_HEIGHT; ++i) {
			fgets(screen_format[i], MAX_FORMAT_WIDTH, fp);
		}
		fclose(fp);
	}
}

static int replace_pattern(char* line, const char* pattern, const char* format, ...) {
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
static void replace_program(INTEL_8080* i8080, char* line, const char* pattern, const size_t num, uint16_t* address) {
	const size_t len = strlen(pattern);
	char* text = (char*)malloc(len);
	snprintf(text, len, OPCODE_NAME[i8080->MEM[address[num]]], i8080->MEM[address[num] + 1]);
	replace_pattern(line, pattern, address[num] != 0 ? "0x%04x %-15s" : "                       ", address[num], text);
	free(text);
}

static void replace_memory(INTEL_8080* i8080, char* line, const char* pattern, const int num, const int memory_offset) {
	char* text = (char*)malloc(MAX_MEMORY_WIDTH * sizeof(char));
	if (text != NULL) {
		snprintf(text, strlen(text), "%02x %02x %02x %02x %02x %02x %02x %02x "
			, i8080->MEM[memory_offset + num * 8 + 0]
			, i8080->MEM[memory_offset + num * 8 + 1]
			, i8080->MEM[memory_offset + num * 8 + 2]
			, i8080->MEM[memory_offset + num * 8 + 3]
			, i8080->MEM[memory_offset + num * 8 + 4]
			, i8080->MEM[memory_offset + num * 8 + 5]
			, i8080->MEM[memory_offset + num * 8 + 6]
			, i8080->MEM[memory_offset + num * 8 + 7]);
	}
	replace_pattern(line, pattern, "0x%04x: %s", memory_offset + num * 8, text);
	free(text);
}

static void prepare_screen(INTEL_8080* i8080) {
	for (int i = 0; i < MAX_FORMAT_HEIGHT; ++i) {
		memcpy(screen_text[i], screen_format[i], sizeof(char) * MAX_FORMAT_WIDTH);
	}

	std::queue<uint16_t> q;
	int qsize = instruction_history.size();

	memset(prev_address, 0, (MAX_HISTORY_SIZE + 1) * sizeof(uint16_t));
	while (qsize--) {
		q.push(instruction_history.front());
		prev_address[qsize + 1] = instruction_history.front();
		instruction_history.pop();
	}
	instruction_history = q;
	

	memset(next_address, 0, (MAX_HISTORY_SIZE + 1) * sizeof(uint16_t));
	uint16_t fake_pc = i8080->PC;
	for (int j = 0; j <= MAX_HISTORY_SIZE; j++) {
		next_address[j] = fake_pc;
		uint8_t opcode_len = OPCODE_LENGTH[i8080->MEM[fake_pc]];
		if (opcode_len == 0) {
			if (j != MAX_HISTORY_SIZE) {
				next_address[j + 1] = fake_pc + 1;
			}
			break;
		}
		else {
			fake_pc += opcode_len;
		}
	}

	for (int i = 0; i < MAX_FORMAT_HEIGHT; i++) {
		for (int j = 0; j < 8; j++) {
			replace_pattern(screen_text[i], register_hex_strings[j], "0x%02x", i8080->REG[j]);
			replace_pattern(screen_text[i], register_dec_strings[j], "%03d", i8080->REG[j]);
			if (flag_strings[j] != "\0") {
				replace_pattern(screen_text[i], flag_strings[j], " %1d", (i8080->F >> j) & 1);
			}
		}
		replace_pattern(screen_text[i], "SPHEEX", "0x%04x", i8080->SP);
		replace_pattern(screen_text[i], "PCHEEX", "0x%04x", i8080->PC);
		replace_pattern(screen_text[i], "FH", " %1d", i8080->HALT);
		replace_pattern(screen_text[i], "FI", " %1d", i8080->INT);
	}

	for(int i = 0; i < MAX_FORMAT_HEIGHT; ++i) {
		for (int j = 0; j < 4 ; ++j) {
			char* ptr = strstr(screen_text[i], template_strings[j]);
			int offset = ptr - screen_text[i];
			if (ptr != NULL) {
				char* template_format = (char*)malloc(TEMPLATE_MAX_WIDTH[j] * sizeof(char));
				memset(template_format, ' ', TEMPLATE_MAX_WIDTH[j]);
				template_format[TEMPLATE_MAX_WIDTH[j] - 1] = '\0';
				memcpy(template_format, template_strings[j], strlen(template_strings[j]));

				int num = atoi(&screen_text[i][offset + strlen(template_strings[j])]);
				memcpy(template_format + strlen(template_strings[j]), &screen_text[i][offset + strlen(template_strings[j])], NUMBERS_MAX_WIDTH[j]);
				switch (j) {
				case 0: //PROGRAM+
					replace_program(i8080, screen_text[i], template_format, num, next_address);
					break;
				case 1: //PROGRAM-
					replace_program(i8080, screen_text[i], template_format, num, prev_address);
					break;
				case 2: //STDOUT
					replace_pattern(screen_text[i], template_format, "%-79s ", standard_output[num]);
					break;
				case 3: //MEM
					replace_memory(i8080, screen_text[i], template_format, num, memory_offset);
					break;
				}
				
				free(template_format);
			}
		}
	}
	for (int i = 0; i < MAX_FORMAT_HEIGHT; ++i) {
		std::cerr << screen_text[i];
	}
}
static inline void bdos_syscall(INTEL_8080* i8080) {
	if (standard_index > standard_index * (MAX_STDOUT_WIDTH - 1) * (MAX_FORMAT_HEIGHT - 1)) {
		for (int i = 0; i < MAX_STDOUT_HEIGHT; ++i) {
			if (standard_output[i] != NULL) {
				memset(standard_output[i], ' ', MAX_STDOUT_WIDTH);
			}
		}
		standard_index = 0;
	}

	switch (i8080->C) {
	case 2:  // C_WRITE
		if (i8080->E == '\n') {
			standard_index = ((standard_index / MAX_STDOUT_WIDTH) + 1) * MAX_STDOUT_WIDTH;
		}
		else if (i8080->E != '\r')
		{
			standard_output[standard_index / MAX_STDOUT_WIDTH][standard_index % MAX_STDOUT_WIDTH] = i8080->E;
			standard_index++;
		}
		break;
	case 9:
		WORD index = i8080->DE;
		while (i8080->MEM[index] != '$') {
			if (i8080->MEM[index] == '\n') {
				standard_index = ((standard_index / MAX_STDOUT_WIDTH) + 1) * MAX_STDOUT_WIDTH;
			}
			else if (i8080->MEM[index] != '\r')
			{
				standard_output[standard_index / MAX_STDOUT_WIDTH][standard_index % MAX_STDOUT_WIDTH] = i8080->MEM[index];
				standard_index++;
			}
			index++;
		}
		break;
	}
	prepare_screen(i8080);
}
static void instruction_print(
	INTEL_8080* i8080,
	BYTE instruction
) {
	printf("PC = %hXh    ", i8080->PC);
	printf(OPCODE_NAME[instruction], SWAPORDER(word_arg(i8080)));
	puts("");
}

static void register_print(
	INTEL_8080* i8080
) {
	WORD be = word_arg(i8080);
	printf("A=%hhX BC=%hX DE=%hX HL=%hX SP=%hX [S=%d Z=%d AC=%d P=%d C=%d]  ", i8080->A,
		i8080->BC, i8080->DE, i8080->HL, i8080->SP, i8080->status.S, i8080->status.Z,
		i8080->status.AC, i8080->status.P, i8080->status.C);
}

static inline int emulate(
	INTEL_8080* i8080,
	BOOL bdos
) {
	while (1) {
		std::cerr << "\033[H";
		//prepare_screen(i8080);
		if (i8080->INT && i8080->INT_PENDING) {
			//instruction_print(i8080, i8080->INT_VECTOR);
			i8080->SP -= 2;
			write_word_on_stack(i8080, i8080->PC);
			i8080->PC = i8080->INT_PENDING << 3;
			i8080->INT = 0;
			i8080->INT_PENDING = 0;
			i8080->INT_VECTOR = 0;
			i8080->HALT = 0;
			instruction_history.push(i8080->INT_VECTOR);
			if (instruction_history.size() > MAX_HISTORY_SIZE)
				instruction_history.pop();
			OPCODE_TABLE[i8080->INT_VECTOR](i8080);
		}
		else if (!i8080->HALT) {
			if (i8080->PC == 0x0005 && bdos)
				bdos_syscall(i8080);
			instruction_history.push(i8080->PC);
			if (instruction_history.size() > MAX_HISTORY_SIZE)
				instruction_history.pop();
			i8080->PC += OPCODE_TABLE[i8080->MEM[i8080->PC]](i8080);
		}
	}
	return 0;
}
static void init_screen() {
	setlocale(LC_ALL, "pl_PL.UTF-8");
	screen_format = (char**)malloc(MAX_FORMAT_HEIGHT * sizeof(char*));
	screen_text = (char**)malloc(MAX_FORMAT_HEIGHT * sizeof(char*));
	standard_output = (char**)malloc(MAX_STDOUT_WIDTH * sizeof(char*));

	for (int i = 0; i < MAX_FORMAT_HEIGHT; ++i) {
		if (screen_format != NULL) {
			screen_format[i] = (char*)malloc(MAX_FORMAT_WIDTH * sizeof(char));
		}
		if (screen_text != NULL) {
			screen_text[i] = (char*)malloc(MAX_FORMAT_WIDTH * sizeof(char));
		}
	}
	for (int i = 0; i < 14; ++i) {
		if (standard_output != NULL) {
			standard_output[i] = (char*)malloc(MAX_STDOUT_WIDTH * sizeof(char));
			if (standard_output[i] != NULL) {
				memset(standard_output[i], ' ', MAX_STDOUT_WIDTH);
			}
		}
	}
	prev_address = (uint16_t*)malloc((MAX_STDOUT_HEIGHT + 1) * sizeof(uint16_t));
	next_address = (uint16_t*)malloc((MAX_STDOUT_HEIGHT + 1) * sizeof(uint16_t));
}

static void destroy_screen() {
	free(screen_format);
	free(screen_text);
	free(standard_output);
	free(prev_address);
	free(next_address);
}

int main(int argc, char** argv) {
	INTEL_8080 i8080;
	init_screen();
	if (initialize(&i8080, 0x0100, 0x0000)) {
		fprintf(stderr, "Could not initialize INTEL_8080 structure\n");
		return 1;
	}
	i8080.MEM[0x0005] = 0xC9; // ret at bdos syscall
	i8080.MEM[0x0000] = 0x76; // hlt at 0x0000
	write_file_to_memory(&i8080, "8080EXER.COM", 0x0100);
	read_memory(&i8080, 0x0100, 128);
	read_screen_format(&i8080, screen_format);
	
	emulate(&i8080, SET);
	destroy_screen();
	destroy(&i8080);
}
