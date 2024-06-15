#pragma once
#include "i8085.h"

int i8085_initialize(
	INTEL_8085* i8085,
	const uint16_t origin_pc,
	const uint16_t origin_sp
);

void i8085_destroy(
	INTEL_8085* i8085
);

int hardware_interrupt(
	INTEL_8085* i8085,
	HARDWARE_INTERRUPT interrupt
);

int intr(
	INTEL_8085* i8085,
	uint16_t address
);

void set_serial_in(
	INTEL_8085* i8085
);

void reset_serial_in(
	INTEL_8085* i8085
);
