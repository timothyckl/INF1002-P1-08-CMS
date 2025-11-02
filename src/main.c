#include <stdlib.h>

#include "utils.h"

// this number was arbitrarily chosen
#define MAX_LINE_LENGTH 256 

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
