#include <stdlib.h>

#include "utils.h"

// this number was arbitrarily chosen
#define MAX_LINE_LENGTH 256 
#define DECLARATION_FILE_PATH "declaration.txt"
#define SHOW_LINE_NUM true

// typedef struct {
//   int id;
//   char name[50];
//   char prog[50]; 
//   float mark;
// } Student;

// pattern for first few rows: Database Name:\s*(.+)
// pattern for actual table: [^\t\r\n]+

// // Example:
// Student s = {1, "tim", "Applied AI", 95.5};
// printf("id: %i, name: %s, prog: %s, mark: %.2f\n", s.id, s.name, s.prog, s.mark);

int main(int argc, char *argv[]) {
  // check command line arguments
  if (check_args(argc, argv) != 0) { return EXIT_FAILURE; }

  // attempt to open file
  const char *file_path = argv[1];
  FILE *file_handle = open_file(file_path);
  if (file_handle == NULL) { return EXIT_FAILURE; }
  printf("Success opening file!\n");

  // reads file line by line
  read_file(MAX_LINE_LENGTH, file_handle, SHOW_LINE_NUM);;

  // close file after usage
  fclose(file_handle);

  return EXIT_SUCCESS;
}
