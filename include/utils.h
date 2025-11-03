#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdbool.h>

int check_args(int argc, char *argv[]);
FILE *get_file_handle(const char *file_path);
void print_file_lines(FILE* handle, int buffer_size, bool show_line_num);

#endif
