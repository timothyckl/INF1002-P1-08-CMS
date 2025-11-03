#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdbool.h>

// NOTES: not sure if this is the appropriate place for these functions
//       but i'll leave them here for now :000
int check_args(int argc, char *argv[]);
FILE *get_file_handle(const char *file_path);
void print_file_lines(FILE* handle, int buffer_size, bool show_line_num);

#endif
