

#ifndef FILE_PROCESSOR_DIRECTORY_H
#define FILE_PROCESSOR_DIRECTORY_H
#include <stddef.h>
#include <string.h>

#include "util.h"
#include "file.h"

typedef struct {
	char name [2048];
	char extension [32];
	size_t size;
	time_t creation_time;
	char* file_path;
	int thread_number;
} DirEntity;




RESULT DIR_list_files(FileList* list, const char* path);
RESULT DIR_get_entity(DirEntity* entity, const char* path);
static inline void DIR_init(DirEntity* self, char* name, char* ext, size_t size, time_t creation_time, char* file_path, int thread_number) {
	strncpy(self->name, name, sizeof(self->name) - 1);
	self->name[sizeof(self->name) - 1] = '\0';
	strncpy(self->extension, ext, sizeof(self->extension) - 1);
	self->extension[sizeof(self->extension) - 1] = '\0';
	self->size = size;
	self->creation_time = creation_time;
	self->file_path = file_path;
	self->thread_number = thread_number;
}

#endif //FILE_PROCESSOR_DIRECTORY_H
