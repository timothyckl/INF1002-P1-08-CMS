#include <stdio.h>
#include <stdlib.h>

#include "database.h"

static void print_record(const StudentRecord *record) {
  if (!record) {
    return;
  }
  printf("  ID: %d\n", record->id);
  printf("  Name: %s\n", record->name);
  printf("  Programme: %s\n", record->prog);
  printf("  Mark: %.1f\n", record->mark);
}

static void run_query(StudentDatabase *db, int id) {
  StudentRecord *record = db_find_record_by_id(db, id);
  if (record) {
    printf("[OK] Found record for ID=%d:\n", id);
    print_record(record);
  } else {
    printf("[WARN] No record found for ID=%d\n", id);
  }
  printf("\n");
}

int main(void) {
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

  printf("Running query helper tests...\n\n");

  // existing IDs from the sample file
  run_query(db, 2301234);
  run_query(db, 2201234);

  // non-existent ID to test miss case
  run_query(db, 9999999);

  db_free(db);
  return EXIT_SUCCESS;
}
