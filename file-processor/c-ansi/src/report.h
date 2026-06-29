
#ifndef FILE_PROCESSOR_REPORT_H
#define FILE_PROCESSOR_REPORT_H
#include <string.h>
#include "util.h"
#include "platform_directory/platform_directory.h"
#include "file.h"

typedef struct {
	unsigned int bytes;
	unsigned int files_count;
	FileList* list;
	DirEntity* entities;
	int entity_count;
} Report;

// Struct in which each row represents every file processed by n threads.
typedef struct {
	char* text;
	size_t text_capacity;
	size_t current_len;
	int thread_id;
} DisplayRow;

typedef struct {
	DisplayRow* rows;
	int rows_count;
	int rows_capacity;
} ThreadDisplayArray;


RESULT ROW_format_by_file_name(DisplayRow* display_row, const char* file_name);

RESULT THREAD_DISPLAY_ARR_init(ThreadDisplayArray* array, int initial_capacity);
RESULT THREAD_DISPLAY_ARR_destroy(ThreadDisplayArray* self);
void THREAD_DISPLAY_ARR_add_element(ThreadDisplayArray* self, const char* file_name, int thread_id);
RESULT THREAD_DISPLAY_to_string(const ThreadDisplayArray* self, char** buffer);

RESULT REPORT_process(Report* self, FileList* file_list, unsigned int number_of_threads);
void REPORT_print(const Report* self);
RESULT REPORT_update_info(Report* self, unsigned int bytes);
RESULT REPORT_destroy(Report* self);

#endif //FILE_PROCESSOR_REPORT_H
