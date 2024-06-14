#pragma once
#include <inttypes.h>
#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#define getch _getch
typedef HANDLE THREAD;
#else
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <linux/delay.h>
typedef pthread_t THREAD;
#endif

THREAD thread_create(void* (*worker)(void*), void* args);
void thread_destroy(THREAD thread);
void thread_sleep(uint32_t milliseconds);
void initialize_screen();
void cleanup_screen();
void initialize_keys();
void cleanup_keys();
