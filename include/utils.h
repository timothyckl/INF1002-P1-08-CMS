#ifndef UTILS_H
#define UTILS_H

/*
 * utilities module
 *
 * provides helper functions for command-line argument handling,
 * file operations, and formatted output.
 */

#include <stdbool.h>
#include <stdio.h>

// validates command-line arguments
int check_args(int argc, char *argv[]);

// opens a file and returns file handle
FILE *get_file_handle(const char *file_path);

// prints file contents line by line with optional line numbers
void print_file_lines(FILE *handle, int buffer_size, bool show_line_num);

#endif
