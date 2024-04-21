#include "system.h"
#ifdef _WIN32

THREAD thread_create(void* worker, void* args) {
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

void initialize_keys() {}

void cleanup_keys() {}

#else

THREAD thread_create(void* worker, void* args) {
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

#endif
