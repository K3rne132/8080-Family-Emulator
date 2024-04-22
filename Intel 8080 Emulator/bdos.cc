#include "bdos.h"
#include <string.h>

void bdos_syscall(INTEL_8080* i8080, DBG_CONSOLE* screen) {
	switch (i8080->C) {
	case 2:  // C_WRITE
		put_character(screen, i8080->E);
		break;
	case 9: // C_WRITESTR
		uint16_t index = i8080->DE;
		while (i8080->MEM[index] != '$') {
			put_character(screen, i8080->MEM[index]);
			index++;
		}
		break;
	}
}
