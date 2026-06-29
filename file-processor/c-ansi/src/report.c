#include "report.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include "platform_directory/platform_directory.h"
#include "platform_threading/platform_thread.h"


#define DISPLAY_INITIAL_ROW_TEXT "Thread of id %d contains: \n"

typedef struct {
	int* shared_counter; // processed files counter
	THREAD_MUTEX_Handle* handle;
	FileList* files; // array that contains the file arrays
	Report* report;
	int thread_id;
} ReportArgs;

static void* process_report(void* arg) {

	ReportArgs* args = (ReportArgs*) arg;

	while (1) {

		THREAD_MUTEX_lock(args->handle);

		int current_index = *(args->shared_counter);

		if (current_index >= args->files->count) {
			THREAD_MUTEX_release(args->handle);
			break;
		}

		const char* file_path = args->files->data[current_index].full_path;

		if (file_path) {

			DirEntity entity;

			DIR_get_entity(&entity, file_path);

			Report* report = args->report;
			report->files_count++;
			report->bytes += entity.size;
			entity.thread_number = args->thread_id;
			report->entities[report->entity_count] = entity;
			report->entity_count++;
		}

		(*args->shared_counter)++;

		THREAD_MUTEX_release(args->handle);

	}

	return 0;
}


RESULT ROW_format_by_file_name(DisplayRow* display_row, const char* file_name) {

	if (display_row == NULL)
		return FAILURE;

	if (display_row->text == NULL || display_row->current_len == 0) {

		display_row->text = malloc(256);
		display_row->text_capacity = 256;

		if (!display_row->text)
			return FAILURE;

		display_row->text[0] = '\0';

		display_row->current_len = sprintf(display_row->text, "The thread with id %d processed files: [%s]", display_row->thread_id, file_name);

		return SUCCESS;

	}

	size_t file_name_len = strlen(file_name);
	char temp_str [file_name_len + 5];

	size_t temp_str_len = sprintf(temp_str, ", [%s]", file_name);

	size_t new_necessary_size = display_row->current_len + temp_str_len + 1;

	if (new_necessary_size > display_row->text_capacity) {
		size_t new_capacity = display_row->text_capacity * 2;
		if (new_capacity < new_necessary_size) {
			new_capacity = new_necessary_size;
		}

		char* new_text = realloc(display_row->text, new_capacity);
		if (!new_text)
			return FAILURE;

		display_row->text = new_text;
		display_row->text_capacity = new_capacity;
	}

	memcpy(display_row->text + display_row->current_len, temp_str, temp_str_len + 1);
	display_row->current_len += temp_str_len;

	return SUCCESS;

}

RESULT THREAD_DISPLAY_ARR_init(ThreadDisplayArray* array, int initial_capacity) {


	const int capacity = initial_capacity > 0 ? initial_capacity : 250;

	DisplayRow* rows = calloc(
		capacity,
		sizeof(DisplayRow)
		);

	if (!rows)
		return FAILURE;

	array->rows = rows;
	array->rows_count = 0;
	array->rows_capacity = capacity;
	return SUCCESS;

}

RESULT THREAD_DISPLAY_ARR_destroy(ThreadDisplayArray* self) {
	for (int i = 0; i < self->rows_capacity; i++) {
		if (self->rows[i].text != NULL) {
			free(self->rows[i].text);
		}
	}
	free(self->rows);
	return SUCCESS;
}

void THREAD_DISPLAY_ARR_add_element(ThreadDisplayArray* self, const char* file_name, int thread_id) {

	if (self->rows_count == 0) {
		DisplayRow new_row;
		new_row.text = NULL;
		new_row.text_capacity = 0;
		new_row.current_len = 0;
		new_row.thread_id = thread_id;

		ROW_format_by_file_name(&new_row, file_name);

		self->rows[0] = new_row;
		self->rows_count++;
		return;
	}

	bool found = false;
	int i = 0;
	DisplayRow* row_with_same_thread_id = NULL;

	for (; i < self->rows_count; i++) {

		if (thread_id == self->rows[i].thread_id) {
			found = true;
			row_with_same_thread_id = &self->rows[i];
			break;
		}
	}

	if (found) {
		ROW_format_by_file_name(row_with_same_thread_id, file_name);
	} else {
		DisplayRow new_row;
		new_row.text = NULL;
		new_row.text_capacity = 0;
		new_row.current_len = 0;
		new_row.thread_id = thread_id;

		ROW_format_by_file_name(&new_row, file_name);

		self->rows[self->rows_count] = new_row;
		self->rows_count++;
	}
}

