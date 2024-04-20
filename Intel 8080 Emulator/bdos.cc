#include "bdos.h"
#include <string.h>

void bdos_syscall(INTEL_8080* i8080, SCREEN* screen) {
	if (screen->standard_index / MAX_STDOUT_WIDTH >= MAX_STDOUT_HEIGHT) {
		for (int i = 0; i < MAX_STDOUT_HEIGHT; ++i) {
			if (screen->standard_output[i] != NULL) {
				memset(screen->standard_output[i], ' ', MAX_STDOUT_WIDTH);
			}
		}
		screen->standard_index = 0;
	}

	switch (i8080->C) {
	case 2:  // C_WRITE
		if (i8080->E == '\n') {
			screen->standard_index =
				((screen->standard_index / MAX_STDOUT_WIDTH) + 1) * MAX_STDOUT_WIDTH;
		}
		else if (i8080->E != '\r') {
			screen->standard_output[screen->standard_index / MAX_STDOUT_WIDTH][screen->standard_index % MAX_STDOUT_WIDTH] = i8080->E;
			screen->standard_index++;
		}
		break;
	case 9:
		uint16_t index = i8080->DE;
		while (i8080->MEM[index] != '$') {
			if (i8080->MEM[index] == '\n') {
				screen->standard_index = ((screen->standard_index / MAX_STDOUT_WIDTH) + 1) * MAX_STDOUT_WIDTH;
			}
			else if (i8080->MEM[index] != '\r')
			{
				screen->standard_output[screen->standard_index / MAX_STDOUT_WIDTH][screen->standard_index % MAX_STDOUT_WIDTH] = i8080->MEM[index];
				screen->standard_index++;
			}
			index++;
		}
		break;
	}
}
