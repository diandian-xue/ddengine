#include "ddclfile.h"

#include <stdio.h>

void t_file(){

	printf("ddclfile test start ----------\n\n");

	const char * bin_path = "../bin";
	const char * exc_path = "../bin/ddcltest";

	printf("cwd:%s\n", ddcl_cwd());
	printf("is_dir %s:%d\n", bin_path, ddcl_is_dir(bin_path));
	printf("is_dir %s:%d\n", exc_path, ddcl_is_dir(exc_path));

	char bin_full_path[DDCL_MAX_PATH] = {0};
	ddcl_full_path(bin_path, bin_full_path);
	printf("full_path %s:%s\n", bin_path, bin_full_path);
	printf("is_absolute_path %s:%d\n", bin_path, ddcl_path_absolute(bin_path));
	printf("is_absolute_path %s:%d\n", bin_full_path, ddcl_path_absolute(bin_full_path));

	size_t exc_sz = 0;
	ddcl_file_size(exc_path, &exc_sz);
	printf("FileSize %s:%lld\n", exc_path, exc_sz);

	ddcl_FileSearch fs;
	ddcl_init_file_search(&fs);
	ddcl_add_path(&fs, bin_path);
	char find_exc_path[DDCL_MAX_PATH] = {0};
	int fret = ddcl_find_file(&fs, exc_path, find_exc_path);
	printf("find_in_file_search %s:  %d => %s\n", exc_path, fret, find_exc_path);

	printf("\nddclfile test ended ----------\n\n");
}