#include <stdio.h>
#include <stdlib.h>

#include "database.h"

/*
 * CLI-level behaviours that shoudl be addressed handled outside this helper:
 * - parsing user input strings into integer IDs
 * - rejecting non-numeric or out-of-range IDs
 * - ensuring the database file has been opened before QUERY
 * - printing user-facing success or warning messages
 * - command dispatch / REPL loop control
 */

static void print_record(const StudentRecord *record) {
  if (!record) {
    return;
  }
  // match main.c single-line tab-separated record format
  printf("  %d\t%s\t%s\t%.1f\n", record->id, record->name, record->prog, record->mark);
}

static void expect_null_record(StudentRecord *record, const char *case_name) {
  // helper to assert scenarios where NULL is expected
  if (record == NULL) {
    printf("[OK] %s returned NULL as expected\n\n", case_name);
  } else {
    printf("[FAIL] %s expected NULL but got ID=%d\n\n", case_name, record->id);
  }
}

static void run_query(StudentDatabase *db, int id) {
  // hit/miss tests using real data loaded from disk
  StudentRecord *record = db_find_record_by_id(db, id);
  if (record) {
    printf("[OK] Found record for ID=%d:\n", id);
    // print header row to mirror main's field order
    printf("  ID\tName\tProgramme\tMark\n");
    print_record(record);
  } else {
    printf("[WARN] No record found for ID=%d\n", id);
  }
  printf("\n");
}

int main(void) {
  printf("Running db_find_record_by_id helper tests...\n\n");

  // case 1: NULL database pointer should be safe
  expect_null_record(db_find_record_by_id(NULL, 1234567),
                     "NULL database pointer");

  // case 2: db initialised but not populated
  StudentDatabase *empty_db = db_init();
  if (!empty_db) {
    fprintf(stderr, "Failed to initialise empty database\n");
    return EXIT_FAILURE;
  }
  expect_null_record(
      db_find_record_by_id(empty_db, 2301234),
      "Empty database (initialised but no tables/records loaded)");
  db_free(empty_db);

  StudentDatabase *db = db_init();
  if (!db) {
    fprintf(stderr, "Failed to initialise database\n");
    return EXIT_FAILURE;
  }

  DBStatus status = db_load(db, "data/P1_8-CMS.txt");
  if (status != DB_SUCCESS) {
    fprintf(stderr, "Failed to load database: %s\n", db_status_string(status));
    db_free(db);
    return EXIT_FAILURE;
  }

  // case 3: known IDs should be found
  run_query(db, 2301234);
  run_query(db, 2201234);

  // case 4: ID that doesn't exist should return NULL
  run_query(db, 9999999);

  db_free(db);
  return EXIT_SUCCESS;
}
