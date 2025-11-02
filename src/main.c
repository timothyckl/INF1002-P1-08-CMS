#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 256 // this number was arbitrarily chosen

// function prototypes
int check_args(int argc, char *argv[]);
FILE *open_file(const char *file_path);
// read_file(file_handle)

int main(int argc, char *argv[]) {
  // check command line arguments
  if (check_args(argc, argv) != 0) {
    return EXIT_FAILURE;
  }

  // attempt to open file
  const char *file_path = argv[1];
  FILE *file_handle = open_file(file_path);
  
  if (file_handle == NULL) {
    return EXIT_FAILURE;
  }

  printf("Success opening file!\n");

  // reads file line by line
  char buffer[MAX_LINE_LENGTH];
  int current_line = 0;

  while (fgets(buffer, MAX_LINE_LENGTH, file_handle) != NULL) {
    current_line++;
    printf("Line %i: %s", current_line, buffer);
  }

  // close file after usage
  fclose(file_handle);

  return EXIT_SUCCESS;
}

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
