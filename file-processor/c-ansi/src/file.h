
#ifndef FILE_PROCESSOR_FILE_H
#define FILE_PROCESSOR_FILE_H

#include <stdlib.h>

#include "util.h"

// Small struct I created to simplify file handling. Since I get a list of files, it's better to create a struct
// that contains both the file name, and the Full path. E.g., let'1s say I have a data.pdf file at C:/users/myuser/Downloads. Sometimes
// For processing, only data.pdf gets lost in the context, and it's now known the full path
typedef struct {
	char file_name [512];
	char* full_path;
} FileUtil;

inline void FILE_add(FileUtil* file_util, const char* file_name, const char* full_path) {
	strcpy(file_util->file_name, file_name);
	file_util->full_path = (char*)full_path;
}

typedef struct {
	FileUtil* data;
	int count;
	int capacity;
} FileList;

static inline void FILE_LIST_init(FileList* list, int initial_capacity) {
	list->capacity = initial_capacity;
	list->count = 0;
	list->data = calloc(initial_capacity, sizeof(FileUtil));
}

static inline void FILE_LIST_destroy(FileList* list) {
	for (int i = 0; i < list->count; i++) {
		free(list->data[i].full_path);
	}
	free(list->data);
}

#endif //FILE_PROCESSOR_FILE_H
