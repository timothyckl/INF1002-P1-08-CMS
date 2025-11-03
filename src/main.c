#include <stdlib.h>

#include "utils.h"

// this number was arbitrarily chosen
#define MAX_LINE_LENGTH 256 
#define DECLARATION_FILE_PATH "declaration.txt"
#define SHOW_LINE_NUM true
#define LOGGING true

// TODO: implement parser (i'm thinking js use regex)
// TODO: implement structure to store data after parsing
int main(int argc, char *argv[]) {
  // check command line arguments
  if (check_args(argc, argv) != 0) { return EXIT_FAILURE; }

  // attempt to open file
  const char *file_path = argv[1];
  FILE *file_handle = get_file_handle(file_path);
  if (file_handle == NULL) { return EXIT_FAILURE; }
  if (LOGGING) { printf("Success opening file!\n"); }

  // reads file line by line
  print_file_lines(file_handle, MAX_LINE_LENGTH, SHOW_LINE_NUM);;

  return EXIT_SUCCESS;
}
