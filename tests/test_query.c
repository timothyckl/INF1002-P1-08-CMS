/*
 * test_query.c
 *
 * Test harness for the basic QUERY operation.
 * Tests the search logic for finding student records by ID.
 *
 * Build command:
 *   clang -Iinclude -Wall -Wextra -g src/{adv_query,cms,database,parser,sorting,utils}.c tests/test_query.c -o build/test_query.exe
 *
 * Run command:
 *   ./build/test_query.exe
 */

#include <stdio.h>
#include <stdlib.h>

#include "database.h"

// lightweight lookup helper mirroring the QUERY operation
static StudentRecord *find_record_by_id(StudentDatabase *db, int id) {
  if (!db || db->table_count == 0) {
    return NULL;
  }

  for (size_t t = 0; t < db->table_count; t++) {
    StudentTable *table = db->tables[t];
    if (!table) {
      continue;
    }

    for (size_t r = 0; r < table->record_count; r++) {
      if (table->records[r].id == id) {
        return &table->records[r];
      }
    }
  }

  return NULL;
}

static void print_record(const StudentRecord *record) {
  if (!record) {
    return;
  }
  printf("  %d\t%s\t%s\t%.2f\n", record->id, record->name, record->prog,
         record->mark);
}

static void expect_null(StudentRecord *record, const char *label) {
  if (record == NULL) {
    printf("[OK] %s returned NULL as expected\n", label);
  } else {
    printf("[FAIL] %s expected NULL but got ID=%d\n", label, record->id);
  }
}

static void run_lookup(StudentDatabase *db, int id) {
  // wrap the helper to show the result in a readable way
  StudentRecord *record = find_record_by_id(db, id);
  if (!record) {
    printf("[WARN] No record found for ID=%d\n", id);
    return;
  }
  printf("[OK] Found record for ID=%d\n", id);
  printf("  ID\tName\tProgramme\tMark\n");
  print_record(record);
}

int main(void) {
  printf("Running query search logic tests...\n\n");

  expect_null(find_record_by_id(NULL, 1234567), "NULL database pointer");

  StudentDatabase *empty = db_init();
  if (!empty) {
    fprintf(stderr, "Failed to initialise empty database\n");
    return EXIT_FAILURE;
  }
  expect_null(find_record_by_id(empty, 2301234),
              "Database initialised but not loaded");
  db_free(empty);

  StudentDatabase *db = db_init();
  if (!db) {
    fprintf(stderr, "Failed to initialise database\n");
    return EXIT_FAILURE;
  }

  DBStatus status = db_load(db, "data/P1_8-CMS.txt", NULL);
  if (status != DB_SUCCESS) {
    fprintf(stderr, "Failed to load database: %s\n", db_status_string(status));
    db_free(db);
    return EXIT_FAILURE;
  }

  run_lookup(db, 2301234);
  run_lookup(db, 2201234);
  run_lookup(db, 9999999);

  db_free(db);
  return EXIT_SUCCESS;
}
