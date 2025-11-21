#include "../include/checksum.h"
#include "../include/database.h"
#include "test_utils.h"
#include <string.h>

// helper functions

// create a test database with sample records
StudentDatabase *create_test_database(void) {
  StudentDatabase *db = db_init();
  if (!db)
    return NULL;

  StudentTable *table = table_init("StudentRecords");
  if (!table) {
    db_free(db);
    return NULL;
  }

  db_add_table(db, table);
  db->is_loaded = true;

  return db;
}

// add a test record to database
void add_test_record(StudentDatabase *db, int id, const char *name,
                     const char *prog, float mark) {
  StudentRecord record;
  record.id = id;
  strncpy(record.name, name, sizeof(record.name) - 1);
  record.name[sizeof(record.name) - 1] = '\0';
  strncpy(record.prog, prog, sizeof(record.prog) - 1);
  record.prog[sizeof(record.prog) - 1] = '\0';
  record.mark = mark;

  StudentTable *table = db->tables[0];
  table_add_record(table, &record);
}

// compute_record_checksum() tests

void test_record_checksum_null(void) {
  unsigned long checksum = compute_record_checksum(NULL);
  ASSERT_TRUE(checksum == 0, "null record should return 0");
}

void test_record_checksum_valid(void) {
  StudentRecord record;
  record.id = 2301234;
  strcpy(record.name, "Joshua Chen");
  strcpy(record.prog, "Software Engineering");
  record.mark = 70.5f;

  unsigned long checksum = compute_record_checksum(&record);
  ASSERT_TRUE(checksum != 0, "valid record should have non-zero checksum");
}

void test_record_checksum_consistency(void) {
  StudentRecord record;
  record.id = 2301234;
  strcpy(record.name, "Joshua Chen");
  strcpy(record.prog, "Software Engineering");
  record.mark = 70.5f;

  unsigned long checksum1 = compute_record_checksum(&record);
  unsigned long checksum2 = compute_record_checksum(&record);

  ASSERT_TRUE(checksum1 == checksum2,
              "same record should produce same checksum");
}

// compute_database_checksum() tests

void test_database_checksum_null(void) {
  unsigned long checksum = compute_database_checksum(NULL);
  ASSERT_TRUE(checksum == 0, "null database should return 0");
}

void test_database_checksum_empty(void) {
  StudentDatabase *db = create_test_database();
  ASSERT_NOT_NULL(db, "test database creation should succeed");

  if (db) {
    unsigned long checksum = compute_database_checksum(db);
    ASSERT_TRUE(checksum == 0, "empty database should return 0");
    db_free(db);
  }
}

void test_database_checksum_single_record(void) {
  StudentDatabase *db = create_test_database();
  ASSERT_NOT_NULL(db, "test database creation should succeed");

  if (db) {
    add_test_record(db, 2301234, "Joshua Chen", "Software Engineering", 70.5f);

    unsigned long checksum = compute_database_checksum(db);
    ASSERT_TRUE(checksum != 0,
                "single record database should have non-zero checksum");
    db_free(db);
  }
}

// compute_file_checksum() tests

void test_file_checksum_null(void) {
  unsigned long checksum = compute_file_checksum(NULL);
  ASSERT_TRUE(checksum == 0, "null filepath should return 0");
}

// main test runner

int main(void) {
  TEST_SUITE_START("Checksum Tests");

  // compute_record_checksum tests
  RUN_TEST(test_record_checksum_null);
  RUN_TEST(test_record_checksum_valid);
  RUN_TEST(test_record_checksum_consistency);

  // compute_database_checksum tests
  RUN_TEST(test_database_checksum_null);
  RUN_TEST(test_database_checksum_empty);
  RUN_TEST(test_database_checksum_single_record);

  // compute_file_checksum tests
  RUN_TEST(test_file_checksum_null);

  TEST_SUITE_END();
}
