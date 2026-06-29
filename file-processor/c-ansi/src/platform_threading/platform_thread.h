#ifndef FILE_PROCESSOR_PLATFORM_THREAD_H
#define FILE_PROCESSOR_PLATFORM_THREAD_H


#include "util.h"

typedef void* (*THREAD_Function)(void* arg);

typedef struct {
	void* handle;
} THREAD_Handle;

typedef struct {
	void* mutex_handle;
} THREAD_MUTEX_Handle;

typedef struct {
	THREAD_Function executor;
	void* args;
} THREAD_Context;

RESULT THREAD_init(THREAD_Handle* handle, THREAD_Function executor, void* arg);
RESULT THREAD_wait(THREAD_Handle* handle);
RESULT THREAD_destroy(THREAD_Handle* handle);

RESULT THREAD_MUTEX_create(THREAD_MUTEX_Handle* handle);
RESULT THREAD_MUTEX_lock(THREAD_MUTEX_Handle* handle);
RESULT THREAD_MUTEX_release(THREAD_MUTEX_Handle* handle);
RESULT THREAD_MUTEX_destroy(THREAD_MUTEX_Handle* handle);

#endif //FILE_PROCESSOR_PLATFORM_THREAD_H
