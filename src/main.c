#include <stdio.h>
#include <stdlib.h>

#include "database.h"
#include "utils.h"

// TODO: abstract away public facing interface with simpler API calls

// TODO handle QUERY command:
// - ensure a database has been opened
// - prompt for student ID input, validate numeric range 0-9,999,999
// - call helper to locate the record
// - print record details if found, otherwise warn user

// TODO helper:
//   find_record_by_id(StudentDatabase*, int id)
//     loop tables -> loop records -> compare id
//     return pointer or NULL

// TODO handle UNIQUE FEATURE command:
// - ensure database loaded
// - gather any extra user input specific to the feature
// - compute result using in-memory tables/records
// - display formatted output to the user

int main(int argc, char *argv[]) {
  // check command line arguments
  if (check_args(argc, argv) != 0) {
    return EXIT_FAILURE;
  }

  // initialise database
  StudentDatabase *db = db_init();
  if (!db) {
    fprintf(stderr, "Failed to initialise database\n");
    return EXIT_FAILURE;
  }

  // load database from file
  const char *file_path = argv[1];
  DBStatus status = db_load(db, file_path);
  if (status != DB_SUCCESS) {
    fprintf(stderr, "Failed to load database: %s\n", db_status_string(status));
    db_free(db);
    return EXIT_FAILURE;
  }

  // print database metadata
  printf("database: %s\n", db->db_name);
  printf("authors: %s\n", db->authors);
  printf("\ntables: %zu\n", db->table_count);

  // print each table hierarchically
  for (size_t t = 0; t < db->table_count; t++) {
    StudentTable *table = db->tables[t];

    printf("\ntable %zu: %s\n", t + 1, table->table_name);

    // print column headers
    printf("columns: ");
    for (size_t c = 0; c < table->column_count; c++) {
      printf("%s", table->column_headers[c]);
      if (c < table->column_count - 1) {
        printf(", ");
      }
    }
    printf("\n");

    // print record count and records
    printf("records: %zu\n", table->record_count);
    for (size_t i = 0; i < table->record_count; i++) {
      StudentRecord *r = &table->records[i];
      printf("  %d\t%s\t%s\t%.1f\n", r->id, r->name, r->prog, r->mark);
    }
  }

  // cleanup
  db_free(db);
  return EXIT_SUCCESS;
}
