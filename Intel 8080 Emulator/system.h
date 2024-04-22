#pragma once
#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#define getch _getch
typedef HANDLE THREAD;
#else
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
typedef pthread_t THREAD;
#endif

THREAD thread_create(void* (*worker)(void*), void* args);
void thread_destroy(THREAD thread);
void initialize_keys();
void cleanup_keys();
