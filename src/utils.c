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