RESULT THREAD_DISPLAY_to_string(const ThreadDisplayArray* self, char** buffer) {

	if (self == NULL || buffer == NULL || *buffer == NULL)
		return FAILURE;

	size_t current_len = strlen(*buffer);
	size_t total_capacity = current_len + 256;
	char* initial_realloc = realloc(*buffer, total_capacity);
	if (!initial_realloc)
		return FAILURE;

	*buffer = initial_realloc;

	for (int i = 0; i < self->rows_capacity; i++) {

		if (self->rows[i].text == NULL)
			continue;

		const DisplayRow row = self->rows[i];

		size_t text_len = strlen(self->rows[i].text);
		size_t new_necessary_size = current_len + text_len + 2;

		if (new_necessary_size > total_capacity) {
			total_capacity = new_necessary_size * 2;
			char* temp = realloc(*buffer, total_capacity);
			if (!temp) {
				return FAILURE;
			}
			*buffer = temp;
		}

		strcat(*buffer, self->rows[i].text);
		strcat(*buffer, "\n");
		current_len += text_len + 1;

	}

	return SUCCESS;

}


RESULT REPORT_process(Report* self, FileList* file_list, unsigned int number_of_threads) {

	if (number_of_threads == 0)
		return FAILURE;

	self->list = file_list;
	self->entities = calloc(file_list->capacity, sizeof(DirEntity));
	if (!self->entities)
		return FAILURE;

	self->entity_count = 0;

	THREAD_Handle handles [300];
	int processed_files = 0;
	int handles_counter = 0;

	THREAD_MUTEX_Handle handle;
	if (THREAD_MUTEX_create(&handle) != SUCCESS) {
		return FAILURE;
	}

	ReportArgs* args_array = malloc(number_of_threads * sizeof(ReportArgs));

	if (!args_array) {
		THREAD_MUTEX_destroy(&handle);
		return FAILURE;
	}

	for (int i = 0; i < number_of_threads; i++) {

		THREAD_Handle thread_handle;
		args_array[i].shared_counter = &processed_files;
		args_array[i].handle = &handle;
		args_array[i].files = file_list;
		args_array[i].report = self;
		args_array[i].thread_id = i;

		THREAD_init(
			&thread_handle,
			process_report,
			&args_array[i]
		);

		handles[handles_counter] = thread_handle;
		handles_counter++;
	}

	for (int i = 0; i < handles_counter; i++) {
		THREAD_wait(&handles[i]);
	}

	for (int i = 0; i < handles_counter; i++) {
		THREAD_destroy(&handles[i]);
	}


	THREAD_MUTEX_destroy(&handle);
	free(args_array);

	return SUCCESS;
}

void REPORT_print(const Report* self) {

	char* dbg_string = malloc(12000);

	if (!dbg_string)
		return;

	dbg_string[0] = '\0';

	sprintf(dbg_string, "Files found: %d | Total bytes: %d KB(s)\n", self->files_count, (int)(self->bytes / 1024));

	ThreadDisplayArray display_array;

	if (THREAD_DISPLAY_ARR_init(&display_array, 120) != SUCCESS) {
		free(dbg_string);
		return;
	}


	for (int i = 0; i < self->entity_count; i++) {

		const DirEntity entity = self->entities[i];

		THREAD_DISPLAY_ARR_add_element(&display_array, entity.name, entity.thread_number);

	}

	THREAD_DISPLAY_to_string(&display_array, &dbg_string);

	printf("%s\n", dbg_string);

	THREAD_DISPLAY_ARR_destroy(&display_array);
	free(dbg_string);
}

RESULT REPORT_destroy(Report* self) {
	free(self->entities);
	return SUCCESS;
}

RESULT REPORT_create(Report* self, unsigned int bytes) {
	self->files_count++;
	self->bytes += bytes;

	return SUCCESS;
}
