#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLSIZE 16 // number of columns
#define ASCII   1  // displays ascii characters on the right of the hex dump

int memdump(const void* ptr, size_t bytes) {
	/*copy n bytes from ptr to dump*/
	unsigned char* dump = (unsigned char*)calloc(bytes, sizeof(char));
	if (dump == NULL)
		return 0;
	memcpy(dump, ptr, bytes);

	for (size_t i = 0; i < bytes; i++) {

		printf("%02X ", dump[i]);

		if (i % COLSIZE == COLSIZE - 1) {
			if (ASCII) {
				printf("\t");
				for (size_t j = i - COLSIZE + 1; j <= i; j++) {
					if (dump[j] >= 0x20 && dump[j] < 0x7F)
						printf("%c", dump[j]);
					else
						printf(".");
				}
			}
			puts("");
		}
	}

	if (ASCII) {
		const unsigned int rest = bytes % COLSIZE;
		for (unsigned int i = 0; i < COLSIZE - rest; i++) {
			printf("   ");
		}
		printf("\t");
		for (size_t i = bytes - rest; i < bytes; i++) {
			if (dump[i] >= 0x20 && dump[i] < 0x7F)
				printf("%c", dump[i]);
			else
				printf(".");
		}
	}
	puts("\n");

	free(dump);
	return 1;
}