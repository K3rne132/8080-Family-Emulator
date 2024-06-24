#include "bdos.h"
#include "system.h"
#include <string.h>
#include <stdio.h>

void bdos_syscall(INTEL_8080* i8080, DBG_CONSOLE* screen) {
	uint16_t index = i8080->DE;
	switch (i8080->C) {
	case 1:  // C_READ
		i8080->L = getch();
		i8080->A = i8080->L;
		put_character(screen, i8080->A);
		break;
	case 2:  // C_WRITE
		put_character(screen, i8080->E);
		break;
	case 9: // C_WRITESTR
		while (i8080->MEM[index] != '$') {
			put_character(screen, i8080->MEM[index]);
			index++;
		}
		break;
	}
}
