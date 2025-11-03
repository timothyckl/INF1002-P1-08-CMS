#include "utils.h"

int check_args(int argc, char *argv[]){
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <path-to-file>\n", argv[0]);
    return -1;
  }
  return 0;
}

FILE *open_file(const char *file_path) {
  FILE *file_handle = fopen(file_path, "r");

  if (file_handle == NULL) {
    fprintf(stderr, "Error opening file.\n");
    return NULL;
  }

  return file_handle;
}

void read_file(int buffer_size, FILE* handle, bool show_line_num){
  char buffer[buffer_size];
  int current_line = 0;

  while (fgets(buffer, buffer_size, handle) != NULL) {
    if (show_line_num) {
      current_line++;
      printf("Line %i: %s", current_line, buffer);
    } else { printf("%s", buffer); }
  }
}
