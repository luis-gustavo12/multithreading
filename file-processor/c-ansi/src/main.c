

#include <stdio.h>
#include <stdlib.h>

#include "report.h"
#include "platform_directory/platform_directory.h"
#include "file.h"


int main(int argc, char* argv[]) {


	if (argc <= 1)
		return 0;


	char* path = argv[1];
	FileList files;
	int files_count = 120;

	FILE_LIST_init(&files, files_count);

	int size = 120;

	if (DIR_list_files(&files, path) != SUCCESS)
		goto FINAL_CLEANUP;;

	Report report;

	if (REPORT_process(&report, &files, 4) != SUCCESS)
		goto FINAL_CLEANUP;


	REPORT_print(&report);

FINAL_CLEANUP:

	FILE_LIST_destroy(&files);
	REPORT_destroy(&report);

	return 0;
}