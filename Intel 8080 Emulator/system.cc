#include "system.h"
#include <locale.h>
#ifdef _WIN32

THREAD thread_create(void* (*worker)(void*), void* args) {
	return CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)worker,
		args,
		0,
		NULL
	);
}

void thread_destroy(THREAD thread) {
	CloseHandle(thread);
}

void thread_sleep(uint32_t milliseconds) {
	Sleep(milliseconds);
}

void initialize_keys() {}

void cleanup_keys() {}

void initialize_screen() {
	setlocale(LC_ALL, "pl_PL.UTF-8");
	HANDLE hstdout = NULL;
	CONSOLE_CURSOR_INFO info = { 0 };

	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleCursorInfo(hstdout, &info);
	info.bVisible = FALSE;
	SetConsoleCursorInfo(hstdout, &info);
}

void cleanup_screen() {
	HANDLE hstdout = NULL;
	CONSOLE_CURSOR_INFO info = { 0 };

	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleCursorInfo(hstdout, &info);
	info.bVisible = TRUE;
	SetConsoleCursorInfo(hstdout, &info);
}

#else

THREAD thread_create(void* (*worker)(void*), void* args) {
	pthread_t thread = NULL;
	pthread_create(&thread, NULL, worker, args);
	return thread;
}

void thread_destroy(THREAD thread) {
	pthread_cancel(thread);
}

void initialize_keys() {
	initscr();
	cbreak();
	noecho();
}

void cleanup_keys() {
	endwin();
}

void thread_sleep(uint32_t milliseconds) {
	msleep(milliseconds);
}

void initialize_screen() {}

#endif
