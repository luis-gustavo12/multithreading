

#include "platform_thread.h"
#include <windows.h>
#include <stdlib.h>

typedef struct {
	THREAD_Function fn;
	void* args;
} ThreadContext;

static DWORD WINAPI trampoline(LPVOID raw) {

	ThreadContext* ctx = (ThreadContext*) raw;
	ctx->fn(ctx->args);
	free(ctx);
	return 0;
}


RESULT THREAD_init(THREAD_Handle* handle, THREAD_Function execution, void* arg) {

	ThreadContext* begin_context = malloc(sizeof(*begin_context));

	if (!begin_context)
		return FAILURE;

	begin_context->fn = execution;
	begin_context->args = arg;

	HANDLE thread = CreateThread(
		NULL,
		0,
		trampoline,
		begin_context,
		0,
		NULL
	);

	if (!thread) {
		free(begin_context);
		return FAILURE;
	}


	handle->handle = thread;
	return SUCCESS;


}

RESULT THREAD_wait(THREAD_Handle* handle) {

	DWORD rsl = WaitForSingleObject((HANDLE)handle->handle, INFINITE);
	return (rsl == WAIT_OBJECT_0) ? SUCCESS : FAILURE;
}

RESULT THREAD_destroy(THREAD_Handle* handle) {

	if (handle->handle == NULL)
		return SUCCESS;

	BOOL rsl = CloseHandle(handle->handle);
	handle->handle = NULL;

	return rsl ? SUCCESS : FAILURE;

}

RESULT THREAD_MUTEX_create(THREAD_MUTEX_Handle* handle) {

	HANDLE mutex_handle = CreateMutex(NULL, FALSE, NULL);

	if (!mutex_handle)
		return FAILURE;

	handle->mutex_handle = mutex_handle;
	return SUCCESS;
}

RESULT THREAD_MUTEX_lock(THREAD_MUTEX_Handle* handle) {

	if (!handle)
		return FAILURE;

	DWORD wait_result = WaitForSingleObject(handle->mutex_handle, INFINITE);

	if (wait_result == WAIT_OBJECT_0)
		return SUCCESS;

	return FAILURE;
}

RESULT THREAD_MUTEX_release(THREAD_MUTEX_Handle* handle) {

	if (!handle || !handle->mutex_handle)
		return FAILURE;

	if (ReleaseMutex(handle->mutex_handle))
		return SUCCESS;

	return FAILURE;

}

RESULT THREAD_MUTEX_destroy(THREAD_MUTEX_Handle* handle) {

	if (!handle || !handle->mutex_handle)
		return FAILURE;

	CloseHandle(handle->mutex_handle);
	handle->mutex_handle = NULL;
	return SUCCESS;
}


