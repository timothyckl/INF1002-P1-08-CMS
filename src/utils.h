#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdbool.h>

int check_args(int argc, char *argv[]);
FILE *open_file(const char *file_path);
void read_file(int buffer_size, FILE* handle, bool show_line_num);

#endif
