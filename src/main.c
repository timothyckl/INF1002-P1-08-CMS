#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 256 

int main(int argc, char *argv[]) {
  // checks the number of command-line arguments
  if (argc < 2) {
    printf("Usage: %s <path-to-file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *file_path = argv[1];

  // attempt to open file
  FILE *file_handle;

  file_handle = fopen(file_path, "r");

  if (file_handle == NULL) {
    printf("Error opening file.\n");
    return EXIT_FAILURE;
  }

  printf("Success opening file!\n");

  char buffer[MAX_LINE_LENGTH];
  int current_line = 0;

  // reads file line by line
  while (fgets(buffer, MAX_LINE_LENGTH, file_handle) != NULL) {
    current_line++;
    printf("Line %i: %s", current_line, buffer);
  }

  // close file after usage
  fclose(file_handle);

  return EXIT_SUCCESS;
}
