

#include <stdio.h>

#include "platform_directory.h"
#include "windows.h"


RESULT DIR_list_files(FileList* list, const char* path) {

	WIN32_FIND_DATA find;
	HANDLE h;
	char search_path [MAX_PATH];
	int index = 0;

	snprintf(search_path, sizeof(search_path), "%s\\*", path);

	h = FindFirstFileA(search_path, &find);
	if (h == INVALID_HANDLE_VALUE) {
		return FAILURE;
	}

	do {
		if (!strcmp(find.cFileName, ".") || !strcmp(find.cFileName, "..")) {
			continue;
		}

		if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}

		if (list->count >= list->capacity) {
			int new_cap = list->capacity * 2;
			FileUtil* temp = realloc(list->data, new_cap * sizeof(FileUtil));
			if (!temp) {
				FindClose(h);
				return FAILURE;
			}
			list->data = temp;
			list->capacity = new_cap;
		}

		FileUtil file_util;
		strcpy(file_util.file_name, find.cFileName);

		char* full_path = NULL;
		size_t needed_size = strlen(path) + strlen("\\") + strlen(find.cFileName) + 1;

		full_path = malloc(needed_size);
		if (!full_path) {
			FindClose(h);
			return FAILURE;
		}
		full_path[0] = '\0';
		sprintf(full_path, "%s\\%s", path, find.cFileName);
		file_util.full_path = full_path;

		list->data[list->count] = file_util;
		list->count++;

	}
	while (FindNextFile(h, &find));
	FindClose(h);
	return SUCCESS;
}

RESULT DIR_get_entity(DirEntity* entity, const char* path) {

	char search_path [3000];

	sprintf(search_path, "%s", path);

	WIN32_FIND_DATA find;
	HANDLE h = FindFirstFileA(search_path, &find);

	if (h == INVALID_HANDLE_VALUE)
		return FAILURE;

	size_t file_size = ((size_t)find.nFileSizeHigh << 32) | find.nFileSizeLow;

	ULARGE_INTEGER ull;

	ull.LowPart = find.ftCreationTime.dwLowDateTime;
	ull.HighPart = find.ftCreationTime.dwHighDateTime;
	time_t creation_time = (time_t)((ull.QuadPart - 116444736000000000ULL) / 10000000ULL);

	char* ext = strrchr(find.cFileName, '.');

	if (ext)
		ext++;
	else
		"";

	DIR_init(entity, find.cFileName, ext, file_size, creation_time, path, 0);

	FindClose(h);
	return SUCCESS;
}
