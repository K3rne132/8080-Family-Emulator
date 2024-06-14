#pragma once
#ifndef EMULATOR
#define EMULATOR
#define I8085
#define CLK_MORE 1
#define CLK_LESS -1
#endif
#include "i8080.h"

// Set Interrupt Mask bits
typedef struct _SIM_BITS {
	uint8_t SOD : 1;
	uint8_t SOE : 1;
	uint8_t : 1;
	uint8_t RST75 : 1;
	uint8_t MSE : 1;
	uint8_t M75 : 1;
	uint8_t M65 : 1;
	uint8_t M55 : 1;
} SIM_BITS;

// Read Interrupt Mask bits
typedef struct _RIM_BITS {
	uint8_t SID : 1;
	uint8_t P75 : 1;
	uint8_t P65 : 1;
	uint8_t P55 : 1;
	uint8_t IE  : 1;
	uint8_t M75 : 1;
	uint8_t M65 : 1;
	uint8_t M55 : 1;
} RIM_BITS;

enum HARDWARE_INTERRUPT {
	TRAP = 0x24,
	RST_75 = 0x3C,
	RST_65 = 0x34,
	RST_55 = 0x2C
};

// Intel 8085 registers set
typedef struct _INTEL_8085 {
	INTEL_8080 CORE;
	RIM_BITS RIM;
} INTEL_8085;

uint16_t sim(INTEL_8080* i8080);
uint16_t rim(INTEL_8080* i8080);

uint16_t arhl(INTEL_8080* i8080);
uint16_t dsub(INTEL_8080* i8080);
uint16_t jnui(INTEL_8080* i8080);
uint16_t jui(INTEL_8080* i8080);
uint16_t ldhi(INTEL_8080* i8080);
uint16_t ldsi(INTEL_8080* i8080);
uint16_t lhlx(INTEL_8080* i8080);
uint16_t rdel(INTEL_8080* i8080);
uint16_t shlx(INTEL_8080* i8080);
